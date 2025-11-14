#!/usr/bin/env python3
"""
Stereo camera calibration tool
Capture checkerboard images and compute calibration parameters
"""
import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import cv2
import numpy as np
from vision.capture.multi_camera import MultiCameraSystem
from vision.depth.stereo_calibration import StereoCalibrator
import time

def main():
    print("=" * 60)
    print("🔨 Thor-san Stereo Calibration Tool")
    print("=" * 60)
    print()
    print("Instructions:")
    print("  1. Position checkerboard in view of both cameras")
    print("  2. Press SPACE to capture image pair")
    print("  3. Capture 20+ image pairs from different angles/positions")
    print("  4. Press 'c' to compute calibration")
    print("  5. Press 'q' to quit")
    print()

    # Calibration parameters
    CHECKERBOARD_SIZE = (9, 6)  # Inner corners
    SQUARE_SIZE = 25.0  # mm

    # Initialize cameras (using two cameras for stereo)
    print("Initializing stereo cameras...")
    cameras = MultiCameraSystem([0, 2])  # Left and right cameras
    cameras.start_capture()
    time.sleep(1.0)

    # Initialize calibrator
    calibrator = StereoCalibrator(CHECKERBOARD_SIZE, SQUARE_SIZE)

    # Storage for calibration images
    left_images = []
    right_images = []
    capture_count = 0

    print("\n✓ Ready to capture. Press SPACE to capture image pairs")

    try:
        while True:
            frames = cameras.get_synchronized_frames()

            if len(frames) < 2:
                time.sleep(0.1)
                continue

            # Get left and right images
            cam_names = list(frames.keys())
            left_frame = frames[cam_names[0]]['image']
            right_frame = frames[cam_names[1]]['image']

            # Display
            combined = np.hstack([
                cv2.resize(left_frame, (640, 360)),
                cv2.resize(right_frame, (640, 360))
            ])

            # Add text overlay
            info = f"Captured: {capture_count} pairs | Press SPACE to capture, 'c' to calibrate, 'q' to quit"
            cv2.putText(combined, info, (10, 30),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

            cv2.imshow('Stereo Calibration', combined)

            key = cv2.waitKey(1) & 0xFF

            if key == ord(' '):
                # Capture image pair
                print(f"Capturing pair {capture_count + 1}...")

                # Check for checkerboard in both images
                gray_left = cv2.cvtColor(left_frame, cv2.COLOR_BGR2GRAY)
                gray_right = cv2.cvtColor(right_frame, cv2.COLOR_BGR2GRAY)

                ret_left, _ = cv2.findChessboardCorners(gray_left, CHECKERBOARD_SIZE, None)
                ret_right, _ = cv2.findChessboardCorners(gray_right, CHECKERBOARD_SIZE, None)

                if ret_left and ret_right:
                    left_images.append(left_frame.copy())
                    right_images.append(right_frame.copy())
                    capture_count += 1
                    print(f"  ✓ Pair {capture_count} captured")
                else:
                    print("  ✗ Checkerboard not found in both images")

            elif key == ord('c'):
                # Compute calibration
                if capture_count < 10:
                    print(f"\n✗ Need at least 10 image pairs (have {capture_count})")
                    continue

                print(f"\nComputing calibration from {capture_count} image pairs...")
                print("This may take a minute...")

                try:
                    calib_data = calibrator.calibrate_from_images(left_images, right_images)
                    calib_data = calibrator.compute_rectification(calib_data)

                    # Save calibration
                    output_path = '../data/calibration/stereo_calib.yaml'
                    os.makedirs(os.path.dirname(output_path), exist_ok=True)
                    calib_data.save(output_path)

                    print(f"\n✓ Calibration complete!")
                    print(f"  Reprojection error: {calib_data.reprojection_error:.4f}")
                    print(f"  Saved to: {output_path}")

                except Exception as e:
                    print(f"\n✗ Calibration failed: {e}")

            elif key == ord('q'):
                print("\nQuitting...")
                break

    except KeyboardInterrupt:
        print("\n\nInterrupted by user")

    finally:
        cameras.release()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
