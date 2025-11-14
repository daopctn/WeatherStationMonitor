"""
Visual attention mechanism
Simulates human selective attention in vision
"""
import cv2
import numpy as np
from typing import List, Tuple, Optional
from dataclasses import dataclass
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class AttentionRegion:
    """Region of visual attention"""
    center: Tuple[int, int]
    radius: float
    saliency: float  # Importance score


class VisualAttention:
    """
    Visual attention mechanism for binocular vision

    Simulates human selective attention:
    - Bottom-up saliency (feature contrast)
    - Top-down attention (task-driven)
    - Attentional weighting of depth estimates
    """

    def __init__(self):
        """Initialize visual attention system"""
        self.attention_map = None
        self.attention_regions = []

        logger.info("✓ Visual attention initialized")

    def compute_attention_map(self, left_image: np.ndarray,
                             right_image: np.ndarray,
                             matches: List) -> np.ndarray:
        """
        Compute attention map from binocular input

        Args:
            left_image: Left eye image
            right_image: Right eye image
            matches: Feature matches

        Returns:
            Attention map (0-1 normalized)
        """
        height, width = left_image.shape[:2]

        # Compute bottom-up saliency
        saliency_map = self._compute_saliency(left_image)

        # Compute match density (areas with many matches are interesting)
        match_density = self._compute_match_density(matches, (height, width))

        # Combine saliency sources
        attention_map = 0.6 * saliency_map + 0.4 * match_density

        # Normalize
        attention_map = cv2.normalize(attention_map, None, 0, 1, cv2.NORM_MINMAX)

        # Apply Gaussian smoothing (simulates attention spread)
        attention_map = cv2.GaussianBlur(attention_map, (31, 31), 10)

        self.attention_map = attention_map

        return attention_map

    def _compute_saliency(self, image: np.ndarray) -> np.ndarray:
        """
        Compute bottom-up visual saliency

        Args:
            image: Input image

        Returns:
            Saliency map
        """
        # Convert to LAB color space
        if len(image.shape) == 3:
            lab = cv2.cvtColor(image, cv2.COLOR_BGR2LAB)
        else:
            gray = image
            lab = cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
            lab = cv2.cvtColor(lab, cv2.COLOR_BGR2LAB)

        # Gaussian blur (center-surround)
        blurred = cv2.GaussianBlur(lab, (21, 21), 7)

        # Compute difference (feature contrast)
        saliency = np.abs(lab.astype(np.float32) - blurred.astype(np.float32))

        # Combine channels
        saliency = np.mean(saliency, axis=2)

        # Normalize
        saliency = cv2.normalize(saliency, None, 0, 1, cv2.NORM_MINMAX)

        return saliency.astype(np.float32)

    def _compute_match_density(self, matches: List, shape: Tuple[int, int]) -> np.ndarray:
        """
        Compute density of feature matches

        Args:
            matches: List of matches
            shape: Image shape (height, width)

        Returns:
            Match density map
        """
        height, width = shape
        density_map = np.zeros((height, width), dtype=np.float32)

        if len(matches) == 0:
            return density_map

        # Create heat map of match locations
        # (Areas with many features are interesting)
        for match in matches:
            # Note: This assumes matches have queryIdx attribute
            # In practice, you'd extract keypoint positions
            pass

        # For now, return uniform map
        # In full implementation, would create density from match locations
        return np.ones((height, width), dtype=np.float32) * 0.5

    def apply_top_down_attention(self, task_regions: List[AttentionRegion],
                                 image_shape: Tuple[int, int]):
        """
        Apply task-driven top-down attention

        Args:
            task_regions: Regions of interest for task
            image_shape: Image dimensions
        """
        height, width = image_shape
        task_map = np.zeros((height, width), dtype=np.float32)

        # Create Gaussian attention at each region
        for region in task_regions:
            cx, cy = region.center
            sigma = region.radius / 2

            # Create Gaussian
            y, x = np.ogrid[:height, :width]
            gaussian = np.exp(-((x - cx)**2 + (y - cy)**2) / (2 * sigma**2))
            gaussian *= region.saliency

            task_map = np.maximum(task_map, gaussian)

        # Combine with bottom-up attention
        if self.attention_map is not None:
            self.attention_map = 0.5 * self.attention_map + 0.5 * task_map
        else:
            self.attention_map = task_map

        self.attention_regions = task_regions

    def visualize_attention(self, base_image: np.ndarray) -> np.ndarray:
        """
        Visualize attention map overlay

        Args:
            base_image: Base image to overlay

        Returns:
            Visualization image
        """
        if self.attention_map is None:
            return base_image

        # Resize attention map if needed
        if base_image.shape[:2] != self.attention_map.shape:
            attention_resized = cv2.resize(self.attention_map,
                                         (base_image.shape[1], base_image.shape[0]))
        else:
            attention_resized = self.attention_map

        # Create colored attention map
        attention_colored = cv2.applyColorMap(
            (attention_resized * 255).astype(np.uint8),
            cv2.COLORMAP_HOT
        )

        # Blend with base image
        output = cv2.addWeighted(base_image, 0.6, attention_colored, 0.4, 0)

        return output
