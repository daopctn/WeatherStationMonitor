"""
Correspondence matching between left and right eye views
Simulates binocular neurons in visual cortex
"""
import cv2
import numpy as np
from typing import List, Tuple, Optional
from dataclasses import dataclass
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class MatchPoint:
    """A matched point between left and right views"""
    left_pt: Tuple[float, float]
    right_pt: Tuple[float, float]
    disparity: float
    depth: float
    confidence: float


class CorrespondenceMatcher:
    """
    Finds corresponding points between left and right eye images

    Simulates the function of binocular neurons in visual cortex V1/V2
    that respond to matching features between eyes
    """

    def __init__(self, config):
        """
        Initialize correspondence matcher

        Args:
            config: BinocularConfig instance
        """
        self.config = config

        # Matcher (simulates binocular matching neurons)
        if config.feature_type in ["orb"]:
            self.matcher = cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=False)
        else:
            self.matcher = cv2.BFMatcher(cv2.NORM_L2, crossCheck=False)

        logger.info("✓ Correspondence matcher initialized")

    def match_features(self, left_kp: List, left_desc: np.ndarray,
                      right_kp: List, right_desc: np.ndarray) -> List:
        """
        Find correspondences between left and right features

        Args:
            left_kp: Left keypoints
            left_desc: Left descriptors
            right_kp: Right keypoints
            right_desc: Right descriptors

        Returns:
            List of good matches
        """
        if len(left_desc) == 0 or len(right_desc) == 0:
            return []

        # Find matches using KNN (k=2 for ratio test)
        matches = self.matcher.knnMatch(left_desc, right_desc, k=2)

        # Apply Lowe's ratio test (simulates match disambiguation)
        good_matches = []
        for match_pair in matches:
            if len(match_pair) < 2:
                continue

            m, n = match_pair
            if m.distance < self.config.match_ratio * n.distance:
                # Additional epipolar constraint for stereo
                if self._check_epipolar_constraint(left_kp[m.queryIdx], right_kp[m.trainIdx]):
                    good_matches.append(m)

        return good_matches

    def _check_epipolar_constraint(self, left_kp, right_kp, threshold: float = 2.0) -> bool:
        """
        Check if match satisfies epipolar constraint
        (For rectified stereo, points should be on same horizontal line)

        Args:
            left_kp: Left keypoint
            right_kp: Right keypoint
            threshold: Maximum vertical disparity in pixels

        Returns:
            True if constraint satisfied
        """
        vertical_diff = abs(left_kp.pt[1] - right_kp.pt[1])
        return vertical_diff < threshold

    def compute_match_quality(self, matches: List, left_kp: List, right_kp: List) -> List[MatchPoint]:
        """
        Compute quality metrics for matches

        Args:
            matches: List of matches
            left_kp: Left keypoints
            right_kp: Right keypoints

        Returns:
            List of MatchPoint objects with quality scores
        """
        match_points = []

        for match in matches:
            left_pt = left_kp[match.queryIdx].pt
            right_pt = right_kp[match.trainIdx].pt

            # Calculate disparity
            disparity = abs(left_pt[0] - right_pt[0])

            # Skip invalid disparities
            if disparity < self.config.min_disparity or disparity > self.config.max_disparity:
                continue

            # Calculate depth
            if disparity > 0:
                depth = (self.config.baseline * self.config.focal_length) / disparity
            else:
                continue

            # Calculate confidence based on match distance
            confidence = 1.0 / (1.0 + match.distance / 100.0)

            match_point = MatchPoint(
                left_pt=left_pt,
                right_pt=right_pt,
                disparity=disparity,
                depth=depth,
                confidence=confidence
            )

            match_points.append(match_point)

        return match_points

    def visualize_matches(self, left_image: np.ndarray, right_image: np.ndarray,
                         left_kp: List, right_kp: List, matches: List) -> np.ndarray:
        """
        Create visualization of feature matches

        Args:
            left_image: Left eye image
            right_image: Right eye image
            left_kp: Left keypoints
            right_kp: Right keypoints
            matches: List of matches

        Returns:
            Visualization image
        """
        # Draw matches
        match_image = cv2.drawMatches(
            left_image, left_kp,
            right_image, right_kp,
            matches[:50],  # Limit to 50 for clarity
            None,
            flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS
        )

        return match_image
