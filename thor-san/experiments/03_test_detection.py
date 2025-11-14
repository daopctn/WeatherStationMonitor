#!/usr/bin/env python3
"""
Test YOLO detection with multi-camera system
Real-time object detection from multiple cameras
"""
import sys
import os
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import cv2
import numpy as np
import time
from vision.capture.multi_camera import MultiCameraSystem
from vision.detection.yolo_detector import YOLODetector
from vision.detection.object_tracker import ObjectTracker

def main():
    print("=" * 60)
    print("🔨 Thor-san Vision Test - Object Detection")
    print("=" * 60)
    print()

    # Initialize cameras
    print("Initializing cameras...")
    cameras = MultiCameraSystem([0, 2, 4])
    cameras.start_capture()
    time.sleep(1.0)

    # Initialize detector
    print("Loading YOLO model...")
    detector = YOLODetector(model_size='medium', device='cpu')
    print("✓ YOLO model loaded")

    # Initialize tracker
    tracker = ObjectTracker(max_disappeared=30, max_distance=50.0)

    print("\nStarting detection... Press 'q' to quit")
    print()

    detection_times = []

    try:
        while True:
            frames = cameras.get_synchronized_frames()

            if not frames:
                time.sleep(0.1)
                continue

            # Process each camera feed
            for cam_name, frame_data in frames.items():
                if not frame_data:
                    continue

                image = frame_data['image']

                # Run detection
                start_time = time.time()
                detections = detector.detect(image, conf_threshold=0.5)
                detection_time = time.time() - start_time
                detection_times.append(detection_time)

                # Update tracker
                tracked_objects = tracker.update(detections)

                # Draw detections
                output = detector.draw_detections(image, detections)

                # Draw tracking IDs
                for obj_id, tracked_obj in tracked_objects.items():
                    x1, y1, x2, y2 = tracked_obj.bbox
                    cx, cy = tracked_obj.center

                    # Draw tracking ID
                    cv2.circle(output, (int(cx), int(cy)), 5, (0, 0, 255), -1)
                    cv2.putText(output, f"ID:{obj_id}", (int(cx) + 10, int(cy)),
                               cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)

                # Add performance info
                avg_time = np.mean(detection_times[-30:]) if detection_times else 0
                fps = 1.0 / avg_time if avg_time > 0 else 0
                info = f"{cam_name} | Detections: {len(detections)} | Tracked: {len(tracked_objects)} | FPS: {fps:.1f}"
                cv2.putText(output, info, (10, 30),
                           cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)

                # Display
                display_image = cv2.resize(output, (640, 360))
                cv2.imshow(cam_name, display_image)

            # Handle keyboard
            key = cv2.waitKey(1) & 0xFF
            if key == ord('q'):
                print("\nQuitting...")
                break

    except KeyboardInterrupt:
        print("\n\nInterrupted by user")

    finally:
        cameras.release()
        cv2.destroyAllWindows()

        # Print statistics
        print("\n" + "=" * 60)
        print("Statistics:")
        if detection_times:
            print(f"  Average detection time: {np.mean(detection_times)*1000:.1f}ms")
            print(f"  Average FPS: {1.0/np.mean(detection_times):.1f}")
        print("=" * 60)

if __name__ == "__main__":
    main()
