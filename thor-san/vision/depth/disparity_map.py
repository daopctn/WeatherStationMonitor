"""
Disparity map computation for stereo depth estimation
Real-time disparity calculation using OpenCV stereo matching
"""
import cv2
import numpy as np
from typing import Optional, Tuple
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class DisparityEstimator:
    """
    Computes disparity maps from stereo image pairs

    Features:
    - Multiple stereo matching algorithms (SGBM, BM)
    - Real-time performance optimization
    - Configurable parameters
    - Post-processing filters
    """

    def __init__(self, method: str = 'sgbm', num_disparities: int = 128,
                 block_size: int = 11):
        """
        Initialize disparity estimator

        Args:
            method: Stereo matching method ('sgbm' or 'bm')
            num_disparities: Maximum disparity value (must be divisible by 16)
            block_size: Size of matching window (odd number)
        """
        self.method = method
        self.num_disparities = num_disparities
        self.block_size = block_size

        # Ensure valid parameters
        if num_disparities % 16 != 0:
            self.num_disparities = ((num_disparities // 16) + 1) * 16
            logger.warning(f"num_disparities adjusted to {self.num_disparities}")

        if block_size % 2 == 0:
            self.block_size = block_size + 1
            logger.warning(f"block_size adjusted to {self.block_size}")

        # Create stereo matcher
        self._create_matcher()

        # WLS filter for post-processing
        self.wls_filter = None
        self.use_wls = False

    def _create_matcher(self):
        """Create stereo matcher based on method"""
        if self.method == 'sgbm':
            # Semi-Global Block Matching - better quality but slower
            self.stereo = cv2.StereoSGBM_create(
                minDisparity=0,
                numDisparities=self.num_disparities,
                blockSize=self.block_size,
                P1=8 * 3 * self.block_size ** 2,
                P2=32 * 3 * self.block_size ** 2,
                disp12MaxDiff=1,
                uniquenessRatio=10,
                speckleWindowSize=100,
                speckleRange=32,
                preFilterCap=63,
                mode=cv2.STEREO_SGBM_MODE_SGBM_3WAY
            )
            logger.info("✓ SGBM matcher created")

        elif self.method == 'bm':
            # Block Matching - faster but lower quality
            self.stereo = cv2.StereoBM_create(
                numDisparities=self.num_disparities,
                blockSize=self.block_size
            )
            logger.info("✓ BM matcher created")

        else:
            raise ValueError(f"Unknown method: {self.method}")

    def enable_wls_filter(self, lambda_param: float = 8000.0, sigma: float = 1.5):
        """
        Enable WLS (Weighted Least Squares) filter for better disparity

        Args:
            lambda_param: Regularization parameter (larger = smoother)
            sigma: Standard deviation for color weight
        """
        self.use_wls = True
        self.wls_filter = cv2.ximgproc.createDisparityWLSFilter(self.stereo)
        self.wls_filter.setLambda(lambda_param)
        self.wls_filter.setSigmaColor(sigma)
        logger.info("✓ WLS filter enabled")

    def compute(self, left_image: np.ndarray, right_image: np.ndarray) -> np.ndarray:
        """
        Compute disparity map from stereo pair

        Args:
            left_image: Left rectified image (grayscale or BGR)
            right_image: Right rectified image (grayscale or BGR)

        Returns:
            Disparity map (float32, values in pixels)
        """
        # Convert to grayscale if needed
        if len(left_image.shape) == 3:
            left_gray = cv2.cvtColor(left_image, cv2.COLOR_BGR2GRAY)
        else:
            left_gray = left_image

        if len(right_image.shape) == 3:
            right_gray = cv2.cvtColor(right_image, cv2.COLOR_BGR2GRAY)
        else:
            right_gray = right_image

        # Compute disparity
        disparity = self.stereo.compute(left_gray, right_gray)

        # Convert to float32 and scale
        disparity = disparity.astype(np.float32) / 16.0

        # Apply WLS filter if enabled
        if self.use_wls and self.wls_filter is not None:
            # Compute right disparity for WLS
            right_matcher = cv2.ximgproc.createRightMatcher(self.stereo)
            disparity_right = right_matcher.compute(right_gray, left_gray)
            disparity_right = disparity_right.astype(np.float32) / 16.0

            # Apply filter
            disparity = self.wls_filter.filter(
                disparity.astype(np.int16),
                left_gray,
                None,
                disparity_right.astype(np.int16)
            )
            disparity = disparity.astype(np.float32)

        return disparity

    def compute_depth(self, disparity: np.ndarray, Q: np.ndarray,
                     max_depth: float = 10000.0) -> np.ndarray:
        """
        Convert disparity to depth using Q matrix

        Args:
            disparity: Disparity map
            Q: Reprojection matrix (4x4) from calibration
            max_depth: Maximum depth value in mm

        Returns:
            Depth map in mm
        """
        # Reproject to 3D
        points_3d = cv2.reprojectImageTo3D(disparity, Q)

        # Extract Z (depth) channel
        depth = points_3d[:, :, 2]

        # Clip to valid range
        depth = np.clip(depth, 0, max_depth)

        # Handle invalid disparities (disparity <= 0)
        depth[disparity <= 0] = 0

        return depth

    def visualize_disparity(self, disparity: np.ndarray) -> np.ndarray:
        """
        Create visualization of disparity map

        Args:
            disparity: Disparity map

        Returns:
            Colored disparity visualization (BGR)
        """
        # Normalize to 0-255
        disp_vis = disparity.copy()
        disp_vis[disp_vis < 0] = 0

        if disp_vis.max() > 0:
            disp_vis = (disp_vis / disp_vis.max() * 255).astype(np.uint8)
        else:
            disp_vis = np.zeros_like(disp_vis, dtype=np.uint8)

        # Apply colormap
        disp_color = cv2.applyColorMap(disp_vis, cv2.COLORMAP_JET)

        return disp_color

    def visualize_depth(self, depth: np.ndarray, max_depth: float = 2000.0) -> np.ndarray:
        """
        Create visualization of depth map

        Args:
            depth: Depth map in mm
            max_depth: Maximum depth for visualization

        Returns:
            Colored depth visualization (BGR)
        """
        depth_vis = depth.copy()
        depth_vis = np.clip(depth_vis, 0, max_depth)

        # Normalize
        if depth_vis.max() > 0:
            depth_vis = (depth_vis / max_depth * 255).astype(np.uint8)
        else:
            depth_vis = np.zeros_like(depth_vis, dtype=np.uint8)

        # Invert (closer = brighter)
        depth_vis = 255 - depth_vis

        # Apply colormap
        depth_color = cv2.applyColorMap(depth_vis, cv2.COLORMAP_TURBO)

        return depth_color

    def update_parameters(self, **kwargs):
        """Update matcher parameters dynamically"""
        if 'num_disparities' in kwargs:
            self.num_disparities = kwargs['num_disparities']
        if 'block_size' in kwargs:
            self.block_size = kwargs['block_size']

        # Recreate matcher with new parameters
        self._create_matcher()

        if self.use_wls:
            self.enable_wls_filter()
