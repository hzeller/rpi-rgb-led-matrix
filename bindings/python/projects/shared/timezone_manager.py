#!/usr/bin/env python3
"""
Timezone Manager - Shared timezone handling utility

Provides timezone detection and management functionality for LED matrix projects.
Supports multiple methods for timezone detection:
1. TimeZoneDB API (requires API key)
2. Coordinate-based estimation using pytz
3. Manual calculation fallback
4. Simple Denver/Mountain Time default
"""

import os
import requests
from datetime import datetime, timezone, timedelta

# Try to import timezone libraries
try:
    from zoneinfo import ZoneInfo  # Python 3.9+
    TIMEZONE_MODULE = 'zoneinfo'
except ImportError:
    try:
        import pytz  # Fallback for older Python versions
        TIMEZONE_MODULE = 'pytz'
    except ImportError:
        TIMEZONE_MODULE = None


class TimezoneManager:
    """Manages timezone detection and conversion for LED matrix displays."""
    
    def __init__(self, default_timezone='America/Denver'):
        """
        Initialize timezone manager.
        
        Args:
            default_timezone: Default timezone to use if detection fails
        """
        self.default_timezone = default_timezone
        self.current_timezone = None
        self.timezone_name = None
    
    def get_timezone_for_coordinates(self, lat=None, lng=None, fallback_to_denver=True):
        """
        Get timezone for given coordinates using multiple detection methods.
        
        Args:
            lat: Latitude (optional)
            lng: Longitude (optional)
            fallback_to_denver: If True, falls back to Denver timezone when coordinates unavailable
            
        Returns:
            timezone object or None if all methods fail
        """
        try:
            # Method 1: If no coordinates, use Denver default
            if lat is None or lng is None:
                if fallback_to_denver:
                    print("No coordinates available. Using Denver/Mountain Time.")
                    return self._set_denver_timezone()
                else:
                    print("No coordinates available. Using system local time.")
                    return None
            
            # Method 2: Try TimeZoneDB API (requires API key)
            timezone_obj = self._try_timezonedb_api(lat, lng)
            if timezone_obj:
                return timezone_obj
            
            # Method 3: Use coordinate-based estimation with pytz/zoneinfo
            timezone_obj = self._try_coordinate_estimation(lat, lng)
            if timezone_obj:
                return timezone_obj
            
            # Method 4: Fallback to manual calculation
            timezone_obj = self._try_manual_calculation(lat, lng)
            if timezone_obj:
                return timezone_obj
            
            # Method 5: Final fallback to Denver
            if fallback_to_denver:
                print("All timezone detection methods failed. Using Denver/Mountain Time.")
                return self._set_denver_timezone()
            else:
                print("All timezone detection methods failed. Using system local time.")
                return None
                
        except Exception as e:
            print(f"Timezone detection error: {e}")
            if fallback_to_denver:
                return self._set_denver_timezone()
            return None
    
    def get_denver_timezone(self):
        """Get Denver/Mountain Time timezone object."""
        return self._set_denver_timezone()
    
    def get_current_time(self, timezone_obj=None):
        """
        Get current time in specified timezone.
        
        Args:
            timezone_obj: Timezone object (uses self.current_timezone if None)
            
        Returns:
            datetime object in specified timezone
        """
        if timezone_obj is None:
            timezone_obj = self.current_timezone
            
        if not TIMEZONE_MODULE or not timezone_obj:
            return datetime.now()
        
        try:
            if TIMEZONE_MODULE == 'zoneinfo':
                return datetime.now(timezone_obj)
            else:  # pytz
                if hasattr(timezone_obj, 'localize'):
                    # pytz timezone object
                    utc_now = datetime.utcnow().replace(tzinfo=pytz.utc)
                    return utc_now.astimezone(timezone_obj)
                else:
                    # Standard timezone object
                    return datetime.now(timezone_obj)
        except Exception:
            return datetime.now()
    
    def _set_denver_timezone(self):
        """Set up Denver/Mountain Time timezone."""
        try:
            if TIMEZONE_MODULE == 'zoneinfo':
                self.current_timezone = ZoneInfo('America/Denver')
                self.timezone_name = 'America/Denver'
                print("🏔️  Using Denver/Mountain Time (zoneinfo)")
            elif TIMEZONE_MODULE == 'pytz':
                self.current_timezone = pytz.timezone('America/Denver')
                self.timezone_name = 'America/Denver'
                print("🏔️  Using Denver/Mountain Time (pytz)")
            else:
                # Fallback to manual Mountain Time (UTC-7/-6 for DST)
                self.current_timezone = timezone(timedelta(hours=-7))
                self.timezone_name = 'Mountain Time (Manual)'
                print("🏔️  Using Mountain Time (Manual UTC-7)")
            
            return self.current_timezone
        except Exception as e:
            print(f"⚠️  Could not set Denver timezone: {e}")
            # Final fallback to manual Mountain Time
            self.current_timezone = timezone(timedelta(hours=-7))
            self.timezone_name = 'Mountain Time (Manual)'
            return self.current_timezone
    
    def _try_timezonedb_api(self, lat, lng):
        """Try TimeZoneDB API for timezone detection."""
        try:
            timezonedb_key = os.getenv('TIMEZONEDB_API_KEY')
            if not timezonedb_key:
                return None
            
            tz_api_url = "http://api.timezonedb.com/v2.1/get-time-zone"
            params = {
                'key': timezonedb_key,
                'format': 'json',
                'by': 'position',
                'lat': lat,
                'lng': lng
            }
            
            response = requests.get(tz_api_url, params=params, timeout=5)
            if response.status_code == 200:
                tz_data = response.json()
                if tz_data.get('status') == 'OK':
                    offset_seconds = tz_data['gmtOffset']
                    offset_hours = offset_seconds / 3600
                    self.current_timezone = timezone(timedelta(seconds=offset_seconds))
                    self.timezone_name = tz_data.get('zoneName', 'TimeZoneDB')
                    print(f"🌍 TimeZoneDB timezone: UTC{offset_hours:+.1f} ({self.timezone_name})")
                    return self.current_timezone
        except Exception as e:
            print(f"TimeZoneDB API error: {e}")
        
        return None
    
    def _try_coordinate_estimation(self, lat, lng):
        """Try coordinate-based timezone estimation using pytz/zoneinfo."""
        if not TIMEZONE_MODULE:
            return None
        
        try:
            timezone_name = None
            
            # North America timezone mapping
            if -125 <= lng <= -60 and 25 <= lat <= 70:
                if lng >= -75:  # Eastern
                    timezone_name = 'America/New_York'
                elif lng >= -90:  # Central
                    timezone_name = 'America/Chicago'
                elif lng >= -105:  # Mountain
                    timezone_name = 'America/Denver'
                else:  # Pacific
                    timezone_name = 'America/Los_Angeles'
            
            # Europe timezone mapping
            elif -10 <= lng <= 40 and 35 <= lat <= 70:
                if lng <= 15:
                    timezone_name = 'Europe/Berlin'  # CET
                else:
                    timezone_name = 'Europe/Athens'  # EET
            
            # Asia timezone mapping
            elif 100 <= lng <= 150 and 20 <= lat <= 50:
                if lng <= 120:
                    timezone_name = 'Asia/Shanghai'  # China
                else:
                    timezone_name = 'Asia/Tokyo'    # Japan
            
            if timezone_name:
                if TIMEZONE_MODULE == 'zoneinfo':
                    self.current_timezone = ZoneInfo(timezone_name)
                else:  # pytz
                    self.current_timezone = pytz.timezone(timezone_name)
                
                self.timezone_name = timezone_name
                print(f"🗺️  Coordinate-based timezone: {timezone_name}")
                return self.current_timezone
                
        except Exception as e:
            print(f"Coordinate estimation error: {e}")
        
        return None
    
    def _try_manual_calculation(self, lat, lng):
        """Try manual timezone calculation based on longitude."""
        try:
            print("Using fallback timezone calculation")
            
            if -125 <= lng <= -60:  # North America
                if lng >= -75:
                    offset = -5  # Eastern
                    zone_name = "Eastern (Manual)"
                elif lng >= -90:
                    offset = -6  # Central
                    zone_name = "Central (Manual)"
                elif lng >= -105:
                    offset = -7  # Mountain
                    zone_name = "Mountain (Manual)"
                else:
                    offset = -8  # Pacific
                    zone_name = "Pacific (Manual)"
            elif -10 <= lng <= 40:  # Europe
                if lng <= 15:
                    offset = 1
                    zone_name = "CET (Manual)"
                else:
                    offset = 2
                    zone_name = "EET (Manual)"
            elif 100 <= lng <= 150:  # Asia
                if lng <= 120:
                    offset = 8
                    zone_name = "China (Manual)"
                else:
                    offset = 9
                    zone_name = "Japan (Manual)"
            else:
                offset = round(lng / 15)
                zone_name = f"UTC{offset:+d} (Manual)"
            
            self.current_timezone = timezone(timedelta(hours=offset))
            self.timezone_name = zone_name
            print(f"📍 Manual timezone calculation: {zone_name}")
            return self.current_timezone
            
        except Exception as e:
            print(f"Manual calculation error: {e}")
        
        return None
    
    def get_timezone_name(self):
        """Get the current timezone name."""
        return self.timezone_name or "Unknown"
    
    def get_timezone_abbreviation(self):
        """Get a short abbreviation for the current timezone."""
        if not self.current_timezone:
            return "LOC"
        
        try:
            # Get current time to determine abbreviation
            current_time = self.get_current_time()
            abbrev = current_time.strftime('%Z')
            if abbrev:
                return abbrev[:3]  # Limit to 3 characters
            
            # Fallback to timezone name abbreviation
            if self.timezone_name:
                if '/' in self.timezone_name:
                    return self.timezone_name.split('/')[-1][:3].upper()
                else:
                    return self.timezone_name[:3].upper()
            
            return "UTC"
        except Exception:
            return "UTC"


# Convenience functions for simple usage
def get_denver_timezone():
    """Get Denver/Mountain Time timezone - simple function for basic usage."""
    manager = TimezoneManager()
    return manager.get_denver_timezone()


def get_timezone_for_location(lat=None, lng=None):
    """Get timezone for coordinates - simple function for basic usage."""
    manager = TimezoneManager()
    return manager.get_timezone_for_coordinates(lat, lng)


def get_current_time_denver():
    """Get current time in Denver/Mountain Time - simple function for basic usage."""
    manager = TimezoneManager()
    denver_tz = manager.get_denver_timezone()
    return manager.get_current_time(denver_tz)