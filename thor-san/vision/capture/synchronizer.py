"""
Frame synchronization utilities for multi-camera systems
Handles timestamp alignment and frame interpolation
"""
import numpy as np
from typing import Dict, List, Optional, Tuple
import time


class FrameSynchronizer:
    """
    Synchronizes frames from multiple cameras based on timestamps

    Features:
    - Timestamp-based frame alignment
    - Interpolation for missing frames
    - Configurable synchronization tolerance
    """

    def __init__(self, max_time_diff: float = 0.033):
        """
        Initialize frame synchronizer

        Args:
            max_time_diff: Maximum allowed time difference between frames (seconds)
                          Default: 33ms (~30fps tolerance)
        """
        self.max_time_diff = max_time_diff
        self.frame_buffer = {}  # Buffer for recent frames
        self.buffer_size = 10

    def add_frame(self, camera_name: str, frame_data: Dict):
        """
        Add frame to synchronization buffer

        Args:
            camera_name: Identifier for camera
            frame_data: Dict containing 'image', 'timestamp', etc.
        """
        if camera_name not in self.frame_buffer:
            self.frame_buffer[camera_name] = []

        self.frame_buffer[camera_name].append(frame_data)

        # Maintain buffer size
        if len(self.frame_buffer[camera_name]) > self.buffer_size:
            self.frame_buffer[camera_name].pop(0)

    def get_synchronized_set(self, camera_names: List[str]) -> Optional[Dict]:
        """
        Get synchronized frame set from specified cameras

        Args:
            camera_names: List of camera names to synchronize

        Returns:
            Dict mapping camera names to frame data, or None if sync failed
        """
        if not all(name in self.frame_buffer for name in camera_names):
            return None

        # Get latest timestamp from any camera
        latest_times = {}
        for name in camera_names:
            if self.frame_buffer[name]:
                latest_times[name] = self.frame_buffer[name][-1]['timestamp']

        if len(latest_times) != len(camera_names):
            return None

        # Find reference timestamp (median of latest)
        ref_timestamp = np.median(list(latest_times.values()))

        # Find closest frame for each camera
        synced_frames = {}
        for name in camera_names:
            closest_frame = self._find_closest_frame(
                self.frame_buffer[name],
                ref_timestamp
            )

            if closest_frame is None:
                return None

            time_diff = abs(closest_frame['timestamp'] - ref_timestamp)
            if time_diff > self.max_time_diff:
                return None

            synced_frames[name] = closest_frame

        return synced_frames

    def _find_closest_frame(self, frames: List[Dict], target_time: float) -> Optional[Dict]:
        """
        Find frame with timestamp closest to target

        Args:
            frames: List of frame data dicts
            target_time: Target timestamp

        Returns:
            Closest frame or None
        """
        if not frames:
            return None

        closest = min(frames, key=lambda f: abs(f['timestamp'] - target_time))
        return closest

    def clear_buffer(self):
        """Clear all buffered frames"""
        self.frame_buffer.clear()

    def get_buffer_status(self) -> Dict:
        """Get status of frame buffers"""
        status = {}
        for name, buffer in self.frame_buffer.items():
            if buffer:
                status[name] = {
                    'count': len(buffer),
                    'oldest': buffer[0]['timestamp'],
                    'newest': buffer[-1]['timestamp'],
                    'span': buffer[-1]['timestamp'] - buffer[0]['timestamp']
                }
            else:
                status[name] = {'count': 0}
        return status
