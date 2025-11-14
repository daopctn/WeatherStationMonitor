#!/usr/bin/env python3
"""
Human-like Binocular Vision Demo
Demonstrates depth perception using two monocular cameras like human eyes
"""
import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import cv2
import numpy as np
import time
from vision.capture.multi_camera import MultiCameraSystem
from vision.binocular.binocular_vision import BinocularVisionSystem, BinocularConfig
from spatial_memory.octree_map import SpatialMemory

def main():
    print("=" * 70)
    print("🔨 Thor-san Human-like Binocular Vision System")
    print("=" * 70)
    print()
    print("This system mimics human binocular vision:")
    print("  ✓ Two monocular cameras (like left/right eyes)")
    print("  ✓ Feature-based correspondence matching (like binocular neurons)")
    print("  ✓ Temporal depth integration (like visual memory)")
    print("  ✓ Attention-weighted processing (like human attention)")
    print()

    # Initialize cameras (two cameras like human eyes)
    print("Initializing binocular camera system...")
    cameras = MultiCameraSystem([0, 2])  # Left and right "eyes"
    cameras.start_capture()
    time.sleep(1.0)

    # Configure binocular vision
    config = BinocularConfig(
        baseline=0.065,  # ~6.5cm like human eyes
        focal_length=700.0,
        feature_type="orb",
        max_features=500,
        temporal_window=10,
        use_attention=True
    )

    # Initialize binocular vision system
    print("Initializing binocular vision system...")
    binocular = BinocularVisionSystem(config)
    print("✓ System ready")

    # Initialize spatial memory for 3D mapping
    spatial_memory = SpatialMemory(resolution=0.01, world_size=2.0)

    print("\nControls:")
    print("  SPACE - Add current view to 3D map")
    print("  'r'   - Reset temporal integration")
    print("  's'   - Save spatial map")
    print("  'q'   - Quit")
    print()

    frame_count = 0
    processing_times = []
    capture_count = 0

    try:
        while True:
            frames = cameras.get_synchronized_frames()

            if len(frames) < 2:
                time.sleep(0.1)
                continue

            # Get left and right eye images
            cam_names = list(frames.keys())
            left_image = frames[cam_names[0]]['image']
            right_image = frames[cam_names[1]]['image']

            # Process with binocular vision system
            start_time = time.time()
            result = binocular.process_frame_pair(left_image, right_image)
            process_time = time.time() - start_time
            processing_times.append(process_time)

            # Get visualizations
            depth_vis = binocular.visualize_depth()

            # Create combined display
            left_display = cv2.resize(left_image, (640, 360))
            right_display = cv2.resize(right_image, (640, 360))
            depth_display = cv2.resize(depth_vis, (640, 360))

            # Add info overlays
            stats = binocular.get_statistics()
            fps = 1.0 / np.mean(processing_times[-30:]) if processing_times else 0

            info_text = [
                f"FPS: {fps:.1f} | Matches: {result['num_matches']}",
                f"Mean Depth: {stats['mean_depth']:.2f}m",
                f"Confidence: {stats['confidence_mean']:.2f}",
                f"Captures: {capture_count}"
            ]

            y_offset = 30
            for text in info_text:
                cv2.putText(left_display, text, (10, y_offset),
                           cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
                y_offset += 25

            # Combine views
            top_row = np.hstack([left_display, right_display])
            bottom_row = np.hstack([depth_display, np.zeros_like(depth_display)])
            combined = np.vstack([top_row, bottom_row])

            # Add labels
            cv2.putText(combined, "Left Eye", (10, 20),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
            cv2.putText(combined, "Right Eye", (650, 20),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
            cv2.putText(combined, "Depth Map (Binocular)", (10, 380),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

            cv2.imshow('Thor-san Binocular Vision', combined)

            frame_count += 1

            # Handle keyboard input
            key = cv2.waitKey(1) & 0xFF

            if key == ord(' '):
                # Add to 3D spatial map
                print(f"\nCapturing 3D points (frame {frame_count})...")

                points_3d = binocular.get_3d_points(depth_threshold=0.1)

                if len(points_3d) > 0:
                    # Downsample for efficiency
                    if len(points_3d) > 10000:
                        indices = np.random.choice(len(points_3d), 10000, replace=False)
                        points_3d = points_3d[indices]

                    spatial_memory.insert_point_cloud(points_3d)
                    capture_count += 1

                    stats = spatial_memory.get_statistics()
                    print(f"  ✓ Added {len(points_3d)} points")
                    print(f"  ✓ Total occupied voxels: {stats['occupied_voxels']}")
                else:
                    print("  ✗ No valid 3D points")

            elif key == ord('r'):
                # Reset temporal integration
                binocular.temporal_fusion.reset()
                print("\n✓ Temporal integration reset")

            elif key == ord('s'):
                # Save spatial map
                if capture_count > 0:
                    output_path = f'../data/maps/binocular_map_{int(time.time())}.pkl'
                    os.makedirs(os.path.dirname(output_path), exist_ok=True)
                    spatial_memory.save(output_path)
                    print(f"\n✓ Spatial map saved: {output_path}")
                else:
                    print("\n✗ No data captured yet")

            elif key == ord('q'):
                print("\nQuitting...")
                break

    except KeyboardInterrupt:
        print("\n\nInterrupted by user")

    finally:
        cameras.release()
        cv2.destroyAllWindows()

        # Print final statistics
        print("\n" + "=" * 70)
        print("Session Statistics:")
        print(f"  Frames processed: {frame_count}")
        print(f"  3D captures: {capture_count}")

        if processing_times:
            print(f"  Average processing time: {np.mean(processing_times)*1000:.1f}ms")
            print(f"  Average FPS: {1.0/np.mean(processing_times):.1f}")

        spatial_stats = spatial_memory.get_statistics()
        print(f"\n3D Spatial Memory:")
        for key, value in spatial_stats.items():
            print(f"  {key}: {value}")

        print("=" * 70)
        print("\n✓ Binocular vision demo complete!")
        print("\nThis human-like system:")
        print("  - Processes each eye independently (like human vision)")
        print("  - Finds correspondences using features (like binocular neurons)")
        print("  - Integrates depth over time (like visual memory)")
        print("  - Uses attention to focus on important regions")
        print()

if __name__ == "__main__":
    main()
