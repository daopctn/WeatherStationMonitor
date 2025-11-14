"""
Human-like binocular vision system
Two monocular cameras working together like human eyes
"""

from .binocular_vision import BinocularVisionSystem, BinocularConfig
from .correspondence_matcher import CorrespondenceMatcher, MatchPoint
from .temporal_fusion import TemporalDepthFusion
from .visual_attention import VisualAttention, AttentionRegion

__all__ = [
    'BinocularVisionSystem',
    'BinocularConfig',
    'CorrespondenceMatcher',
    'MatchPoint',
    'TemporalDepthFusion',
    'VisualAttention',
    'AttentionRegion'
]
