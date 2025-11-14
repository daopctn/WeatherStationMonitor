"""
Binocular Vision System - Human-like depth perception
Processes two monocular camera streams like human eyes
"""
import cv2
import numpy as np
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class BinocularConfig:
    """Configuration for binocular vision system"""
    # Camera parameters
    baseline: float = 0.065  # Inter-ocular distance in meters (human: ~6.5cm)
    focal_length: float = 700.0  # Focal length in pixels

    # Processing parameters
    feature_type: str = "orb"  # Feature detector: orb, sift, surf
    max_features: int = 500
    match_ratio: float = 0.75  # Lowe's ratio test

    # Depth estimation
    min_disparity: float = 1.0
    max_disparity: float = 100.0

    # Temporal integration
    temporal_window: int = 10  # Frames to integrate
    confidence_decay: float = 0.95

    # Attention mechanism
    use_attention: bool = True
    attention_regions: int = 5


class BinocularVisionSystem:
    """
    Human-like binocular vision processing

    Features:
    - Independent monocular processing (like each eye)
    - Feature-based correspondence matching
    - Temporal depth integration (visual memory)
    - Attention-driven processing
    - Multiple depth cue fusion
    """

    def __init__(self, config: Optional[BinocularConfig] = None):
        """
        Initialize binocular vision system

        Args:
            config: BinocularConfig instance
        """
        self.config = config if config else BinocularConfig()

        # Feature detector (simulates visual feature extraction)
        self._init_feature_detector()

        # Correspondence matcher (simulates binocular neurons)
        from .correspondence_matcher import CorrespondenceMatcher
        self.correspondence_matcher = CorrespondenceMatcher(self.config)

        # Temporal fusion (simulates visual memory)
        from .temporal_fusion import TemporalDepthFusion
        self.temporal_fusion = TemporalDepthFusion(
            window_size=self.config.temporal_window
        )

        # Visual attention (simulates attention mechanism)
        if self.config.use_attention:
            from .visual_attention import VisualAttention
            self.attention = VisualAttention()
        else:
            self.attention = None

        # State
        self.left_features = None
        self.right_features = None
        self.depth_map = None
        self.confidence_map = None

        logger.info("✓ Binocular vision system initialized (human-like)")

    def _init_feature_detector(self):
        """Initialize feature detector (simulates retinal processing)"""
        if self.config.feature_type == "orb":
            self.detector = cv2.ORB_create(nfeatures=self.config.max_features)
        elif self.config.feature_type == "sift":
            self.detector = cv2.SIFT_create(nfeatures=self.config.max_features)
        elif self.config.feature_type == "surf":
            self.detector = cv2.xfeatures2d.SURF_create()
        else:
            self.detector = cv2.ORB_create(nfeatures=self.config.max_features)

    def process_frame_pair(self, left_image: np.ndarray, right_image: np.ndarray) -> Dict:
        """
        Process binocular frame pair (like visual cortex)

        Args:
            left_image: Left eye image
            right_image: Right eye image

        Returns:
            Dictionary with depth map, confidence, and features
        """
        # Step 1: Monocular preprocessing (like retinal processing)
        left_gray = self._preprocess_monocular(left_image)
        right_gray = self._preprocess_monocular(right_image)

        # Step 2: Extract features from each eye independently
        left_kp, left_desc = self._extract_features(left_gray, "left")
        right_kp, right_desc = self._extract_features(right_gray, "right")

        # Step 3: Find correspondences (like binocular neurons)
        matches = self.correspondence_matcher.match_features(
            left_kp, left_desc, right_kp, right_desc
        )

        # Step 4: Apply visual attention if enabled
        if self.attention:
            attention_mask = self.attention.compute_attention_map(
                left_image, right_image, matches
            )
        else:
            attention_mask = None

        # Step 5: Estimate depth from correspondences
        depth_map, confidence_map = self._estimate_depth_from_matches(
            left_image.shape[:2], matches, left_kp, right_kp
        )

        # Step 6: Apply attention-weighted processing
        if attention_mask is not None:
            confidence_map = confidence_map * attention_mask

        # Step 7: Temporal integration (visual memory)
        depth_map, confidence_map = self.temporal_fusion.integrate(
            depth_map, confidence_map
        )

        # Store state
        self.depth_map = depth_map
        self.confidence_map = confidence_map
        self.left_features = (left_kp, left_desc)
        self.right_features = (right_kp, right_desc)

        return {
            'depth_map': depth_map,
            'confidence_map': confidence_map,
            'num_matches': len(matches),
            'attention_mask': attention_mask,
            'left_features': len(left_kp),
            'right_features': len(right_kp)
        }

    def _preprocess_monocular(self, image: np.ndarray) -> np.ndarray:
        """
        Preprocess single camera image (simulates retinal preprocessing)

        Args:
            image: Input image

        Returns:
            Preprocessed grayscale image
        """
        if len(image.shape) == 3:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        else:
            gray = image

        # Contrast enhancement (simulates retinal adaptation)
        gray = cv2.equalizeHist(gray)

        # Noise reduction
        gray = cv2.GaussianBlur(gray, (3, 3), 0)

        return gray

    def _extract_features(self, image: np.ndarray, eye: str) -> Tuple:
        """
        Extract visual features (simulates V1 cortex feature detection)

        Args:
            image: Grayscale image
            eye: 'left' or 'right'

        Returns:
            Tuple of (keypoints, descriptors)
        """
        keypoints, descriptors = self.detector.detectAndCompute(image, None)

        if descriptors is None:
            descriptors = np.array([])

        return keypoints, descriptors

    def _estimate_depth_from_matches(self, image_shape: Tuple[int, int],
                                    matches: List,
                                    left_kp: List, right_kp: List) -> Tuple[np.ndarray, np.ndarray]:
        """
        Estimate dense depth map from sparse matches

        Args:
            image_shape: Image dimensions (height, width)
            matches: List of feature matches
            left_kp: Left keypoints
            right_kp: Right keypoints

        Returns:
            Tuple of (depth_map, confidence_map)
        """
        height, width = image_shape

        # Initialize maps
        depth_map = np.zeros((height, width), dtype=np.float32)
        confidence_map = np.zeros((height, width), dtype=np.float32)

        if len(matches) == 0:
            return depth_map, confidence_map

        # Calculate depth for each match
        for match in matches:
            left_pt = left_kp[match.queryIdx].pt
            right_pt = right_kp[match.trainIdx].pt

            # Calculate disparity (horizontal offset between eyes)
            disparity = abs(left_pt[0] - right_pt[0])

            # Skip if disparity out of range
            if disparity < self.config.min_disparity or disparity > self.config.max_disparity:
                continue

            # Calculate depth using triangulation
            # depth = (baseline * focal_length) / disparity
            if disparity > 0:
                depth = (self.config.baseline * self.config.focal_length) / disparity
            else:
                continue

            # Place depth value at feature location
            x, y = int(left_pt[0]), int(left_pt[1])
            if 0 <= x < width and 0 <= y < height:
                # Use match distance as confidence (lower = better)
                confidence = 1.0 / (1.0 + match.distance / 100.0)

                depth_map[y, x] = depth
                confidence_map[y, x] = confidence

        # Dense interpolation (fill gaps like visual cortex)
        depth_map = self._interpolate_depth_map(depth_map, confidence_map)

        return depth_map, confidence_map

    def _interpolate_depth_map(self, depth_map: np.ndarray,
                               confidence_map: np.ndarray) -> np.ndarray:
        """
        Interpolate sparse depth to dense map

        Args:
            depth_map: Sparse depth map
            confidence_map: Confidence values

        Returns:
            Dense depth map
        """
        # Find valid depth points
        valid_mask = (depth_map > 0) & (confidence_map > 0.3)

        if not valid_mask.any():
            return depth_map

        # Use inpainting to fill gaps (simulates cortical filling-in)
        depth_map_8u = cv2.normalize(depth_map, None, 0, 255, cv2.NORM_MINMAX).astype(np.uint8)
        inpaint_mask = (1 - valid_mask).astype(np.uint8) * 255

        inpainted = cv2.inpaint(depth_map_8u, inpaint_mask, 3, cv2.INPAINT_TELEA)

        # Convert back to float depth
        depth_dense = inpainted.astype(np.float32) / 255.0 * depth_map.max()

        return depth_dense

    def get_3d_points(self, depth_threshold: float = 0.1) -> np.ndarray:
        """
        Convert depth map to 3D point cloud

        Args:
            depth_threshold: Minimum depth value

        Returns:
            Nx3 array of 3D points
        """
        if self.depth_map is None:
            return np.array([])

        height, width = self.depth_map.shape

        # Create coordinate grids
        u, v = np.meshgrid(np.arange(width), np.arange(height))

        # Filter by confidence and depth
        valid_mask = (self.confidence_map > 0.3) & (self.depth_map > depth_threshold)

        # Get valid depth values
        depth = self.depth_map[valid_mask]
        u_valid = u[valid_mask]
        v_valid = v[valid_mask]

        # Back-project to 3D (pinhole camera model)
        # X = (u - cx) * Z / fx
        # Y = (v - cy) * Z / fy
        # Z = depth

        cx, cy = width / 2, height / 2
        fx, fy = self.config.focal_length, self.config.focal_length

        X = (u_valid - cx) * depth / fx
        Y = (v_valid - cy) * depth / fy
        Z = depth

        points_3d = np.stack([X, Y, Z], axis=-1)

        return points_3d

    def visualize_depth(self, colormap: int = cv2.COLORMAP_TURBO) -> np.ndarray:
        """
        Create visualization of depth map

        Args:
            colormap: OpenCV colormap

        Returns:
            Colored depth visualization
        """
        if self.depth_map is None:
            return np.zeros((480, 640, 3), dtype=np.uint8)

        # Normalize depth for visualization
        depth_norm = self.depth_map.copy()
        depth_norm[depth_norm == 0] = depth_norm.max()

        depth_vis = cv2.normalize(depth_norm, None, 0, 255, cv2.NORM_MINMAX)
        depth_vis = depth_vis.astype(np.uint8)

        # Apply colormap
        depth_colored = cv2.applyColorMap(depth_vis, colormap)

        # Mask out invalid regions
        if self.confidence_map is not None:
            mask = (self.confidence_map < 0.3).astype(np.uint8)
            depth_colored[mask > 0] = [0, 0, 0]

        return depth_colored

    def get_statistics(self) -> Dict:
        """Get processing statistics"""
        stats = {
            'depth_map_shape': self.depth_map.shape if self.depth_map is not None else None,
            'valid_depth_pixels': int(np.sum(self.depth_map > 0)) if self.depth_map is not None else 0,
            'mean_depth': float(np.mean(self.depth_map[self.depth_map > 0])) if self.depth_map is not None else 0,
            'confidence_mean': float(np.mean(self.confidence_map)) if self.confidence_map is not None else 0
        }

        if self.left_features:
            stats['left_features'] = len(self.left_features[0])
        if self.right_features:
            stats['right_features'] = len(self.right_features[0])

        return stats
