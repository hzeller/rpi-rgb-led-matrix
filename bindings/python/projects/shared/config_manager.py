#!/usr/bin/env python
"""
ConfigManager - Configuration management for RGB LED Matrix projects.

Provides centralized configuration handling with environment variable support,
default values, and type validation.
"""

import os
from typing import Any, Dict, Optional, Union, Type
from dotenv import load_dotenv


class ConfigManager:
    """
    Centralized configuration management for matrix applications.
    
    Supports environment variables, default values, and type validation.
    Can load configuration from .env files automatically.
    """
    
    def __init__(self, env_file: Optional[str] = None, auto_load_env: bool = True):
        """
        Initialize configuration manager.
        
        Args:
            env_file: Specific .env file to load (optional)
            auto_load_env: Whether to automatically load .env file
        """
        self._config = {}
        
        if auto_load_env:
            if env_file and os.path.exists(env_file):
                load_dotenv(env_file)
            else:
                load_dotenv()  # Load from current directory or parent directories
                
    def get(self, key: str, default: Any = None, value_type: Type = str) -> Any:
        """
        Get a configuration value.
        
        Args:
            key: Configuration key (will also check environment variables)
            default: Default value if key not found
            value_type: Type to convert the value to (str, int, float, bool)
            
        Returns:
            Configuration value converted to specified type
        """
        # Check cache first
        if key in self._config:
            return self._config[key]
            
        # Check environment variables
        env_value = os.getenv(key)
        if env_value is not None:
            converted_value = self._convert_value(env_value, value_type)
            self._config[key] = converted_value
            return converted_value
            
        # Return default
        self._config[key] = default
        return default
        
    def set(self, key: str, value: Any):
        """
        Set a configuration value.
        
        Args:
            key: Configuration key
            value: Configuration value
        """
        self._config[key] = value
        
    def update(self, config_dict: Dict[str, Any]):
        """
        Update configuration with multiple values.
        
        Args:
            config_dict: Dictionary of configuration values
        """
        self._config.update(config_dict)
        
    def _convert_value(self, value: str, value_type: Type) -> Any:
        """
        Convert string value to specified type.
        
        Args:
            value: String value from environment variable
            value_type: Target type
            
        Returns:
            Converted value
        """
        if value_type == str:
            return value
        elif value_type == int:
            return int(value)
        elif value_type == float:
            return float(value)
        elif value_type == bool:
            return value.lower() in ('true', '1', 'yes', 'on')
        else:
            return value
            
    def get_matrix_config(self) -> Dict[str, Any]:
        """
        Get standard matrix configuration values.
        
        Returns:
            Dictionary with matrix configuration
        """
        return {
            'rows': self.get('MATRIX_ROWS', 32, int),
            'cols': self.get('MATRIX_COLS', 64, int),
            'chain_length': self.get('MATRIX_CHAIN_LENGTH', 1, int),
            'parallel': self.get('MATRIX_PARALLEL', 1, int),
            'hardware_mapping': self.get('MATRIX_HARDWARE_MAPPING', 'adafruit-hat-pwm'),
            'brightness': self.get('MATRIX_BRIGHTNESS', 60, int)
        }
        
    def get_brightness_config(self) -> Dict[str, Any]:
        """
        Get brightness-related configuration values.
        
        Returns:
            Dictionary with brightness configuration
        """
        return {
            'default_brightness': self.get('MATRIX_BRIGHTNESS', 60, int),
            'min_brightness': self.get('BRIGHTNESS_MIN', 1, int),
            'max_brightness': self.get('BRIGHTNESS_MAX', 100, int),
            'auto_dim_enabled': self.get('BRIGHTNESS_AUTO_DIM', False, bool),
            'auto_dim_time': self.get('BRIGHTNESS_AUTO_DIM_TIME', '22:00'),
            'auto_dim_level': self.get('BRIGHTNESS_AUTO_DIM_LEVEL', 20, int),
            'schedule_enabled': self.get('BRIGHTNESS_SCHEDULE_ENABLED', False, bool)
        }
        
    def get_api_config(self, service: str) -> Dict[str, str]:
        """
        Get API configuration for a specific service.
        
        Args:
            service: Service name (e.g., 'OPENWEATHER', 'SPOTIFY')
            
        Returns:
            Dictionary with API configuration
        """
        base_key = service.upper()
        return {
            'api_key': self.get(f'{base_key}_API_KEY'),
            'base_url': self.get(f'{base_key}_BASE_URL'),
            'client_id': self.get(f'{base_key}_CLIENT_ID'),
            'client_secret': self.get(f'{base_key}_CLIENT_SECRET')
        }
        
    def to_dict(self) -> Dict[str, Any]:
        """
        Get all configuration as a dictionary.
        
        Returns:
            Dictionary with all configuration values
        """
        return self._config.copy()


# Pre-configured managers for common use cases
class MatrixProjectConfig(ConfigManager):
    """
    Configuration manager with common defaults for matrix projects.
    """
    
    def __init__(self, project_name: str = None, **kwargs):
        """
        Initialize with matrix project defaults.
        
        Args:
            project_name: Optional project name for environment variable prefixes
            **kwargs: Additional arguments passed to ConfigManager
        """
        super().__init__(**kwargs)
        
        self.project_name = project_name
        
        # Set common defaults
        self.update({
            'refresh_rate': 0.1,
            'font_size': 'medium',
            'theme': 'default',
            'debug': False
        })
        
    def get_project_setting(self, setting: str, default: Any = None, value_type: Type = str) -> Any:
        """
        Get a project-specific setting with optional project name prefix.
        
        Args:
            setting: Setting name
            default: Default value
            value_type: Value type
            
        Returns:
            Setting value
        """
        if self.project_name:
            # Try project-specific first
            project_key = f"{self.project_name.upper()}_{setting.upper()}"
            value = self.get(project_key, None, value_type)
            if value is not None:
                return value
                
        # Fall back to generic setting
        return self.get(setting.upper(), default, value_type)