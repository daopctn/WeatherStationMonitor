"""
Stereo camera calibration for depth estimation
Handles camera calibration, rectification, and parameter estimation
"""
import cv2
import numpy as np
from typing import List, Tuple, Optional, Dict
import yaml
import os
from dataclasses import dataclass, asdict
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class CalibrationData:
    """Container for stereo calibration results"""
    # Left camera
    camera_matrix_left: np.ndarray
    dist_coeffs_left: np.ndarray

    # Right camera
    camera_matrix_right: np.ndarray
    dist_coeffs_right: np.ndarray

    # Stereo parameters
    rotation_matrix: np.ndarray
    translation_vector: np.ndarray
    essential_matrix: np.ndarray
    fundamental_matrix: np.ndarray

    # Rectification
    R1: Optional[np.ndarray] = None
    R2: Optional[np.ndarray] = None
    P1: Optional[np.ndarray] = None
    P2: Optional[np.ndarray] = None
    Q: Optional[np.ndarray] = None

    # Image dimensions
    image_size: Tuple[int, int] = (1280, 720)

    # Calibration quality metrics
    reprojection_error: float = 0.0

    def save(self, filepath: str):
        """Save calibration data to YAML file"""
        data = {
            'camera_matrix_left': self.camera_matrix_left.tolist(),
            'dist_coeffs_left': self.dist_coeffs_left.tolist(),
            'camera_matrix_right': self.camera_matrix_right.tolist(),
            'dist_coeffs_right': self.dist_coeffs_right.tolist(),
            'rotation_matrix': self.rotation_matrix.tolist(),
            'translation_vector': self.translation_vector.tolist(),
            'essential_matrix': self.essential_matrix.tolist(),
            'fundamental_matrix': self.fundamental_matrix.tolist(),
            'image_size': list(self.image_size),
            'reprojection_error': float(self.reprojection_error)
        }

        if self.R1 is not None:
            data['R1'] = self.R1.tolist()
            data['R2'] = self.R2.tolist()
            data['P1'] = self.P1.tolist()
            data['P2'] = self.P2.tolist()
            data['Q'] = self.Q.tolist()

        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, 'w') as f:
            yaml.dump(data, f)

        logger.info(f"✓ Calibration saved to {filepath}")

    @classmethod
    def load(cls, filepath: str) -> 'CalibrationData':
        """Load calibration data from YAML file"""
        with open(filepath, 'r') as f:
            data = yaml.safe_load(f)

        calib = cls(
            camera_matrix_left=np.array(data['camera_matrix_left']),
            dist_coeffs_left=np.array(data['dist_coeffs_left']),
            camera_matrix_right=np.array(data['camera_matrix_right']),
            dist_coeffs_right=np.array(data['dist_coeffs_right']),
            rotation_matrix=np.array(data['rotation_matrix']),
            translation_vector=np.array(data['translation_vector']),
            essential_matrix=np.array(data['essential_matrix']),
            fundamental_matrix=np.array(data['fundamental_matrix']),
            image_size=tuple(data['image_size']),
            reprojection_error=data['reprojection_error']
        )

        if 'R1' in data:
            calib.R1 = np.array(data['R1'])
            calib.R2 = np.array(data['R2'])
            calib.P1 = np.array(data['P1'])
            calib.P2 = np.array(data['P2'])
            calib.Q = np.array(data['Q'])

        logger.info(f"✓ Calibration loaded from {filepath}")
        return calib


class StereoCalibrator:
    """
    Stereo camera calibration system

    Features:
    - Checkerboard-based calibration
    - Individual camera calibration
    - Stereo parameter estimation
    - Rectification computation
    """

    def __init__(self, checkerboard_size: Tuple[int, int] = (9, 6),
                 square_size: float = 25.0):
        """
        Initialize stereo calibrator

        Args:
            checkerboard_size: Number of inner corners (width, height)
            square_size: Size of checkerboard squares in mm
        """
        self.checkerboard_size = checkerboard_size
        self.square_size = square_size

        # Termination criteria for corner refinement
        self.criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

        # Results
        self.calibration_data: Optional[CalibrationData] = None

    def calibrate_from_images(self, left_images: List[np.ndarray],
                              right_images: List[np.ndarray]) -> CalibrationData:
        """
        Calibrate stereo pair from checkerboard images

        Args:
            left_images: List of images from left camera
            right_images: List of images from right camera

        Returns:
            CalibrationData object with calibration results
        """
        logger.info(f"Starting calibration with {len(left_images)} image pairs...")

        if len(left_images) != len(right_images):
            raise ValueError("Number of left and right images must match")

        # Prepare object points (3D points in real world space)
        objp = np.zeros((self.checkerboard_size[0] * self.checkerboard_size[1], 3),
                       np.float32)
        objp[:, :2] = np.mgrid[0:self.checkerboard_size[0],
                               0:self.checkerboard_size[1]].T.reshape(-1, 2)
        objp *= self.square_size

        # Arrays to store object points and image points
        objpoints = []  # 3D points
        imgpoints_left = []  # 2D points in left image
        imgpoints_right = []  # 2D points in right image

        image_size = None

        # Find checkerboard corners in each image pair
        for i, (img_left, img_right) in enumerate(zip(left_images, right_images)):
            gray_left = cv2.cvtColor(img_left, cv2.COLOR_BGR2GRAY)
            gray_right = cv2.cvtColor(img_right, cv2.COLOR_BGR2GRAY)

            if image_size is None:
                image_size = gray_left.shape[::-1]

            # Find corners
            ret_left, corners_left = cv2.findChessboardCorners(
                gray_left, self.checkerboard_size, None
            )
            ret_right, corners_right = cv2.findChessboardCorners(
                gray_right, self.checkerboard_size, None
            )

            if ret_left and ret_right:
                # Refine corner positions
                corners_left = cv2.cornerSubPix(
                    gray_left, corners_left, (11, 11), (-1, -1), self.criteria
                )
                corners_right = cv2.cornerSubPix(
                    gray_right, corners_right, (11, 11), (-1, -1), self.criteria
                )

                objpoints.append(objp)
                imgpoints_left.append(corners_left)
                imgpoints_right.append(corners_right)

                logger.info(f"✓ Image pair {i+1}/{len(left_images)} - corners found")
            else:
                logger.warning(f"✗ Image pair {i+1}/{len(left_images)} - corners not found")

        if len(objpoints) < 10:
            raise ValueError(f"Not enough valid image pairs found ({len(objpoints)}). Need at least 10.")

        logger.info(f"Found corners in {len(objpoints)} image pairs")

        # Calibrate individual cameras first
        logger.info("Calibrating left camera...")
        ret_left, mtx_left, dist_left, rvecs_left, tvecs_left = cv2.calibrateCamera(
            objpoints, imgpoints_left, image_size, None, None
        )

        logger.info("Calibrating right camera...")
        ret_right, mtx_right, dist_right, rvecs_right, tvecs_right = cv2.calibrateCamera(
            objpoints, imgpoints_right, image_size, None, None
        )

        # Stereo calibration
        logger.info("Performing stereo calibration...")
        flags = cv2.CALIB_FIX_INTRINSIC
        ret_stereo, mtx_left, dist_left, mtx_right, dist_right, R, T, E, F = cv2.stereoCalibrate(
            objpoints, imgpoints_left, imgpoints_right,
            mtx_left, dist_left,
            mtx_right, dist_right,
            image_size,
            criteria=self.criteria,
            flags=flags
        )

        logger.info(f"✓ Stereo calibration complete. Reprojection error: {ret_stereo:.4f}")

        # Create calibration data object
        self.calibration_data = CalibrationData(
            camera_matrix_left=mtx_left,
            dist_coeffs_left=dist_left,
            camera_matrix_right=mtx_right,
            dist_coeffs_right=dist_right,
            rotation_matrix=R,
            translation_vector=T,
            essential_matrix=E,
            fundamental_matrix=F,
            image_size=image_size,
            reprojection_error=ret_stereo
        )

        return self.calibration_data

    def compute_rectification(self, calibration_data: Optional[CalibrationData] = None) -> CalibrationData:
        """
        Compute rectification parameters

        Args:
            calibration_data: CalibrationData object (uses self.calibration_data if None)

        Returns:
            Updated CalibrationData with rectification parameters
        """
        if calibration_data is None:
            calibration_data = self.calibration_data

        if calibration_data is None:
            raise ValueError("No calibration data available. Run calibrate_from_images first.")

        logger.info("Computing rectification parameters...")

        R1, R2, P1, P2, Q, roi_left, roi_right = cv2.stereoRectify(
            calibration_data.camera_matrix_left,
            calibration_data.dist_coeffs_left,
            calibration_data.camera_matrix_right,
            calibration_data.dist_coeffs_right,
            calibration_data.image_size,
            calibration_data.rotation_matrix,
            calibration_data.translation_vector,
            alpha=0
        )

        calibration_data.R1 = R1
        calibration_data.R2 = R2
        calibration_data.P1 = P1
        calibration_data.P2 = P2
        calibration_data.Q = Q

        logger.info("✓ Rectification parameters computed")

        return calibration_data

    def undistort_rectify(self, image: np.ndarray, camera: str,
                         calibration_data: Optional[CalibrationData] = None) -> np.ndarray:
        """
        Undistort and rectify image

        Args:
            image: Input image
            camera: 'left' or 'right'
            calibration_data: CalibrationData object

        Returns:
            Rectified image
        """
        if calibration_data is None:
            calibration_data = self.calibration_data

        if calibration_data is None or calibration_data.R1 is None:
            raise ValueError("Rectification parameters not computed")

        if camera == 'left':
            mtx = calibration_data.camera_matrix_left
            dist = calibration_data.dist_coeffs_left
            R = calibration_data.R1
            P = calibration_data.P1
        elif camera == 'right':
            mtx = calibration_data.camera_matrix_right
            dist = calibration_data.dist_coeffs_right
            R = calibration_data.R2
            P = calibration_data.P2
        else:
            raise ValueError("camera must be 'left' or 'right'")

        # Create rectification map
        map1, map2 = cv2.initUndistortRectifyMap(
            mtx, dist, R, P, calibration_data.image_size, cv2.CV_32FC1
        )

        # Apply rectification
        rectified = cv2.remap(image, map1, map2, cv2.INTER_LINEAR)

        return rectified
