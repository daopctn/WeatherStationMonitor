"""
Grasp planning for robotic manipulation
Generates grasp poses for objects in 3D space
"""
import numpy as np
from typing import List, Optional, Tuple
from dataclasses import dataclass
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class GraspPose:
    """6-DOF grasp pose"""
    position: np.ndarray  # 3D position (x, y, z)
    orientation: np.ndarray  # Quaternion (x, y, z, w)
    approach_direction: np.ndarray  # Unit vector
    grasp_width: float  # Gripper opening width
    quality_score: float  # 0-1 quality rating
    object_id: Optional[int] = None


class GraspPlanner:
    """
    Grasp planning system

    Features:
    - Generate grasp candidates
    - Score grasp quality
    - Check collision with environment
    - Select best grasp
    """

    def __init__(self, gripper_width: float = 0.085):
        """
        Initialize grasp planner

        Args:
            gripper_width: Maximum gripper opening (meters)
        """
        self.gripper_width = gripper_width
        self.min_grasp_width = 0.02
        self.approach_distance = 0.15  # Distance to start approach

    def plan_grasps(self, object_position: np.ndarray,
                   object_dimensions: np.ndarray,
                   spatial_memory,
                   num_candidates: int = 10) -> List[GraspPose]:
        """
        Generate grasp candidates for an object

        Args:
            object_position: Object center position
            object_dimensions: Object size (width, height, depth)
            spatial_memory: SpatialMemory for collision checking
            num_candidates: Number of grasp candidates to generate

        Returns:
            List of grasp poses sorted by quality
        """
        logger.info(f"Planning grasps for object at {object_position}")

        grasps = []

        # Generate grasps from different approach directions
        approach_directions = [
            np.array([0, 0, -1]),   # Top-down
            np.array([1, 0, 0]),    # From +X
            np.array([-1, 0, 0]),   # From -X
            np.array([0, 1, 0]),    # From +Y
            np.array([0, -1, 0]),   # From -Y
        ]

        for direction in approach_directions:
            grasp = self._generate_grasp_from_direction(
                object_position,
                object_dimensions,
                direction,
                spatial_memory
            )

            if grasp and grasp.quality_score > 0.3:
                grasps.append(grasp)

        # Add angled grasps
        for angle in [45, -45]:
            angle_rad = np.deg2rad(angle)
            direction = np.array([
                np.cos(angle_rad), 0, np.sin(angle_rad)
            ])
            grasp = self._generate_grasp_from_direction(
                object_position,
                object_dimensions,
                direction,
                spatial_memory
            )
            if grasp and grasp.quality_score > 0.3:
                grasps.append(grasp)

        # Sort by quality
        grasps.sort(key=lambda g: g.quality_score, reverse=True)

        logger.info(f"✓ Generated {len(grasps)} valid grasps")

        return grasps[:num_candidates]

    def _generate_grasp_from_direction(self, position: np.ndarray,
                                      dimensions: np.ndarray,
                                      direction: np.ndarray,
                                      spatial_memory) -> Optional[GraspPose]:
        """
        Generate single grasp from approach direction

        Args:
            position: Object center
            dimensions: Object dimensions
            direction: Approach direction (unit vector)
            spatial_memory: For collision checking

        Returns:
            GraspPose or None if invalid
        """
        # Normalize direction
        direction = direction / np.linalg.norm(direction)

        # Calculate grasp position (offset from center)
        grasp_offset = -direction * (dimensions[2] / 2 + 0.02)  # 2cm clearance
        grasp_position = position + grasp_offset

        # Calculate grasp width based on object size
        # Choose smallest perpendicular dimension
        perp_dims = []
        for i in range(3):
            if abs(direction[i]) < 0.1:  # Perpendicular to approach
                perp_dims.append(dimensions[i])

        if perp_dims:
            grasp_width = min(perp_dims)
        else:
            grasp_width = min(dimensions)

        # Check if graspable
        if grasp_width < self.min_grasp_width or grasp_width > self.gripper_width:
            return None

        # Calculate orientation (quaternion)
        # For simplicity, align Z-axis with approach direction
        orientation = self._direction_to_quaternion(direction)

        # Calculate quality score
        quality = self._calculate_grasp_quality(
            grasp_position,
            direction,
            grasp_width,
            position,
            spatial_memory
        )

        grasp = GraspPose(
            position=grasp_position,
            orientation=orientation,
            approach_direction=direction,
            grasp_width=grasp_width,
            quality_score=quality
        )

        return grasp

    def _direction_to_quaternion(self, direction: np.ndarray) -> np.ndarray:
        """
        Convert direction vector to quaternion

        Args:
            direction: Approach direction vector

        Returns:
            Quaternion (x, y, z, w)
        """
        # Simplified: align with direction
        # In practice, would need proper rotation matrix conversion

        # Default orientation
        z_axis = direction / np.linalg.norm(direction)

        # Find perpendicular vector for x-axis
        if abs(z_axis[2]) < 0.9:
            x_axis = np.cross(z_axis, np.array([0, 0, 1]))
        else:
            x_axis = np.cross(z_axis, np.array([1, 0, 0]))

        x_axis = x_axis / np.linalg.norm(x_axis)

        # Y-axis completes the frame
        y_axis = np.cross(z_axis, x_axis)

        # Convert to quaternion (simplified)
        # For full implementation, use rotation matrix to quaternion conversion
        # Placeholder: identity quaternion
        return np.array([0, 0, 0, 1])

    def _calculate_grasp_quality(self, grasp_position: np.ndarray,
                                 approach_direction: np.ndarray,
                                 grasp_width: float,
                                 object_position: np.ndarray,
                                 spatial_memory) -> float:
        """
        Calculate grasp quality score

        Args:
            grasp_position: Grasp position
            approach_direction: Approach direction
            grasp_width: Gripper width
            object_position: Object center
            spatial_memory: For collision checking

        Returns:
            Quality score (0-1)
        """
        quality = 0.5  # Base score

        # Prefer top-down grasps (more stable)
        if approach_direction[2] < -0.7:  # Pointing down
            quality += 0.2

        # Prefer centered grasps
        offset_from_center = np.linalg.norm(grasp_position - object_position)
        centering_score = max(0, 1.0 - offset_from_center / 0.1)
        quality += centering_score * 0.2

        # Check approach path for collisions
        approach_start = grasp_position - approach_direction * self.approach_distance
        collision_free = self._check_approach_collision_free(
            approach_start,
            grasp_position,
            spatial_memory
        )

        if collision_free:
            quality += 0.2
        else:
            quality -= 0.3

        # Prefer moderate grasp widths
        width_ratio = grasp_width / self.gripper_width
        if 0.3 < width_ratio < 0.8:
            quality += 0.1

        return np.clip(quality, 0.0, 1.0)

    def _check_approach_collision_free(self, start: np.ndarray,
                                       end: np.ndarray,
                                       spatial_memory) -> bool:
        """
        Check if approach path is collision-free

        Args:
            start: Start position
            end: End position
            spatial_memory: For occupancy checking

        Returns:
            True if collision-free
        """
        # Sample points along path
        num_samples = 10
        for i in range(num_samples):
            t = i / (num_samples - 1)
            point = start + t * (end - start)

            # Check occupancy
            if spatial_memory.is_occupied(point):
                return False

        return True

    def select_best_grasp(self, grasps: List[GraspPose]) -> Optional[GraspPose]:
        """
        Select best grasp from candidates

        Args:
            grasps: List of grasp candidates

        Returns:
            Best grasp or None
        """
        if not grasps:
            return None

        # Already sorted by quality
        return grasps[0]

    def filter_reachable_grasps(self, grasps: List[GraspPose],
                               robot_position: np.ndarray,
                               max_reach: float = 0.8) -> List[GraspPose]:
        """
        Filter grasps by reachability

        Args:
            grasps: List of grasp candidates
            robot_position: Robot base position
            max_reach: Maximum reach distance

        Returns:
            Filtered list of reachable grasps
        """
        reachable = []

        for grasp in grasps:
            distance = np.linalg.norm(grasp.position[:2] - robot_position[:2])
            if distance <= max_reach:
                reachable.append(grasp)

        return reachable

    def visualize_grasp(self, grasp: GraspPose) -> Dict:
        """
        Generate visualization data for grasp

        Args:
            grasp: Grasp pose to visualize

        Returns:
            Dictionary with visualization data
        """
        # Gripper finger positions
        finger_width = grasp.grasp_width / 2

        # Simplified: fingers perpendicular to approach
        finger_direction = np.array([1, 0, 0])  # Placeholder

        finger1_pos = grasp.position + finger_direction * finger_width
        finger2_pos = grasp.position - finger_direction * finger_width

        return {
            'grasp_position': grasp.position.tolist(),
            'approach_start': (grasp.position - grasp.approach_direction * 0.1).tolist(),
            'finger1': finger1_pos.tolist(),
            'finger2': finger2_pos.tolist(),
            'quality': grasp.quality_score
        }
