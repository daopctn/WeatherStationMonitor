"""
Camera capture and synchronization modules
"""

from .multi_camera import MultiCameraSystem, CameraConfig
from .synchronizer import FrameSynchronizer

__all__ = ['MultiCameraSystem', 'CameraConfig', 'FrameSynchronizer']
