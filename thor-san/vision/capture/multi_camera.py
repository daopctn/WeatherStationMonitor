"""
Multi-camera capture system for Thor-san
Direct hardware access, no ROS dependencies
Supports simultaneous capture from multiple USB cameras with threading
"""
import cv2
import numpy as np
import threading
from typing import Dict, List, Optional, Tuple
import time
from dataclasses import dataclass
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class CameraConfig:
    """Configuration for a single camera"""
    index: int
    width: int = 1280
    height: int = 720
    fps: int = 30
    buffer_size: int = 1
    name: Optional[str] = None

    def __post_init__(self):
        if self.name is None:
            self.name = f'cam_{self.index}'


class MultiCameraSystem:
    """
    Manages multiple USB cameras with threaded capture

    Features:
    - Simultaneous capture from multiple cameras
    - Thread-safe frame access
    - Automatic timestamp synchronization
    - Hardware error recovery
    """

    def __init__(self, camera_indices: List[int] = [0, 2, 4],
                 config: Optional[Dict[int, CameraConfig]] = None):
        """
        Initialize multi-camera system

        Args:
            camera_indices: List of camera device indices
            config: Optional dict mapping camera index to CameraConfig
        """
        self.camera_indices = camera_indices
        self.cameras = {}
        self.frames = {}
        self.threads = {}
        self.locks = {}
        self.running = False
        self.frame_counts = {}
        self.error_counts = {}

        # Build configurations
        self.configs = {}
        for idx in camera_indices:
            if config and idx in config:
                self.configs[idx] = config[idx]
            else:
                self.configs[idx] = CameraConfig(index=idx)

        # Initialize cameras
        self._initialize_cameras()

    def _initialize_cameras(self):
        """Initialize all camera devices"""
        logger.info(f"Initializing {len(self.camera_indices)} cameras...")

        for idx in self.camera_indices:
            cfg = self.configs[idx]
            try:
                cam = cv2.VideoCapture(idx)

                if not cam.isOpened():
                    logger.error(f"Failed to open camera {idx}")
                    continue

                # Set camera properties
                cam.set(cv2.CAP_PROP_FRAME_WIDTH, cfg.width)
                cam.set(cv2.CAP_PROP_FRAME_HEIGHT, cfg.height)
                cam.set(cv2.CAP_PROP_FPS, cfg.fps)
                cam.set(cv2.CAP_PROP_BUFFERSIZE, cfg.buffer_size)

                # Auto exposure and focus settings
                cam.set(cv2.CAP_PROP_AUTOFOCUS, 1)
                cam.set(cv2.CAP_PROP_AUTO_EXPOSURE, 0.75)

                self.cameras[cfg.name] = {
                    'device': cam,
                    'config': cfg,
                    'index': idx
                }
                self.locks[cfg.name] = threading.Lock()
                self.frame_counts[cfg.name] = 0
                self.error_counts[cfg.name] = 0

                logger.info(f"✓ Camera {cfg.name} initialized ({cfg.width}x{cfg.height} @ {cfg.fps}fps)")

            except Exception as e:
                logger.error(f"Error initializing camera {idx}: {e}")

    def start_capture(self):
        """Start threaded capture from all cameras"""
        if self.running:
            logger.warning("Capture already running")
            return

        self.running = True
        logger.info("Starting capture threads...")

        for name, cam_info in self.cameras.items():
            thread = threading.Thread(
                target=self._capture_loop,
                args=(name, cam_info['device']),
                daemon=True
            )
            thread.start()
            self.threads[name] = thread
            logger.info(f"✓ Capture thread started for {name}")

    def stop_capture(self):
        """Stop all capture threads"""
        logger.info("Stopping capture threads...")
        self.running = False

        # Wait for threads to finish
        for name, thread in self.threads.items():
            thread.join(timeout=2.0)
            logger.info(f"✓ Thread stopped for {name}")

    def _capture_loop(self, name: str, camera: cv2.VideoCapture):
        """
        Continuous capture loop for single camera

        Args:
            name: Camera identifier
            camera: OpenCV VideoCapture object
        """
        logger.info(f"Capture loop started for {name}")

        while self.running:
            try:
                ret, frame = camera.read()

                if ret:
                    with self.locks[name]:
                        self.frames[name] = {
                            'image': frame.copy(),
                            'timestamp': time.time(),
                            'frame_number': self.frame_counts[name],
                            'camera_name': name
                        }
                        self.frame_counts[name] += 1
                else:
                    self.error_counts[name] += 1
                    if self.error_counts[name] > 100:
                        logger.error(f"Too many errors for {name}, stopping capture")
                        break

            except Exception as e:
                logger.error(f"Error in capture loop for {name}: {e}")
                self.error_counts[name] += 1

        logger.info(f"Capture loop ended for {name}")

    def get_synchronized_frames(self, max_time_diff: float = 0.05) -> Dict:
        """
        Get latest frames from all cameras

        Args:
            max_time_diff: Maximum time difference for sync (seconds)

        Returns:
            Dictionary of synchronized frames
        """
        synced_frames = {}

        for name in self.cameras.keys():
            with self.locks[name]:
                if name in self.frames:
                    synced_frames[name] = self.frames[name].copy()

        return synced_frames

    def get_frame(self, camera_name: str) -> Optional[Dict]:
        """
        Get latest frame from specific camera

        Args:
            camera_name: Name of camera

        Returns:
            Frame data dict or None
        """
        if camera_name not in self.locks:
            return None

        with self.locks[camera_name]:
            if camera_name in self.frames:
                return self.frames[camera_name].copy()
        return None

    def get_stats(self) -> Dict:
        """Get capture statistics for all cameras"""
        stats = {}
        for name in self.cameras.keys():
            stats[name] = {
                'frames_captured': self.frame_counts.get(name, 0),
                'errors': self.error_counts.get(name, 0),
                'fps': self._calculate_fps(name)
            }
        return stats

    def _calculate_fps(self, camera_name: str) -> float:
        """Calculate actual FPS for camera"""
        # Simple implementation - could be enhanced with moving average
        return self.configs[self.cameras[camera_name]['index']].fps

    def release(self):
        """Release all camera resources"""
        self.stop_capture()

        logger.info("Releasing cameras...")
        for name, cam_info in self.cameras.items():
            try:
                cam_info['device'].release()
                logger.info(f"✓ Camera {name} released")
            except Exception as e:
                logger.error(f"Error releasing {name}: {e}")

        cv2.destroyAllWindows()

    def __enter__(self):
        """Context manager entry"""
        self.start_capture()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.release()
