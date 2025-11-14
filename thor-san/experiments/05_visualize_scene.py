#!/usr/bin/env python3
"""
3D scene visualization and analysis
Load spatial memory, detect objects, and analyze scene
"""
import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import numpy as np
import cv2
import time
from spatial_memory.octree_map import SpatialMemory
from spatial_memory.object_database import ObjectDatabase, ObjectRecord
from spatial_memory.scene_graph import SceneGraph, RelationType
from intelligence.scene_analyzer import SceneAnalyzer

def visualize_octree_2d(spatial_memory, resolution=100):
    """
    Create 2D top-down visualization of octree

    Args:
        spatial_memory: SpatialMemory instance
        resolution: Image resolution
    """
    # Get all occupied voxels
    voxels = spatial_memory.get_occupied_voxels()

    if not voxels:
        print("No voxels to visualize")
        return np.zeros((resolution, resolution, 3), dtype=np.uint8)

    # Find bounds
    positions = np.array([v[0] for v in voxels])
    min_pos = positions.min(axis=0)
    max_pos = positions.max(axis=0)
    extent = max_pos - min_pos

    # Create image
    image = np.zeros((resolution, resolution, 3), dtype=np.uint8)

    # Draw voxels
    for center, size, color in voxels:
        # Project to 2D (X-Y plane)
        x = int((center[0] - min_pos[0]) / extent[0] * (resolution - 1))
        y = int((center[1] - min_pos[1]) / extent[1] * (resolution - 1))

        # Use height for color
        z_normalized = (center[2] - min_pos[2]) / extent[2] if extent[2] > 0 else 0.5
        color_val = int(z_normalized * 255)

        # Use color from voxel if available
        if color is not None:
            bgr_color = tuple(int(c) for c in color[:3])
        else:
            bgr_color = (color_val, color_val, color_val)

        cv2.circle(image, (x, resolution - 1 - y), 2, bgr_color, -1)

    return image

def main():
    print("=" * 60)
    print("🔨 Thor-san 3D Scene Visualization & Analysis")
    print("=" * 60)
    print()

    # Load spatial memory
    map_path = '../data/maps/spatial_map_final.pkl'
    if not os.path.exists(map_path):
        print(f"✗ Spatial map not found: {map_path}")
        print("  Run 04_build_3d_map.py first!")
        return

    print("Loading spatial memory...")
    spatial_memory = SpatialMemory()
    spatial_memory.load(map_path)
    print("✓ Spatial memory loaded")

    # Initialize components
    print("Initializing analysis components...")
    scene_analyzer = SceneAnalyzer()
    scene_graph = SceneGraph()
    object_db = ObjectDatabase('../data/objects.db')

    # Analyze scene
    print("\nAnalyzing scene...")
    voxels = spatial_memory.get_occupied_voxels()

    if voxels:
        points = np.array([v[0] for v in voxels])
        colors = np.array([v[2] if v[2] is not None else [128, 128, 128] for v in voxels])

        analysis = scene_analyzer.analyze_point_cloud(points, colors)

        print("\nScene Analysis Results:")
        print("=" * 60)
        print(f"Ground plane Z: {analysis['ground_plane']:.3f}m")
        print(f"Number of surfaces: {len(analysis['surfaces'])}")
        print(f"Point density: {analysis['point_density']:.1f} points/m³")

        if analysis['surfaces']:
            print("\nDetected Surfaces:")
            for i, surface in enumerate(analysis['surfaces']):
                print(f"  Surface {i+1}:")
                print(f"    Height: {surface['height_above_ground']*100:.1f}cm above ground")
                print(f"    Area: {surface['area']:.3f}m²")
                print(f"    Points: {surface['point_count']}")

    # Visualize
    print("\nGenerating visualization...")
    vis_image = visualize_octree_2d(spatial_memory, resolution=800)

    # Add info overlay
    stats = spatial_memory.get_statistics()
    y_offset = 30
    for key, value in stats.items():
        text = f"{key}: {value}"
        cv2.putText(vis_image, text, (10, y_offset),
                   cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
        y_offset += 25

    print("\n✓ Visualization ready")
    print("\nShowing 2D top-down view of 3D map")
    print("Press 'q' to quit")

    # Display
    cv2.imshow('Thor-san 3D Map - Top View', vis_image)

    while True:
        key = cv2.waitKey(100) & 0xFF
        if key == ord('q'):
            break

    cv2.destroyAllWindows()
    object_db.close()

    print("\n" + "=" * 60)
    print("Analysis complete!")
    print("=" * 60)

if __name__ == "__main__":
    main()
