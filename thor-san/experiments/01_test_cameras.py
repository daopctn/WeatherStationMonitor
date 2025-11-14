#!/usr/bin/env python3
"""
Test multi-camera capture system
Verify access to all cameras and display live feeds
"""
import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import cv2
import time
from vision.capture.multi_camera import MultiCameraSystem

def main():
    print("=" * 60)
    print("🔨 Thor-san Vision Test - Multi-Camera Capture")
    print("=" * 60)
    print()

    # Initialize camera system
    print("Initializing cameras...")
    camera_indices = [0, 2, 4]  # Adjust based on your system
    cameras = MultiCameraSystem(camera_indices)

    # Start capture
    cameras.start_capture()
    time.sleep(1.0)  # Give cameras time to warm up

    print("\nCapture started. Press 'q' to quit, 's' to save snapshot")
    print()

    frame_count = 0
    start_time = time.time()

    try:
        while True:
            # Get synchronized frames
            frames = cameras.get_synchronized_frames()

            if not frames:
                print("Waiting for frames...")
                time.sleep(0.1)
                continue

            # Display each camera feed
            for cam_name, frame_data in frames.items():
                if frame_data and 'image' in frame_data:
                    image = frame_data['image']
                    timestamp = frame_data['timestamp']
                    frame_num = frame_data['frame_number']

                    # Add info overlay
                    info_text = f"{cam_name} | Frame: {frame_num} | FPS: {frame_count/(time.time()-start_time):.1f}"
                    cv2.putText(image, info_text, (10, 30),
                               cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

                    # Resize for display
                    display_image = cv2.resize(image, (640, 360))
                    cv2.imshow(cam_name, display_image)

            frame_count += 1

            # Handle keyboard input
            key = cv2.waitKey(1) & 0xFF

            if key == ord('q'):
                print("\nQuitting...")
                break
            elif key == ord('s'):
                # Save snapshot from all cameras
                timestamp = int(time.time())
                for cam_name, frame_data in frames.items():
                    if frame_data:
                        filename = f"snapshot_{cam_name}_{timestamp}.jpg"
                        cv2.imwrite(filename, frame_data['image'])
                        print(f"✓ Saved {filename}")

    except KeyboardInterrupt:
        print("\n\nInterrupted by user")

    finally:
        # Cleanup
        cameras.stop_capture()
        cameras.release()
        cv2.destroyAllWindows()

        # Print statistics
        print("\n" + "=" * 60)
        print("Statistics:")
        stats = cameras.get_stats()
        for cam_name, stat in stats.items():
            print(f"  {cam_name}:")
            print(f"    Frames captured: {stat['frames_captured']}")
            print(f"    Errors: {stat['errors']}")
        print("=" * 60)

if __name__ == "__main__":
    main()
