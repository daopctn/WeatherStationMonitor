#!/usr/bin/env python3
"""
Build 3D map from stereo vision
Capture depth and build octree spatial memory
"""
import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import cv2
import numpy as np
import time
from vision.capture.multi_camera import MultiCameraSystem
from vision.depth.stereo_calibration import CalibrationData
from vision.depth.disparity_map import DisparityEstimator
from vision.depth.point_cloud import PointCloudGenerator
from spatial_memory.octree_map import SpatialMemory

def main():
    print("=" * 60)
    print("🔨 Thor-san 3D Mapping - Build Spatial Memory")
    print("=" * 60)
    print()

    # Load calibration
    calib_path = '../data/calibration/stereo_calib.yaml'
    if not os.path.exists(calib_path):
        print(f"✗ Calibration file not found: {calib_path}")
        print("  Run 02_calibrate_stereo.py first!")
        return

    print("Loading calibration...")
    calib = CalibrationData.load(calib_path)
    print("✓ Calibration loaded")

    # Initialize cameras
    print("Initializing stereo cameras...")
    cameras = MultiCameraSystem([0, 2])
    cameras.start_capture()
    time.sleep(1.0)

    # Initialize disparity estimator
    print("Initializing disparity estimator...")
    disparity_estimator = DisparityEstimator(method='sgbm', num_disparities=128, block_size=11)
    disparity_estimator.enable_wls_filter()

    # Initialize point cloud generator
    pc_generator = PointCloudGenerator()

    # Initialize spatial memory
    print("Initializing spatial memory...")
    spatial_memory = SpatialMemory(resolution=0.01, world_size=2.0)

    print("\n✓ System ready")
    print("\nControls:")
    print("  SPACE - Capture and add to 3D map")
    print("  's'   - Save map to file")
    print("  'c'   - Clear map")
    print("  'q'   - Quit")
    print()

    capture_count = 0

    try:
        while True:
            frames = cameras.get_synchronized_frames()

            if len(frames) < 2:
                time.sleep(0.1)
                continue

            # Get stereo pair
            cam_names = list(frames.keys())
            left_image = frames[cam_names[0]]['image']
            right_image = frames[cam_names[1]]['image']

            # Display stereo pair
            display = np.hstack([
                cv2.resize(left_image, (640, 360)),
                cv2.resize(right_image, (640, 360))
            ])

            info = f"Captures: {capture_count} | Map size: {spatial_memory.get_statistics()['occupied_voxels']} voxels"
            cv2.putText(display, info, (10, 30),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            cv2.putText(display, "Press SPACE to capture", (10, 340),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

            cv2.imshow('Stereo View', display)

            key = cv2.waitKey(1) & 0xFF

            if key == ord(' '):
                print(f"\nCapturing frame {capture_count + 1}...")

                # Compute disparity
                print("  Computing disparity...")
                start_time = time.time()
                disparity = disparity_estimator.compute(left_image, right_image)
                print(f"  ✓ Disparity computed ({time.time()-start_time:.2f}s)")

                # Generate point cloud
                print("  Generating point cloud...")
                points, colors = pc_generator.generate_from_disparity(
                    disparity, calib.Q, left_image, min_disparity=1.0
                )
                print(f"  ✓ Generated {len(points)} points")

                # Filter and downsample
                pc_generator.filter_depth_range(min_depth=100, max_depth=2000)
                pc_generator.downsample(factor=5)
                print(f"  ✓ Filtered to {len(pc_generator.points_3d)} points")

                # Convert from mm to meters
                points_m = pc_generator.points_3d / 1000.0

                # Add to spatial memory
                print("  Inserting into spatial memory...")
                spatial_memory.insert_point_cloud(points_m, pc_generator.colors)
                capture_count += 1

                stats = spatial_memory.get_statistics()
                print(f"  ✓ Map now contains {stats['occupied_voxels']} occupied voxels")

                # Show disparity visualization
                disp_vis = disparity_estimator.visualize_disparity(disparity)
                cv2.imshow('Disparity Map', cv2.resize(disp_vis, (640, 360)))

            elif key == ord('s'):
                # Save map
                output_path = f'../data/maps/spatial_map_{int(time.time())}.pkl'
                os.makedirs(os.path.dirname(output_path), exist_ok=True)
                spatial_memory.save(output_path)
                print(f"\n✓ Spatial memory saved to {output_path}")

                # Also save latest point cloud
                pc_path = output_path.replace('.pkl', '.ply')
                pc_generator.save_ply(pc_path)
                print(f"✓ Point cloud saved to {pc_path}")

            elif key == ord('c'):
                # Clear map
                spatial_memory = SpatialMemory(resolution=0.01, world_size=2.0)
                capture_count = 0
                print("\n✓ Spatial memory cleared")

            elif key == ord('q'):
                print("\nQuitting...")
                break

    except KeyboardInterrupt:
        print("\n\nInterrupted by user")

    finally:
        # Save final map
        if capture_count > 0:
            output_path = '../data/maps/spatial_map_final.pkl'
            os.makedirs(os.path.dirname(output_path), exist_ok=True)
            spatial_memory.save(output_path)
            print(f"\n✓ Final spatial memory saved to {output_path}")

        cameras.release()
        cv2.destroyAllWindows()

        # Print final statistics
        print("\n" + "=" * 60)
        print("Final Statistics:")
        stats = spatial_memory.get_statistics()
        for key, value in stats.items():
            print(f"  {key}: {value}")
        print("=" * 60)

if __name__ == "__main__":
    main()
