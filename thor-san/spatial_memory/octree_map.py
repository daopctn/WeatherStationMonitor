"""
Octree-based 3D spatial memory for Thor-san
Efficient storage and querying of 3D occupied space
"""
import numpy as np
from typing import List, Optional, Tuple, Set
import pickle
import logging
from dataclasses import dataclass

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class OctreeNode:
    """Single node in the octree"""
    center: np.ndarray  # 3D center position
    size: float  # Size of the cube
    children: List[Optional['OctreeNode']] = None
    occupied: bool = False
    color: Optional[np.ndarray] = None
    confidence: float = 0.0
    is_leaf: bool = True

    def __post_init__(self):
        if self.children is None:
            self.children = [None] * 8

    def get_octant(self, point: np.ndarray) -> int:
        """Determine which octant a point belongs to"""
        octant = 0
        for i in range(3):
            if point[i] > self.center[i]:
                octant |= (1 << i)
        return octant

    def get_child_center(self, octant: int) -> np.ndarray:
        """Calculate center position of child octant"""
        child_size = self.size / 2
        offset = child_size / 2
        child_center = self.center.copy()

        for i in range(3):
            if octant & (1 << i):
                child_center[i] += offset
            else:
                child_center[i] -= offset

        return child_center


class SpatialMemory:
    """
    Octree-based 3D spatial memory system

    Features:
    - Hierarchical 3D space representation
    - Efficient point cloud insertion
    - Region queries (sphere, box)
    - Occupancy checking
    - Persistent storage
    """

    def __init__(self, resolution: float = 0.01, world_size: float = 2.0):
        """
        Initialize spatial memory

        Args:
            resolution: Minimum voxel size in meters (default: 1cm)
            world_size: Size of root cube in meters (default: 2m)
        """
        self.resolution = resolution
        self.world_size = world_size

        # Root node centered at origin
        self.root = OctreeNode(
            center=np.array([0.0, 0.0, 0.0]),
            size=world_size
        )

        # Statistics
        self.point_count = 0
        self.node_count = 1

        logger.info(f"✓ Spatial memory initialized: {resolution*1000:.1f}mm resolution, {world_size}m world")

    def insert_point_cloud(self, points: np.ndarray, colors: Optional[np.ndarray] = None):
        """
        Insert point cloud into octree

        Args:
            points: Nx3 array of 3D points (meters)
            colors: Nx3 array of RGB colors (0-255)
        """
        if colors is None:
            colors = np.zeros((len(points), 3))

        for i, point in enumerate(points):
            color = colors[i] if colors is not None else None
            self._insert_point(self.root, point, color)
            self.point_count += 1

        logger.info(f"✓ Inserted {len(points)} points into spatial memory")

    def _insert_point(self, node: OctreeNode, point: np.ndarray, color: Optional[np.ndarray]):
        """
        Recursively insert point into octree

        Args:
            node: Current octree node
            point: 3D point coordinates
            color: RGB color
        """
        # Check if we've reached minimum resolution
        if node.size < self.resolution:
            node.occupied = True
            node.is_leaf = True
            if color is not None:
                # Average colors if multiple points in voxel
                if node.color is not None:
                    node.color = (node.color + color) / 2
                else:
                    node.color = color
            node.confidence = min(1.0, node.confidence + 0.1)
            return

        # Determine which octant the point belongs to
        octant = node.get_octant(point)

        # Create child if it doesn't exist
        if node.children[octant] is None:
            child_center = node.get_child_center(octant)
            node.children[octant] = OctreeNode(
                center=child_center,
                size=node.size / 2
            )
            node.is_leaf = False
            self.node_count += 1

        # Recursively insert into child
        self._insert_point(node.children[octant], point, color)

    def query_region_sphere(self, center: np.ndarray, radius: float) -> List[Tuple[np.ndarray, Optional[np.ndarray]]]:
        """
        Query all occupied points within sphere

        Args:
            center: Center of sphere (meters)
            radius: Radius of sphere (meters)

        Returns:
            List of (position, color) tuples
        """
        results = []
        self._query_sphere(self.root, center, radius, results)
        return results

    def _query_sphere(self, node: OctreeNode, center: np.ndarray, radius: float, results: List):
        """Recursively query sphere region"""
        if node is None:
            return

        # Check if node sphere intersects query sphere
        node_radius = node.size * np.sqrt(3) / 2  # Radius of bounding sphere
        distance = np.linalg.norm(node.center - center)

        if distance - node_radius > radius:
            return  # Node is completely outside query sphere

        # If leaf node and occupied, add to results
        if node.is_leaf and node.occupied:
            if distance <= radius:
                results.append((node.center.copy(), node.color))
            return

        # Recurse to children
        for child in node.children:
            if child is not None:
                self._query_sphere(child, center, radius, results)

    def query_region_box(self, min_point: np.ndarray, max_point: np.ndarray) -> List[Tuple[np.ndarray, Optional[np.ndarray]]]:
        """
        Query all occupied points within axis-aligned box

        Args:
            min_point: Minimum corner of box
            max_point: Maximum corner of box

        Returns:
            List of (position, color) tuples
        """
        results = []
        self._query_box(self.root, min_point, max_point, results)
        return results

    def _query_box(self, node: OctreeNode, min_point: np.ndarray, max_point: np.ndarray, results: List):
        """Recursively query box region"""
        if node is None:
            return

        # Check if node intersects query box
        half_size = node.size / 2
        node_min = node.center - half_size
        node_max = node.center + half_size

        # Check for intersection
        if np.any(node_max < min_point) or np.any(node_min > max_point):
            return  # No intersection

        # If leaf node and occupied, check if inside box
        if node.is_leaf and node.occupied:
            if np.all(node.center >= min_point) and np.all(node.center <= max_point):
                results.append((node.center.copy(), node.color))
            return

        # Recurse to children
        for child in node.children:
            if child is not None:
                self._query_box(child, min_point, max_point, results)

    def is_occupied(self, point: np.ndarray) -> bool:
        """
        Check if a point is occupied

        Args:
            point: 3D point coordinates

        Returns:
            True if point is occupied
        """
        return self._is_occupied(self.root, point)

    def _is_occupied(self, node: OctreeNode, point: np.ndarray) -> bool:
        """Recursively check occupancy"""
        if node is None:
            return False

        if node.is_leaf:
            return node.occupied

        octant = node.get_octant(point)
        return self._is_occupied(node.children[octant], point)

    def clear_region(self, center: np.ndarray, radius: float):
        """
        Clear all occupied voxels within radius of center

        Args:
            center: Center point
            radius: Radius to clear
        """
        self._clear_sphere(self.root, center, radius)

    def _clear_sphere(self, node: OctreeNode, center: np.ndarray, radius: float):
        """Recursively clear sphere region"""
        if node is None:
            return

        # Check if node intersects sphere
        node_radius = node.size * np.sqrt(3) / 2
        distance = np.linalg.norm(node.center - center)

        if distance - node_radius > radius:
            return

        # If leaf and within radius, clear it
        if node.is_leaf:
            if distance <= radius:
                node.occupied = False
                node.confidence = 0.0
            return

        # Recurse to children
        for child in node.children:
            if child is not None:
                self._clear_sphere(child, center, radius)

    def get_occupied_voxels(self) -> List[Tuple[np.ndarray, float, Optional[np.ndarray]]]:
        """
        Get all occupied voxels with their positions, sizes, and colors

        Returns:
            List of (center, size, color) tuples
        """
        voxels = []
        self._collect_voxels(self.root, voxels)
        return voxels

    def _collect_voxels(self, node: OctreeNode, voxels: List):
        """Recursively collect occupied voxels"""
        if node is None:
            return

        if node.is_leaf and node.occupied:
            voxels.append((node.center.copy(), node.size, node.color))
            return

        for child in node.children:
            if child is not None:
                self._collect_voxels(child, voxels)

    def save(self, filename: str):
        """
        Save octree to file

        Args:
            filename: Output filename (.pkl)
        """
        data = {
            'root': self.root,
            'resolution': self.resolution,
            'world_size': self.world_size,
            'point_count': self.point_count,
            'node_count': self.node_count
        }

        with open(filename, 'wb') as f:
            pickle.dump(data, f)

        logger.info(f"✓ Spatial memory saved to {filename}")

    def load(self, filename: str):
        """
        Load octree from file

        Args:
            filename: Input filename (.pkl)
        """
        with open(filename, 'rb') as f:
            data = pickle.load(f)

        self.root = data['root']
        self.resolution = data['resolution']
        self.world_size = data['world_size']
        self.point_count = data['point_count']
        self.node_count = data['node_count']

        logger.info(f"✓ Spatial memory loaded from {filename}")

    def get_statistics(self) -> dict:
        """Get statistics about the octree"""
        voxels = self.get_occupied_voxels()

        return {
            'total_points_inserted': self.point_count,
            'total_nodes': self.node_count,
            'occupied_voxels': len(voxels),
            'resolution_mm': self.resolution * 1000,
            'world_size_m': self.world_size
        }
