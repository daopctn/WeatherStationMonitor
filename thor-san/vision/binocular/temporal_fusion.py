"""
Temporal depth fusion - Visual memory integration
Integrates depth estimates over time like human visual memory
"""
import numpy as np
from collections import deque
from typing import Tuple, Optional
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class TemporalDepthFusion:
    """
    Integrates depth measurements over time

    Simulates visual memory and temporal integration in human vision
    - Reduces noise through temporal averaging
    - Maintains consistent 3D representation
    - Handles occlusions and disocclusions
    """

    def __init__(self, window_size: int = 10, decay_factor: float = 0.95):
        """
        Initialize temporal fusion

        Args:
            window_size: Number of frames to integrate
            decay_factor: Confidence decay per frame
        """
        self.window_size = window_size
        self.decay_factor = decay_factor

        # Temporal buffers (simulates visual memory)
        self.depth_buffer = deque(maxlen=window_size)
        self.confidence_buffer = deque(maxlen=window_size)

        # Accumulated maps
        self.accumulated_depth = None
        self.accumulated_confidence = None
        self.frame_count = 0

        logger.info(f"✓ Temporal depth fusion initialized (window={window_size})")

    def integrate(self, depth_map: np.ndarray,
                 confidence_map: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """
        Integrate new depth measurement into temporal model

        Args:
            depth_map: Current depth map
            confidence_map: Current confidence map

        Returns:
            Tuple of (fused_depth, fused_confidence)
        """
        # Add to buffers
        self.depth_buffer.append(depth_map)
        self.confidence_buffer.append(confidence_map)
        self.frame_count += 1

        # Initialize accumulated maps on first frame
        if self.accumulated_depth is None:
            self.accumulated_depth = depth_map.copy()
            self.accumulated_confidence = confidence_map.copy()
            return self.accumulated_depth, self.accumulated_confidence

        # Apply temporal decay to existing confidence
        self.accumulated_confidence *= self.decay_factor

        # Fuse with new measurement
        fused_depth, fused_confidence = self._fuse_measurements()

        # Update accumulated maps
        self.accumulated_depth = fused_depth
        self.accumulated_confidence = fused_confidence

        return fused_depth, fused_confidence

    def _fuse_measurements(self) -> Tuple[np.ndarray, np.ndarray]:
        """
        Fuse depth measurements using confidence-weighted averaging

        Returns:
            Tuple of (fused_depth, fused_confidence)
        """
        if len(self.depth_buffer) == 0:
            return self.accumulated_depth, self.accumulated_confidence

        # Stack buffers
        depth_stack = np.array(list(self.depth_buffer))
        conf_stack = np.array(list(self.confidence_buffer))

        # Confidence-weighted mean depth
        # depth_fused = sum(depth * confidence) / sum(confidence)

        # Mask invalid measurements
        valid_mask = depth_stack > 0

        # Weighted sum
        weighted_depth = np.sum(depth_stack * conf_stack * valid_mask, axis=0)
        weight_sum = np.sum(conf_stack * valid_mask, axis=0)

        # Avoid division by zero
        weight_sum = np.maximum(weight_sum, 1e-6)

        # Fused depth
        fused_depth = weighted_depth / weight_sum

        # Fused confidence (accumulated evidence)
        fused_confidence = np.mean(conf_stack * valid_mask, axis=0)

        return fused_depth, fused_confidence

    def reset(self):
        """Reset temporal integration"""
        self.depth_buffer.clear()
        self.confidence_buffer.clear()
        self.accumulated_depth = None
        self.accumulated_confidence = None
        self.frame_count = 0
        logger.info("✓ Temporal fusion reset")

    def get_statistics(self) -> dict:
        """Get fusion statistics"""
        return {
            'frame_count': self.frame_count,
            'buffer_size': len(self.depth_buffer),
            'mean_confidence': float(np.mean(self.accumulated_confidence)) if self.accumulated_confidence is not None else 0
        }
