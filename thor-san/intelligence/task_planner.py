"""
High-level task planning for robotic manipulation
Plans sequences of actions to achieve goals
"""
import numpy as np
from typing import List, Dict, Optional, Callable
from dataclasses import dataclass, field
from enum import Enum
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class TaskStatus(Enum):
    """Status of a task"""
    PENDING = "pending"
    PLANNING = "planning"
    READY = "ready"
    EXECUTING = "executing"
    COMPLETED = "completed"
    FAILED = "failed"
    CANCELLED = "cancelled"


class ActionType(Enum):
    """Types of primitive actions"""
    MOVE_TO = "move_to"
    GRASP = "grasp"
    RELEASE = "release"
    PLACE = "place"
    WAIT = "wait"
    OBSERVE = "observe"


@dataclass
class Action:
    """Single primitive action"""
    action_type: ActionType
    parameters: Dict
    duration: float = 1.0
    preconditions: List[str] = field(default_factory=list)
    effects: List[str] = field(default_factory=list)


@dataclass
class Task:
    """High-level task with action sequence"""
    task_id: int
    description: str
    goal: str
    actions: List[Action] = field(default_factory=list)
    status: TaskStatus = TaskStatus.PENDING
    current_action_index: int = 0
    metadata: Dict = field(default_factory=dict)

    def add_action(self, action: Action):
        """Add action to task"""
        self.actions.append(action)

    def get_current_action(self) -> Optional[Action]:
        """Get currently executing action"""
        if 0 <= self.current_action_index < len(self.actions):
            return self.actions[self.current_action_index]
        return None

    def advance_action(self):
        """Move to next action"""
        self.current_action_index += 1
        if self.current_action_index >= len(self.actions):
            self.status = TaskStatus.COMPLETED

    def is_complete(self) -> bool:
        """Check if task is complete"""
        return self.status == TaskStatus.COMPLETED


class TaskPlanner:
    """
    High-level task planning system

    Features:
    - Plan pick-and-place tasks
    - Generate action sequences
    - Check preconditions
    - Handle failure recovery
    """

    def __init__(self):
        """Initialize task planner"""
        self.tasks: List[Task] = []
        self.next_task_id = 0
        self.current_task: Optional[Task] = None

    def plan_pick_and_place(self, object_class: str,
                           target_location: np.ndarray,
                           scene_graph,
                           spatial_memory) -> Task:
        """
        Plan a pick and place task

        Args:
            object_class: Class of object to pick
            target_location: Where to place object
            scene_graph: SceneGraph instance
            spatial_memory: SpatialMemory instance

        Returns:
            Planned task
        """
        task = Task(
            task_id=self.next_task_id,
            description=f"Pick {object_class} and place at target",
            goal=f"object_{object_class}_at_target"
        )
        self.next_task_id += 1

        logger.info(f"Planning pick-and-place task for {object_class}")

        # Find target object
        object_ids = scene_graph.query_by_class(object_class)

        if not object_ids:
            logger.error(f"No {object_class} found in scene")
            task.status = TaskStatus.FAILED
            return task

        # Choose closest object
        target_obj_id = object_ids[0]
        obj_position = scene_graph.objects[target_obj_id]['position']

        # Action 1: Observe scene
        task.add_action(Action(
            action_type=ActionType.OBSERVE,
            parameters={'target': object_class},
            duration=0.5
        ))

        # Action 2: Move to pre-grasp position
        pre_grasp_pos = obj_position + np.array([0, 0, 0.1])  # 10cm above
        task.add_action(Action(
            action_type=ActionType.MOVE_TO,
            parameters={'position': pre_grasp_pos, 'speed': 'slow'},
            duration=2.0,
            preconditions=['workspace_clear'],
            effects=['at_pre_grasp']
        ))

        # Action 3: Grasp object
        task.add_action(Action(
            action_type=ActionType.GRASP,
            parameters={
                'object_id': target_obj_id,
                'position': obj_position,
                'approach_direction': np.array([0, 0, -1])
            },
            duration=1.5,
            preconditions=['at_pre_grasp', 'gripper_open'],
            effects=['holding_object']
        ))

        # Action 4: Move to target pre-place position
        pre_place_pos = target_location + np.array([0, 0, 0.1])
        task.add_action(Action(
            action_type=ActionType.MOVE_TO,
            parameters={'position': pre_place_pos, 'speed': 'medium'},
            duration=2.0,
            preconditions=['holding_object'],
            effects=['at_pre_place']
        ))

        # Action 5: Place object
        task.add_action(Action(
            action_type=ActionType.PLACE,
            parameters={
                'position': target_location,
                'descent_speed': 'slow'
            },
            duration=1.5,
            preconditions=['at_pre_place', 'holding_object'],
            effects=['object_placed']
        ))

        # Action 6: Release object
        task.add_action(Action(
            action_type=ActionType.RELEASE,
            parameters={},
            duration=0.5,
            preconditions=['object_placed'],
            effects=['gripper_open', 'task_complete']
        ))

        # Action 7: Move to home position
        task.add_action(Action(
            action_type=ActionType.MOVE_TO,
            parameters={'position': np.array([0, 0, 0.3]), 'speed': 'medium'},
            duration=2.0
        ))

        task.status = TaskStatus.READY
        logger.info(f"✓ Task planned with {len(task.actions)} actions")

        return task

    def plan_observation_task(self, target_classes: List[str]) -> Task:
        """
        Plan task to observe and detect specific objects

        Args:
            target_classes: List of object classes to look for

        Returns:
            Observation task
        """
        task = Task(
            task_id=self.next_task_id,
            description=f"Observe and locate {', '.join(target_classes)}",
            goal="objects_detected"
        )
        self.next_task_id += 1

        # Add observation action
        task.add_action(Action(
            action_type=ActionType.OBSERVE,
            parameters={'target_classes': target_classes, 'duration': 5.0},
            duration=5.0
        ))

        task.status = TaskStatus.READY
        return task

    def plan_exploration_task(self, waypoints: List[np.ndarray]) -> Task:
        """
        Plan task to explore environment from multiple viewpoints

        Args:
            waypoints: List of positions to visit

        Returns:
            Exploration task
        """
        task = Task(
            task_id=self.next_task_id,
            description="Explore environment",
            goal="environment_mapped"
        )
        self.next_task_id += 1

        for i, waypoint in enumerate(waypoints):
            # Move to waypoint
            task.add_action(Action(
                action_type=ActionType.MOVE_TO,
                parameters={'position': waypoint, 'speed': 'medium'},
                duration=2.0
            ))

            # Observe from waypoint
            task.add_action(Action(
                action_type=ActionType.OBSERVE,
                parameters={'duration': 2.0},
                duration=2.0
            ))

        task.status = TaskStatus.READY
        return task

    def execute_task(self, task: Task, action_executor: Optional[Callable] = None) -> bool:
        """
        Execute a planned task

        Args:
            task: Task to execute
            action_executor: Optional callback function to execute actions

        Returns:
            True if task completed successfully
        """
        if task.status != TaskStatus.READY:
            logger.error(f"Task {task.task_id} is not ready for execution")
            return False

        task.status = TaskStatus.EXECUTING
        self.current_task = task

        logger.info(f"Executing task {task.task_id}: {task.description}")

        while not task.is_complete():
            action = task.get_current_action()

            if action is None:
                break

            logger.info(f"  Action: {action.action_type.value} - {action.parameters}")

            # Execute action
            if action_executor:
                success = action_executor(action)
                if not success:
                    logger.error(f"Action failed: {action.action_type.value}")
                    task.status = TaskStatus.FAILED
                    return False

            # Simulate action duration
            # In real system, this would wait for action completion
            task.advance_action()

        if task.is_complete():
            logger.info(f"✓ Task {task.task_id} completed successfully")
            return True
        else:
            logger.error(f"✗ Task {task.task_id} failed")
            return False

    def add_task(self, task: Task):
        """Add task to planner"""
        self.tasks.append(task)

    def get_task(self, task_id: int) -> Optional[Task]:
        """Get task by ID"""
        for task in self.tasks:
            if task.task_id == task_id:
                return task
        return None

    def get_pending_tasks(self) -> List[Task]:
        """Get all pending tasks"""
        return [t for t in self.tasks if t.status == TaskStatus.PENDING]

    def cancel_task(self, task_id: int):
        """Cancel a task"""
        task = self.get_task(task_id)
        if task:
            task.status = TaskStatus.CANCELLED
            logger.info(f"Task {task_id} cancelled")

    def clear_completed_tasks(self):
        """Remove completed tasks from list"""
        self.tasks = [
            t for t in self.tasks
            if t.status not in [TaskStatus.COMPLETED, TaskStatus.CANCELLED]
        ]
