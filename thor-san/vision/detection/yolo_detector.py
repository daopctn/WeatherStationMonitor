"""
YOLO v8 object detection for Thor-san
Real-time object detection with bounding boxes and confidence scores
"""
from ultralytics import YOLO
import numpy as np
import cv2
from typing import List, Dict, Optional, Tuple
from dataclasses import dataclass
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class Detection:
    """Single object detection result"""
    class_name: str
    class_id: int
    confidence: float
    bbox: Tuple[float, float, float, float]  # x1, y1, x2, y2
    center: Tuple[float, float]
    area: float
    mask: Optional[np.ndarray] = None

    def to_dict(self) -> Dict:
        """Convert detection to dictionary"""
        return {
            'class': self.class_name,
            'class_id': self.class_id,
            'confidence': self.confidence,
            'bbox': self.bbox,
            'center': self.center,
            'area': self.area
        }


class YOLODetector:
    """
    YOLO v8 object detector

    Features:
    - Multiple YOLO model support (nano, small, medium, large)
    - Real-time detection
    - Configurable confidence thresholds
    - Optional segmentation masks
    """

    # Available YOLO models
    MODELS = {
        'nano': 'yolov8n.pt',
        'small': 'yolov8s.pt',
        'medium': 'yolov8m.pt',
        'large': 'yolov8l.pt',
        'xlarge': 'yolov8x.pt',
    }

    def __init__(self, model_size: str = 'medium', device: str = 'cpu'):
        """
        Initialize YOLO detector

        Args:
            model_size: Model size ('nano', 'small', 'medium', 'large', 'xlarge')
            device: Device to run on ('cpu', 'cuda', 'mps')
        """
        self.model_size = model_size
        self.device = device

        # Load model
        model_path = self.MODELS.get(model_size, 'yolov8m.pt')
        logger.info(f"Loading YOLO model: {model_path} on {device}")

        try:
            self.model = YOLO(model_path)
            self.model.to(device)
            self.class_names = self.model.names
            logger.info(f"✓ YOLO model loaded: {len(self.class_names)} classes")
        except Exception as e:
            logger.error(f"Failed to load YOLO model: {e}")
            raise

    def detect(self, image: np.ndarray, conf_threshold: float = 0.5,
               iou_threshold: float = 0.45, classes: Optional[List[int]] = None) -> List[Detection]:
        """
        Run detection on single image

        Args:
            image: Input image (BGR format)
            conf_threshold: Minimum confidence threshold
            iou_threshold: IoU threshold for NMS
            classes: Optional list of class IDs to detect

        Returns:
            List of Detection objects
        """
        try:
            # Run inference
            results = self.model(
                image,
                conf=conf_threshold,
                iou=iou_threshold,
                classes=classes,
                verbose=False
            )

            detections = []
            for r in results:
                boxes = r.boxes
                for i in range(len(boxes)):
                    box = boxes[i]
                    x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                    cls_id = int(box.cls[0])
                    conf = float(box.conf[0])

                    detection = Detection(
                        class_name=self.class_names[cls_id],
                        class_id=cls_id,
                        confidence=conf,
                        bbox=(float(x1), float(y1), float(x2), float(y2)),
                        center=((x1 + x2) / 2, (y1 + y2) / 2),
                        area=(x2 - x1) * (y2 - y1)
                    )
                    detections.append(detection)

            return detections

        except Exception as e:
            logger.error(f"Detection error: {e}")
            return []

    def detect_batch(self, images: List[np.ndarray], conf_threshold: float = 0.5) -> List[List[Detection]]:
        """
        Run detection on batch of images

        Args:
            images: List of input images
            conf_threshold: Minimum confidence threshold

        Returns:
            List of detection lists (one per image)
        """
        try:
            results = self.model(images, conf=conf_threshold, verbose=False)

            batch_detections = []
            for r in results:
                detections = []
                boxes = r.boxes
                for i in range(len(boxes)):
                    box = boxes[i]
                    x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                    cls_id = int(box.cls[0])
                    conf = float(box.conf[0])

                    detection = Detection(
                        class_name=self.class_names[cls_id],
                        class_id=cls_id,
                        confidence=conf,
                        bbox=(float(x1), float(y1), float(x2), float(y2)),
                        center=((x1 + x2) / 2, (y1 + y2) / 2),
                        area=(x2 - x1) * (y2 - y1)
                    )
                    detections.append(detection)

                batch_detections.append(detections)

            return batch_detections

        except Exception as e:
            logger.error(f"Batch detection error: {e}")
            return [[] for _ in images]

    def draw_detections(self, image: np.ndarray, detections: List[Detection],
                       show_confidence: bool = True, thickness: int = 2) -> np.ndarray:
        """
        Draw detections on image

        Args:
            image: Input image
            detections: List of detections to draw
            show_confidence: Whether to show confidence scores
            thickness: Line thickness for bounding boxes

        Returns:
            Image with drawn detections
        """
        output = image.copy()

        for det in detections:
            x1, y1, x2, y2 = det.bbox
            x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)

            # Draw bounding box
            color = self._get_class_color(det.class_id)
            cv2.rectangle(output, (x1, y1), (x2, y2), color, thickness)

            # Draw label
            if show_confidence:
                label = f"{det.class_name}: {det.confidence:.2f}"
            else:
                label = det.class_name

            # Text background
            (tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.6, 1)
            cv2.rectangle(output, (x1, y1 - th - 8), (x1 + tw, y1), color, -1)

            # Text
            cv2.putText(output, label, (x1, y1 - 5),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)

        return output

    def _get_class_color(self, class_id: int) -> Tuple[int, int, int]:
        """Get consistent color for class ID"""
        np.random.seed(class_id)
        color = tuple(np.random.randint(0, 255, 3).tolist())
        return color

    def filter_by_class(self, detections: List[Detection], class_names: List[str]) -> List[Detection]:
        """Filter detections by class name"""
        return [d for d in detections if d.class_name in class_names]

    def filter_by_confidence(self, detections: List[Detection], min_conf: float) -> List[Detection]:
        """Filter detections by minimum confidence"""
        return [d for d in detections if d.confidence >= min_conf]

    def filter_by_area(self, detections: List[Detection],
                      min_area: Optional[float] = None,
                      max_area: Optional[float] = None) -> List[Detection]:
        """Filter detections by bounding box area"""
        filtered = detections
        if min_area is not None:
            filtered = [d for d in filtered if d.area >= min_area]
        if max_area is not None:
            filtered = [d for d in filtered if d.area <= max_area]
        return filtered

    def get_largest_detection(self, detections: List[Detection]) -> Optional[Detection]:
        """Get detection with largest area"""
        if not detections:
            return None
        return max(detections, key=lambda d: d.area)

    def get_most_confident(self, detections: List[Detection]) -> Optional[Detection]:
        """Get detection with highest confidence"""
        if not detections:
            return None
        return max(detections, key=lambda d: d.confidence)
