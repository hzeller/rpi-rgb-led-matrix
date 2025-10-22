"""
Shared components for RGB LED Matrix projects.

This package provides common utilities and base classes for all matrix projects,
reducing code duplication and ensuring consistency across applications.
"""

from .matrix_base import MatrixBase, SampleMatrixBase
from .font_manager import FontManager
from .color_palette import ColorPalette
from .image_utils import ImageUtils
from .config_manager import ConfigManager, MatrixProjectConfig

__all__ = [
    'MatrixBase', 'SampleMatrixBase', 'FontManager', 'ColorPalette', 
    'ImageUtils', 'ConfigManager', 'MatrixProjectConfig'
]