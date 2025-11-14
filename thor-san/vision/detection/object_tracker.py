"""
Object tracking across frames using simple centroid tracking
Maintains object identity across multiple frames
"""
import numpy as np
from typing import Dict, List, Optional
from dataclasses import dataclass, field
from scipy.spatial import distance as dist
from collections import OrderedDict


@dataclass
class TrackedObject:
    """Tracked object across multiple frames"""
    object_id: int
    class_name: str
    bbox: tuple
    center: tuple
    confidence: float
    frames_tracked: int = 1
    frames_missing: int = 0
    velocity: tuple = (0.0, 0.0)
    history: List[tuple] = field(default_factory=list)

    def update(self, bbox: tuple, center: tuple, confidence: float):
        """Update tracked object with new detection"""
        # Calculate velocity
        if self.center:
            self.velocity = (
                center[0] - self.center[0],
                center[1] - self.center[1]
            )

        self.bbox = bbox
        self.history.append(center)
        self.center = center
        self.confidence = confidence
        self.frames_tracked += 1
        self.frames_missing = 0

        # Limit history size
        if len(self.history) > 30:
            self.history.pop(0)


class ObjectTracker:
    """
    Simple centroid-based object tracker

    Features:
    - Tracks objects across frames using centroid distance
    - Handles object appearance/disappearance
    - Maintains object IDs
    - Calculates object velocities
    """

    def __init__(self, max_disappeared: int = 30, max_distance: float = 50.0):
        """
        Initialize object tracker

        Args:
            max_disappeared: Max frames an object can be missing before deletion
            max_distance: Max distance for matching objects between frames (pixels)
        """
        self.next_object_id = 0
        self.objects: Dict[int, TrackedObject] = OrderedDict()
        self.max_disappeared = max_disappeared
        self.max_distance = max_distance

    def update(self, detections: List) -> Dict[int, TrackedObject]:
        """
        Update tracker with new detections

        Args:
            detections: List of Detection objects from current frame

        Returns:
            Dictionary of tracked objects (id -> TrackedObject)
        """
        # If no detections, increment disappeared counter
        if len(detections) == 0:
            for obj_id in list(self.objects.keys()):
                self.objects[obj_id].frames_missing += 1

                # Deregister if missing too long
                if self.objects[obj_id].frames_missing > self.max_disappeared:
                    self._deregister(obj_id)

            return self.objects

        # Extract centers from detections
        input_centroids = np.array([d.center for d in detections])

        # If no existing objects, register all detections
        if len(self.objects) == 0:
            for i, det in enumerate(detections):
                self._register(det)
        else:
            # Get existing object IDs and centroids
            object_ids = list(self.objects.keys())
            object_centroids = np.array([self.objects[oid].center for oid in object_ids])

            # Compute distance between each pair
            D = dist.cdist(object_centroids, input_centroids)

            # Find minimum distance for each existing object
            rows = D.min(axis=1).argsort()
            cols = D.argmin(axis=1)[rows]

            used_rows = set()
            used_cols = set()

            # Match objects
            for (row, col) in zip(rows, cols):
                if row in used_rows or col in used_cols:
                    continue

                # Check if distance is within threshold
                if D[row, col] > self.max_distance:
                    continue

                # Update existing object
                obj_id = object_ids[row]
                det = detections[col]
                self.objects[obj_id].update(det.bbox, det.center, det.confidence)

                used_rows.add(row)
                used_cols.add(col)

            # Handle unmatched objects (disappeared)
            unused_rows = set(range(D.shape[0])) - used_rows
            for row in unused_rows:
                obj_id = object_ids[row]
                self.objects[obj_id].frames_missing += 1

                if self.objects[obj_id].frames_missing > self.max_disappeared:
                    self._deregister(obj_id)

            # Handle unmatched detections (new objects)
            unused_cols = set(range(D.shape[1])) - used_cols
            for col in unused_cols:
                self._register(detections[col])

        return self.objects

    def _register(self, detection):
        """Register new object"""
        tracked_obj = TrackedObject(
            object_id=self.next_object_id,
            class_name=detection.class_name,
            bbox=detection.bbox,
            center=detection.center,
            confidence=detection.confidence
        )
        self.objects[self.next_object_id] = tracked_obj
        self.next_object_id += 1

    def _deregister(self, object_id: int):
        """Remove object from tracking"""
        del self.objects[object_id]

    def get_object(self, object_id: int) -> Optional[TrackedObject]:
        """Get tracked object by ID"""
        return self.objects.get(object_id)

    def get_all_objects(self) -> Dict[int, TrackedObject]:
        """Get all currently tracked objects"""
        return self.objects

    def get_objects_by_class(self, class_name: str) -> List[TrackedObject]:
        """Get all tracked objects of specific class"""
        return [obj for obj in self.objects.values() if obj.class_name == class_name]

    def reset(self):
        """Reset tracker"""
        self.objects.clear()
        self.next_object_id = 0
