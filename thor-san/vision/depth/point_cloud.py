"""
Point cloud generation from stereo depth
Converts disparity maps to 3D point clouds with color
"""
import cv2
import numpy as np
from typing import Optional, Tuple
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class PointCloudGenerator:
    """
    Generates 3D point clouds from stereo images and disparity

    Features:
    - Colored point clouds from RGB images
    - Filtering and downsampling
    - Export to various formats
    - Coordinate transformations
    """

    def __init__(self):
        """Initialize point cloud generator"""
        self.points_3d = None
        self.colors = None

    def generate_from_disparity(self, disparity: np.ndarray, Q: np.ndarray,
                                left_image: np.ndarray,
                                min_disparity: float = 1.0) -> Tuple[np.ndarray, np.ndarray]:
        """
        Generate point cloud from disparity map

        Args:
            disparity: Disparity map
            Q: Reprojection matrix (4x4) from stereo calibration
            left_image: Left image for colors (BGR)
            min_disparity: Minimum valid disparity value

        Returns:
            Tuple of (points, colors) where:
                points: Nx3 array of 3D coordinates
                colors: Nx3 array of RGB colors (0-255)
        """
        # Reproject to 3D
        points_3d = cv2.reprojectImageTo3D(disparity, Q)

        # Get colors from left image
        if len(left_image.shape) == 2:
            # Grayscale - replicate to RGB
            colors = cv2.cvtColor(left_image, cv2.COLOR_GRAY2BGR)
        else:
            colors = left_image

        # Create mask for valid disparities
        mask = disparity > min_disparity

        # Filter points and colors
        points = points_3d[mask]
        point_colors = colors[mask]

        # Convert BGR to RGB
        point_colors = cv2.cvtColor(
            point_colors.reshape(-1, 1, 3),
            cv2.COLOR_BGR2RGB
        ).reshape(-1, 3)

        self.points_3d = points
        self.colors = point_colors

        logger.info(f"✓ Generated point cloud with {len(points)} points")

        return points, point_colors

    def filter_depth_range(self, min_depth: float = 0.0, max_depth: float = 10000.0):
        """
        Filter point cloud by depth range

        Args:
            min_depth: Minimum depth in mm
            max_depth: Maximum depth in mm
        """
        if self.points_3d is None:
            return

        z_values = self.points_3d[:, 2]
        mask = (z_values >= min_depth) & (z_values <= max_depth)

        self.points_3d = self.points_3d[mask]
        self.colors = self.colors[mask]

        logger.info(f"✓ Filtered to {len(self.points_3d)} points")

    def downsample(self, factor: int = 2):
        """
        Downsample point cloud

        Args:
            factor: Downsampling factor (keep every Nth point)
        """
        if self.points_3d is None:
            return

        self.points_3d = self.points_3d[::factor]
        self.colors = self.colors[::factor]

        logger.info(f"✓ Downsampled to {len(self.points_3d)} points")

    def remove_outliers(self, nb_neighbors: int = 20, std_ratio: float = 2.0):
        """
        Remove statistical outliers from point cloud

        Args:
            nb_neighbors: Number of neighbors to analyze
            std_ratio: Standard deviation ratio threshold
        """
        if self.points_3d is None or len(self.points_3d) < nb_neighbors:
            return

        from scipy.spatial import KDTree

        # Build KD-tree
        tree = KDTree(self.points_3d)

        # Calculate distances to neighbors
        distances, _ = tree.query(self.points_3d, k=nb_neighbors + 1)
        mean_distances = np.mean(distances[:, 1:], axis=1)

        # Compute threshold
        mean_dist = np.mean(mean_distances)
        std_dist = np.std(mean_distances)
        threshold = mean_dist + std_ratio * std_dist

        # Filter outliers
        mask = mean_distances < threshold
        self.points_3d = self.points_3d[mask]
        self.colors = self.colors[mask]

        logger.info(f"✓ Removed outliers, {len(self.points_3d)} points remaining")

    def save_ply(self, filename: str):
        """
        Save point cloud to PLY file

        Args:
            filename: Output filename
        """
        if self.points_3d is None:
            logger.error("No point cloud to save")
            return

        with open(filename, 'w') as f:
            # Write header
            f.write("ply\n")
            f.write("format ascii 1.0\n")
            f.write(f"element vertex {len(self.points_3d)}\n")
            f.write("property float x\n")
            f.write("property float y\n")
            f.write("property float z\n")
            f.write("property uchar red\n")
            f.write("property uchar green\n")
            f.write("property uchar blue\n")
            f.write("end_header\n")

            # Write vertices
            for point, color in zip(self.points_3d, self.colors):
                f.write(f"{point[0]} {point[1]} {point[2]} ")
                f.write(f"{int(color[0])} {int(color[1])} {int(color[2])}\n")

        logger.info(f"✓ Point cloud saved to {filename}")

    def save_xyz(self, filename: str):
        """
        Save point cloud to XYZ file (no color)

        Args:
            filename: Output filename
        """
        if self.points_3d is None:
            logger.error("No point cloud to save")
            return

        np.savetxt(filename, self.points_3d, fmt='%.6f')
        logger.info(f"✓ Point cloud saved to {filename}")

    def transform(self, rotation: np.ndarray, translation: np.ndarray):
        """
        Apply rigid transformation to point cloud

        Args:
            rotation: 3x3 rotation matrix
            translation: 3x1 translation vector
        """
        if self.points_3d is None:
            return

        # Apply transformation: P' = R * P + T
        self.points_3d = (rotation @ self.points_3d.T).T + translation

    def get_bounds(self) -> Optional[Tuple[np.ndarray, np.ndarray]]:
        """
        Get bounding box of point cloud

        Returns:
            Tuple of (min_point, max_point) as 3D coordinates
        """
        if self.points_3d is None:
            return None

        min_point = np.min(self.points_3d, axis=0)
        max_point = np.max(self.points_3d, axis=0)

        return min_point, max_point

    def get_statistics(self) -> dict:
        """Get statistics about the point cloud"""
        if self.points_3d is None:
            return {}

        return {
            'num_points': len(self.points_3d),
            'mean': np.mean(self.points_3d, axis=0).tolist(),
            'std': np.std(self.points_3d, axis=0).tolist(),
            'min': np.min(self.points_3d, axis=0).tolist(),
            'max': np.max(self.points_3d, axis=0).tolist()
        }
