"""
Scene graph for spatial relationships between objects
Represents objects and their spatial/semantic relationships
"""
import numpy as np
from typing import Dict, List, Optional, Tuple, Set
from dataclasses import dataclass
from enum import Enum
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class RelationType(Enum):
    """Types of spatial relationships"""
    ON_TOP_OF = "on_top_of"
    UNDER = "under"
    LEFT_OF = "left_of"
    RIGHT_OF = "right_of"
    IN_FRONT_OF = "in_front_of"
    BEHIND = "behind"
    INSIDE = "inside"
    NEAR = "near"
    FAR_FROM = "far_from"
    TOUCHING = "touching"


@dataclass
class SpatialRelation:
    """Relationship between two objects"""
    object_a_id: int
    object_b_id: int
    relation_type: RelationType
    confidence: float = 1.0
    distance: Optional[float] = None

    def __str__(self):
        return f"Object {self.object_a_id} {self.relation_type.value} Object {self.object_b_id}"


class SceneGraph:
    """
    Scene graph representing objects and their relationships

    Features:
    - Track spatial relationships
    - Query relationships
    - Update relationships dynamically
    - Support semantic reasoning
    """

    def __init__(self):
        """Initialize scene graph"""
        self.objects: Dict[int, Dict] = {}  # object_id -> object data
        self.relations: List[SpatialRelation] = []

        # Distance thresholds for relationships (meters)
        self.near_threshold = 0.3
        self.touching_threshold = 0.05

    def add_object(self, object_id: int, position: np.ndarray,
                  class_name: str, dimensions: Optional[np.ndarray] = None):
        """
        Add object to scene graph

        Args:
            object_id: Unique object identifier
            position: 3D position (x, y, z)
            class_name: Object class name
            dimensions: Optional object dimensions (width, height, depth)
        """
        self.objects[object_id] = {
            'position': position,
            'class_name': class_name,
            'dimensions': dimensions if dimensions is not None else np.array([0.1, 0.1, 0.1])
        }

    def remove_object(self, object_id: int):
        """Remove object and all its relationships"""
        if object_id in self.objects:
            del self.objects[object_id]

        # Remove all relations involving this object
        self.relations = [
            r for r in self.relations
            if r.object_a_id != object_id and r.object_b_id != object_id
        ]

    def update_object_position(self, object_id: int, position: np.ndarray):
        """Update object position"""
        if object_id in self.objects:
            self.objects[object_id]['position'] = position

    def add_relation(self, relation: SpatialRelation):
        """Add spatial relationship"""
        # Remove existing relation of same type between same objects
        self.relations = [
            r for r in self.relations
            if not (r.object_a_id == relation.object_a_id and
                   r.object_b_id == relation.object_b_id and
                   r.relation_type == relation.relation_type)
        ]

        self.relations.append(relation)

    def compute_all_relations(self):
        """Compute spatial relationships between all objects"""
        self.relations.clear()

        object_ids = list(self.objects.keys())

        for i, id_a in enumerate(object_ids):
            for id_b in object_ids[i+1:]:
                relations = self._compute_relations_between(id_a, id_b)
                self.relations.extend(relations)

        logger.info(f"✓ Computed {len(self.relations)} spatial relations")

    def _compute_relations_between(self, id_a: int, id_b: int) -> List[SpatialRelation]:
        """Compute relationships between two objects"""
        relations = []

        obj_a = self.objects[id_a]
        obj_b = self.objects[id_b]

        pos_a = obj_a['position']
        pos_b = obj_b['position']
        dim_a = obj_a['dimensions']
        dim_b = obj_b['dimensions']

        # Calculate distance
        distance = np.linalg.norm(pos_a - pos_b)

        # Distance-based relations
        if distance < self.touching_threshold:
            relations.append(SpatialRelation(id_a, id_b, RelationType.TOUCHING, distance=distance))
        elif distance < self.near_threshold:
            relations.append(SpatialRelation(id_a, id_b, RelationType.NEAR, distance=distance))
        else:
            relations.append(SpatialRelation(id_a, id_b, RelationType.FAR_FROM, distance=distance))

        # Vertical relationships (based on Z axis)
        z_diff = pos_a[2] - pos_b[2]
        threshold = (dim_a[2] + dim_b[2]) / 4

        if z_diff > threshold:
            relations.append(SpatialRelation(id_a, id_b, RelationType.ON_TOP_OF))
            relations.append(SpatialRelation(id_b, id_a, RelationType.UNDER))

        # Horizontal relationships (X axis - left/right)
        x_diff = pos_a[0] - pos_b[0]
        if abs(x_diff) > 0.1:
            if x_diff > 0:
                relations.append(SpatialRelation(id_a, id_b, RelationType.RIGHT_OF))
                relations.append(SpatialRelation(id_b, id_a, RelationType.LEFT_OF))
            else:
                relations.append(SpatialRelation(id_a, id_b, RelationType.LEFT_OF))
                relations.append(SpatialRelation(id_b, id_a, RelationType.RIGHT_OF))

        # Depth relationships (Y axis - front/behind)
        y_diff = pos_a[1] - pos_b[1]
        if abs(y_diff) > 0.1:
            if y_diff > 0:
                relations.append(SpatialRelation(id_a, id_b, RelationType.IN_FRONT_OF))
                relations.append(SpatialRelation(id_b, id_a, RelationType.BEHIND))
            else:
                relations.append(SpatialRelation(id_a, id_b, RelationType.BEHIND))
                relations.append(SpatialRelation(id_b, id_a, RelationType.IN_FRONT_OF))

        return relations

    def get_relations(self, object_id: int,
                     relation_type: Optional[RelationType] = None) -> List[SpatialRelation]:
        """
        Get all relations involving an object

        Args:
            object_id: Object to query
            relation_type: Optional filter by relation type

        Returns:
            List of relations
        """
        results = [
            r for r in self.relations
            if r.object_a_id == object_id or r.object_b_id == object_id
        ]

        if relation_type is not None:
            results = [r for r in results if r.relation_type == relation_type]

        return results

    def find_objects_with_relation(self, object_id: int,
                                   relation_type: RelationType) -> List[int]:
        """
        Find all objects having a specific relationship with given object

        Args:
            object_id: Reference object
            relation_type: Type of relationship

        Returns:
            List of object IDs
        """
        results = []

        for relation in self.relations:
            if relation.relation_type == relation_type:
                if relation.object_a_id == object_id:
                    results.append(relation.object_b_id)
                elif relation.object_b_id == object_id:
                    # For symmetric relations
                    if relation_type in [RelationType.NEAR, RelationType.FAR_FROM, RelationType.TOUCHING]:
                        results.append(relation.object_a_id)

        return results

    def get_nearest_object(self, object_id: int,
                          class_filter: Optional[List[str]] = None) -> Optional[int]:
        """
        Find nearest object to given object

        Args:
            object_id: Reference object
            class_filter: Optional list of class names to filter by

        Returns:
            ID of nearest object or None
        """
        if object_id not in self.objects:
            return None

        pos = self.objects[object_id]['position']
        min_distance = float('inf')
        nearest_id = None

        for other_id, obj_data in self.objects.items():
            if other_id == object_id:
                continue

            # Apply class filter
            if class_filter and obj_data['class_name'] not in class_filter:
                continue

            distance = np.linalg.norm(obj_data['position'] - pos)
            if distance < min_distance:
                min_distance = distance
                nearest_id = other_id

        return nearest_id

    def query_by_class(self, class_name: str) -> List[int]:
        """Get all objects of a specific class"""
        return [
            obj_id for obj_id, data in self.objects.items()
            if data['class_name'] == class_name
        ]

    def describe_scene(self) -> str:
        """Generate textual description of scene"""
        description = f"Scene contains {len(self.objects)} objects:\n"

        for obj_id, data in self.objects.items():
            pos = data['position']
            description += f"  - Object {obj_id} ({data['class_name']}) at ({pos[0]:.2f}, {pos[1]:.2f}, {pos[2]:.2f})\n"

        description += f"\nSpatial relations ({len(self.relations)}):\n"

        for relation in self.relations[:20]:  # Limit to first 20
            obj_a_class = self.objects[relation.object_a_id]['class_name']
            obj_b_class = self.objects[relation.object_b_id]['class_name']
            description += f"  - {obj_a_class} (#{relation.object_a_id}) {relation.relation_type.value} {obj_b_class} (#{relation.object_b_id})\n"

        return description

    def clear(self):
        """Clear all objects and relations"""
        self.objects.clear()
        self.relations.clear()

    def get_statistics(self) -> Dict:
        """Get scene graph statistics"""
        return {
            'num_objects': len(self.objects),
            'num_relations': len(self.relations),
            'relation_types': {
                rt.value: sum(1 for r in self.relations if r.relation_type == rt)
                for rt in RelationType
            }
        }
