"""
Object pose database with persistent SQLite storage
Tracks detected objects and their 3D poses over time
"""
import sqlite3
import numpy as np
import json
from typing import List, Optional, Dict, Tuple
from dataclasses import dataclass, asdict
from datetime import datetime
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class ObjectRecord:
    """Record of a detected object and its 3D pose"""
    object_id: Optional[int] = None
    class_name: str = ""
    confidence: float = 0.0
    position: Tuple[float, float, float] = (0.0, 0.0, 0.0)  # XYZ in meters
    orientation: Tuple[float, float, float, float] = (0.0, 0.0, 0.0, 1.0)  # Quaternion
    dimensions: Tuple[float, float, float] = (0.0, 0.0, 0.0)  # Width, height, depth
    timestamp: Optional[str] = None
    camera_source: str = ""
    tracking_id: int = -1
    metadata: Dict = None

    def __post_init__(self):
        if self.timestamp is None:
            self.timestamp = datetime.now().isoformat()
        if self.metadata is None:
            self.metadata = {}

    def to_dict(self) -> Dict:
        """Convert to dictionary for JSON serialization"""
        data = asdict(self)
        data['metadata'] = json.dumps(self.metadata)
        return data

    @classmethod
    def from_dict(cls, data: Dict) -> 'ObjectRecord':
        """Create from dictionary"""
        if 'metadata' in data and isinstance(data['metadata'], str):
            data['metadata'] = json.loads(data['metadata'])
        return cls(**data)


class ObjectDatabase:
    """
    SQLite database for persistent object storage

    Features:
    - Store detected objects with 3D poses
    - Query by location, class, time
    - Update object poses
    - Track object history
    """

    def __init__(self, db_path: str = "data/objects.db"):
        """
        Initialize object database

        Args:
            db_path: Path to SQLite database file
        """
        self.db_path = db_path
        self.conn = None
        self._create_database()

    def _create_database(self):
        """Create database and tables if they don't exist"""
        self.conn = sqlite3.connect(self.db_path, check_same_thread=False)
        self.conn.row_factory = sqlite3.Row

        cursor = self.conn.cursor()

        # Objects table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS objects (
                object_id INTEGER PRIMARY KEY AUTOINCREMENT,
                class_name TEXT NOT NULL,
                confidence REAL,
                pos_x REAL,
                pos_y REAL,
                pos_z REAL,
                orient_x REAL,
                orient_y REAL,
                orient_z REAL,
                orient_w REAL,
                dim_width REAL,
                dim_height REAL,
                dim_depth REAL,
                timestamp TEXT,
                camera_source TEXT,
                tracking_id INTEGER,
                metadata TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')

        # Object history table for tracking changes
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS object_history (
                history_id INTEGER PRIMARY KEY AUTOINCREMENT,
                object_id INTEGER,
                pos_x REAL,
                pos_y REAL,
                pos_z REAL,
                confidence REAL,
                timestamp TEXT,
                FOREIGN KEY (object_id) REFERENCES objects (object_id)
            )
        ''')

        # Indices for fast queries
        cursor.execute('CREATE INDEX IF NOT EXISTS idx_class ON objects (class_name)')
        cursor.execute('CREATE INDEX IF NOT EXISTS idx_timestamp ON objects (timestamp)')
        cursor.execute('CREATE INDEX IF NOT EXISTS idx_tracking ON objects (tracking_id)')

        self.conn.commit()
        logger.info(f"✓ Object database initialized: {self.db_path}")

    def insert_object(self, record: ObjectRecord) -> int:
        """
        Insert new object record

        Args:
            record: ObjectRecord to insert

        Returns:
            ID of inserted object
        """
        cursor = self.conn.cursor()

        cursor.execute('''
            INSERT INTO objects (
                class_name, confidence,
                pos_x, pos_y, pos_z,
                orient_x, orient_y, orient_z, orient_w,
                dim_width, dim_height, dim_depth,
                timestamp, camera_source, tracking_id, metadata
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            record.class_name, record.confidence,
            record.position[0], record.position[1], record.position[2],
            record.orientation[0], record.orientation[1], record.orientation[2], record.orientation[3],
            record.dimensions[0], record.dimensions[1], record.dimensions[2],
            record.timestamp, record.camera_source, record.tracking_id,
            json.dumps(record.metadata)
        ))

        self.conn.commit()
        object_id = cursor.lastrowid

        # Add to history
        self._add_to_history(object_id, record)

        return object_id

    def update_object(self, object_id: int, record: ObjectRecord):
        """
        Update existing object record

        Args:
            object_id: ID of object to update
            record: New object data
        """
        cursor = self.conn.cursor()

        cursor.execute('''
            UPDATE objects SET
                class_name = ?,
                confidence = ?,
                pos_x = ?, pos_y = ?, pos_z = ?,
                orient_x = ?, orient_y = ?, orient_z = ?, orient_w = ?,
                dim_width = ?, dim_height = ?, dim_depth = ?,
                timestamp = ?,
                camera_source = ?,
                tracking_id = ?,
                metadata = ?
            WHERE object_id = ?
        ''', (
            record.class_name, record.confidence,
            record.position[0], record.position[1], record.position[2],
            record.orientation[0], record.orientation[1], record.orientation[2], record.orientation[3],
            record.dimensions[0], record.dimensions[1], record.dimensions[2],
            record.timestamp, record.camera_source, record.tracking_id,
            json.dumps(record.metadata),
            object_id
        ))

        self.conn.commit()

        # Add to history
        self._add_to_history(object_id, record)

    def _add_to_history(self, object_id: int, record: ObjectRecord):
        """Add object state to history"""
        cursor = self.conn.cursor()
        cursor.execute('''
            INSERT INTO object_history (object_id, pos_x, pos_y, pos_z, confidence, timestamp)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (object_id, record.position[0], record.position[1], record.position[2],
              record.confidence, record.timestamp))
        self.conn.commit()

    def get_object(self, object_id: int) -> Optional[ObjectRecord]:
        """
        Get object by ID

        Args:
            object_id: Object ID

        Returns:
            ObjectRecord or None
        """
        cursor = self.conn.cursor()
        cursor.execute('SELECT * FROM objects WHERE object_id = ?', (object_id,))
        row = cursor.fetchone()

        if row:
            return self._row_to_record(row)
        return None

    def query_by_class(self, class_name: str) -> List[ObjectRecord]:
        """
        Query objects by class name

        Args:
            class_name: Class name to search for

        Returns:
            List of matching ObjectRecords
        """
        cursor = self.conn.cursor()
        cursor.execute('SELECT * FROM objects WHERE class_name = ? ORDER BY timestamp DESC',
                      (class_name,))

        return [self._row_to_record(row) for row in cursor.fetchall()]

    def query_by_location(self, center: Tuple[float, float, float],
                         radius: float) -> List[ObjectRecord]:
        """
        Query objects within radius of center point

        Args:
            center: Center point (x, y, z)
            radius: Search radius in meters

        Returns:
            List of matching ObjectRecords
        """
        cursor = self.conn.cursor()
        cursor.execute('SELECT * FROM objects')

        results = []
        for row in cursor.fetchall():
            pos = (row['pos_x'], row['pos_y'], row['pos_z'])
            distance = np.linalg.norm(np.array(pos) - np.array(center))
            if distance <= radius:
                results.append(self._row_to_record(row))

        return results

    def query_recent(self, limit: int = 10) -> List[ObjectRecord]:
        """
        Query most recent objects

        Args:
            limit: Maximum number of results

        Returns:
            List of ObjectRecords
        """
        cursor = self.conn.cursor()
        cursor.execute('SELECT * FROM objects ORDER BY timestamp DESC LIMIT ?', (limit,))

        return [self._row_to_record(row) for row in cursor.fetchall()]

    def get_object_history(self, object_id: int) -> List[Dict]:
        """
        Get position history for an object

        Args:
            object_id: Object ID

        Returns:
            List of history entries
        """
        cursor = self.conn.cursor()
        cursor.execute('''
            SELECT * FROM object_history
            WHERE object_id = ?
            ORDER BY timestamp ASC
        ''', (object_id,))

        return [dict(row) for row in cursor.fetchall()]

    def delete_object(self, object_id: int):
        """Delete object and its history"""
        cursor = self.conn.cursor()
        cursor.execute('DELETE FROM object_history WHERE object_id = ?', (object_id,))
        cursor.execute('DELETE FROM objects WHERE object_id = ?', (object_id,))
        self.conn.commit()

    def clear_all(self):
        """Clear all objects from database"""
        cursor = self.conn.cursor()
        cursor.execute('DELETE FROM object_history')
        cursor.execute('DELETE FROM objects')
        self.conn.commit()
        logger.info("✓ Database cleared")

    def get_statistics(self) -> Dict:
        """Get database statistics"""
        cursor = self.conn.cursor()

        cursor.execute('SELECT COUNT(*) FROM objects')
        total_objects = cursor.fetchone()[0]

        cursor.execute('SELECT COUNT(DISTINCT class_name) FROM objects')
        unique_classes = cursor.fetchone()[0]

        cursor.execute('SELECT class_name, COUNT(*) as count FROM objects GROUP BY class_name')
        class_counts = {row[0]: row[1] for row in cursor.fetchall()}

        return {
            'total_objects': total_objects,
            'unique_classes': unique_classes,
            'class_counts': class_counts
        }

    def _row_to_record(self, row: sqlite3.Row) -> ObjectRecord:
        """Convert database row to ObjectRecord"""
        return ObjectRecord(
            object_id=row['object_id'],
            class_name=row['class_name'],
            confidence=row['confidence'],
            position=(row['pos_x'], row['pos_y'], row['pos_z']),
            orientation=(row['orient_x'], row['orient_y'], row['orient_z'], row['orient_w']),
            dimensions=(row['dim_width'], row['dim_height'], row['dim_depth']),
            timestamp=row['timestamp'],
            camera_source=row['camera_source'],
            tracking_id=row['tracking_id'],
            metadata=json.loads(row['metadata']) if row['metadata'] else {}
        )

    def close(self):
        """Close database connection"""
        if self.conn:
            self.conn.close()
            logger.info("✓ Database connection closed")

    def __enter__(self):
        """Context manager entry"""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.close()
