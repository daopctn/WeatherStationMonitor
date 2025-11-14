"""
Scene understanding and analysis
Interprets 3D environment and detected objects
"""
import numpy as np
from typing import List, Dict, Optional, Tuple
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class SceneAnalyzer:
    """
    Analyzes 3D scenes for high-level understanding

    Features:
    - Identify surfaces (tables, floors, walls)
    - Find graspable objects
    - Detect obstacles
    - Estimate object stability
    """

    def __init__(self):
        """Initialize scene analyzer"""
        self.ground_plane = None
        self.surfaces = []
        self.obstacles = []

    def analyze_point_cloud(self, points: np.ndarray, colors: Optional[np.ndarray] = None) -> Dict:
        """
        Analyze point cloud to understand scene structure

        Args:
            points: Nx3 array of 3D points
            colors: Nx3 array of RGB colors

        Returns:
            Dictionary with scene analysis results
        """
        logger.info("Analyzing scene from point cloud...")

        analysis = {
            'ground_plane': None,
            'surfaces': [],
            'point_density': 0,
            'bounds': None
        }

        if len(points) == 0:
            return analysis

        # Find ground plane (lowest dominant horizontal surface)
        ground_plane = self._find_ground_plane(points)
        analysis['ground_plane'] = ground_plane

        # Find horizontal surfaces (tables, shelves)
        surfaces = self._find_horizontal_surfaces(points, ground_plane)
        analysis['surfaces'] = surfaces

        # Calculate point density
        bounds = self._compute_bounds(points)
        volume = np.prod(bounds[1] - bounds[0])
        analysis['point_density'] = len(points) / volume if volume > 0 else 0
        analysis['bounds'] = bounds

        logger.info(f"✓ Found ground plane at Z={ground_plane:.3f}m")
        logger.info(f"✓ Found {len(surfaces)} horizontal surfaces")

        return analysis

    def _find_ground_plane(self, points: np.ndarray) -> float:
        """
        Find ground plane Z coordinate using RANSAC

        Args:
            points: Point cloud

        Returns:
            Z coordinate of ground plane
        """
        # Simple approach: use histogram of Z values
        z_values = points[:, 2]
        hist, edges = np.histogram(z_values, bins=50)

        # Find most common Z value (likely ground)
        max_bin = np.argmax(hist)
        ground_z = (edges[max_bin] + edges[max_bin + 1]) / 2

        return ground_z

    def _find_horizontal_surfaces(self, points: np.ndarray, ground_z: float) -> List[Dict]:
        """
        Find horizontal surfaces above ground plane

        Args:
            points: Point cloud
            ground_z: Ground plane Z coordinate

        Returns:
            List of surface dictionaries
        """
        surfaces = []

        # Filter points above ground
        above_ground = points[points[:, 2] > ground_z + 0.1]

        if len(above_ground) == 0:
            return surfaces

        # Cluster by Z coordinate (height levels)
        z_values = above_ground[:, 2]
        hist, edges = np.histogram(z_values, bins=30)

        # Find peaks in histogram (likely surfaces)
        threshold = len(above_ground) * 0.02  # 2% of points

        for i, count in enumerate(hist):
            if count > threshold:
                z_min = edges[i]
                z_max = edges[i + 1]
                z_center = (z_min + z_max) / 2

                # Get points at this height
                surface_points = above_ground[
                    (above_ground[:, 2] >= z_min) &
                    (above_ground[:, 2] <= z_max)
                ]

                if len(surface_points) > 100:
                    # Calculate surface bounds
                    x_min, y_min = surface_points[:, :2].min(axis=0)
                    x_max, y_max = surface_points[:, :2].max(axis=0)

                    surfaces.append({
                        'z': z_center,
                        'height_above_ground': z_center - ground_z,
                        'bounds': ((x_min, y_min), (x_max, y_max)),
                        'area': (x_max - x_min) * (y_max - y_min),
                        'point_count': len(surface_points)
                    })

        # Sort by height
        surfaces.sort(key=lambda s: s['z'])

        return surfaces

    def _compute_bounds(self, points: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """Compute axis-aligned bounding box"""
        min_point = points.min(axis=0)
        max_point = points.max(axis=0)
        return (min_point, max_point)

    def find_reachable_surfaces(self, surfaces: List[Dict],
                               robot_position: np.ndarray,
                               max_reach: float = 0.8) -> List[Dict]:
        """
        Find surfaces within robot reach

        Args:
            surfaces: List of surface dicts
            robot_position: Robot base position (x, y, z)
            max_reach: Maximum reach distance in meters

        Returns:
            List of reachable surfaces
        """
        reachable = []

        for surface in surfaces:
            # Calculate distance from robot to surface center
            surface_center = np.array([
                (surface['bounds'][0][0] + surface['bounds'][1][0]) / 2,
                (surface['bounds'][0][1] + surface['bounds'][1][1]) / 2,
                surface['z']
            ])

            distance = np.linalg.norm(surface_center[:2] - robot_position[:2])

            if distance <= max_reach:
                surface['distance_from_robot'] = distance
                reachable.append(surface)

        return reachable

    def identify_graspable_objects(self, detections: List,
                                   spatial_memory,
                                   min_size: float = 0.02,
                                   max_size: float = 0.3) -> List[Dict]:
        """
        Identify objects that can be grasped

        Args:
            detections: List of object detections
            spatial_memory: SpatialMemory instance
            min_size: Minimum object size (meters)
            max_size: Maximum object size (meters)

        Returns:
            List of graspable object dicts
        """
        graspable = []

        for det in detections:
            # Estimate object size from bounding box
            bbox = det.bbox
            size = max(bbox[2] - bbox[0], bbox[3] - bbox[1])

            # Convert pixel size to approximate metric size (rough estimate)
            # This would need camera calibration for accuracy
            estimated_size = size * 0.001  # Very rough approximation

            if min_size <= estimated_size <= max_size:
                graspable.append({
                    'detection': det,
                    'estimated_size': estimated_size,
                    'position': det.center,
                    'graspability_score': self._calculate_graspability(det)
                })

        return graspable

    def _calculate_graspability(self, detection) -> float:
        """
        Calculate how graspable an object is (0-1)

        Args:
            detection: Detection object

        Returns:
            Graspability score
        """
        # Simple heuristic based on object class and size
        graspable_classes = [
            'bottle', 'cup', 'bowl', 'book', 'cell phone',
            'mouse', 'keyboard', 'remote', 'vase', 'scissors'
        ]

        score = 0.5  # Base score

        if detection.class_name in graspable_classes:
            score += 0.3

        # Prefer higher confidence
        score += detection.confidence * 0.2

        # Prefer moderate sizes (area-based)
        area_score = min(detection.area / 50000, 1.0)
        score += area_score * 0.1

        return min(score, 1.0)

    def check_collision_free_path(self, start: np.ndarray, end: np.ndarray,
                                  spatial_memory) -> bool:
        """
        Check if path between two points is collision-free

        Args:
            start: Start position (x, y, z)
            end: End position (x, y, z)
            spatial_memory: SpatialMemory instance

        Returns:
            True if path is collision-free
        """
        # Sample points along path
        num_samples = 20
        for i in range(num_samples):
            t = i / (num_samples - 1)
            point = start + t * (end - start)

            # Check if point is occupied
            if spatial_memory.is_occupied(point):
                return False

        return True

    def find_placement_locations(self, surface: Dict,
                                spatial_memory,
                                object_size: Tuple[float, float]) -> List[np.ndarray]:
        """
        Find suitable placement locations on a surface

        Args:
            surface: Surface dictionary
            spatial_memory: SpatialMemory instance
            object_size: Object dimensions (width, depth)

        Returns:
            List of valid placement positions
        """
        placements = []

        # Sample grid on surface
        x_min, y_min = surface['bounds'][0]
        x_max, y_max = surface['bounds'][1]
        z = surface['z']

        grid_size = 0.05  # 5cm grid

        x_samples = np.arange(x_min, x_max, grid_size)
        y_samples = np.arange(y_min, y_max, grid_size)

        for x in x_samples:
            for y in y_samples:
                position = np.array([x, y, z])

                # Check if area is free
                # Simple check: query small region around position
                occupied_voxels = spatial_memory.query_region_sphere(position, 0.03)

                if len(occupied_voxels) == 0:
                    placements.append(position)

        return placements

    def estimate_object_stability(self, object_position: np.ndarray,
                                  support_surface: Dict) -> float:
        """
        Estimate how stable an object is on a surface

        Args:
            object_position: Object center position
            support_surface: Surface dictionary

        Returns:
            Stability score (0-1)
        """
        # Check if object is above surface
        if object_position[2] < support_surface['z']:
            return 0.0

        # Check if object is within surface bounds
        x, y = object_position[:2]
        x_min, y_min = support_surface['bounds'][0]
        x_max, y_max = support_surface['bounds'][1]

        if not (x_min <= x <= x_max and y_min <= y <= y_max):
            return 0.0

        # Distance from surface edge
        dist_to_edge = min(
            x - x_min, x_max - x,
            y - y_min, y_max - y
        )

        # Normalize (1.0 at center, lower near edges)
        max_dist = min(x_max - x_min, y_max - y_min) / 2
        stability = min(dist_to_edge / max_dist, 1.0) if max_dist > 0 else 0.0

        return stability
