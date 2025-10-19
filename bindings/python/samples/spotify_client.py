#!/usr/bin/env python
"""
Spotify API client for retrieving currently playing track information.

This module provides classes for:
- SpotifyClient: Handles Spotify API authentication and track retrieval
- SpotifyUpdateThread: Manages background updates in a separate thread
"""

import time
import requests
import base64
import json
import os
import queue
from io import BytesIO
from PIL import Image
from config import DisplayConfig


class SpotifyClient:
    """Handles Spotify API authentication and track information retrieval."""
    
    def __init__(self):
        self.client_id = os.getenv("SPOTIFY_CLIENT_ID")
        self.client_secret = os.getenv("SPOTIFY_CLIENT_SECRET")
        self.refresh_token = os.getenv("SPOTIFY_REFRESH_TOKEN")
        self.last_error_time = None
        
    def _validate_credentials(self):
        """Check if all required Spotify credentials are available."""
        if not all([self.client_id, self.client_secret, self.refresh_token]):
            print("Error: Spotify credentials not found. Please check your .env file.")
            print(f"CLIENT_ID present: {bool(self.client_id)}")
            print(f"CLIENT_SECRET present: {bool(self.client_secret)}")
            print(f"REFRESH_TOKEN present: {bool(self.refresh_token)}")
            return False
        return True
    
    def get_access_token(self):
        """Get access token using refresh token."""
        if not self._validate_credentials():
            return None
            
        auth_str = f"{self.client_id}:{self.client_secret}"
        auth_bytes = auth_str.encode("ascii")
        auth_b64 = base64.b64encode(auth_bytes).decode("ascii")
        
        headers = {
            "Authorization": f"Basic {auth_b64}",
            "Content-Type": "application/x-www-form-urlencoded"
        }
        
        data = {
            "grant_type": "refresh_token",
            "refresh_token": self.refresh_token
        }
        
        try:
            response = requests.post("https://accounts.spotify.com/api/token", headers=headers, data=data)
            print(f"Token request status: {response.status_code}")
            
            if response.status_code == 200:
                return response.json()["access_token"]
            else:
                print(f"Token request failed: {response.text}")
                return None
        except requests.RequestException as e:
            print(f"Network error getting access token: {e}")
            return None
    
    def get_current_playing(self):
        """Get currently playing track from Spotify."""
        access_token = self.get_access_token()
        if not access_token:
            print("Failed to get Spotify access token")
            return None, None, None, None
        
        headers = {"Authorization": f"Bearer {access_token}"}
        
        try:
            response = requests.get("https://api.spotify.com/v1/me/player/currently-playing", headers=headers)
            print(f"Spotify API response status: {response.status_code}")
            
            if response.status_code == 401:
                print("Unauthorized - check your Spotify credentials and refresh token")
                return None, None, None, None
            elif response.status_code == 403:
                print("Forbidden - check your Spotify app permissions")
                return None, None, None, None
            elif response.status_code == 429:
                print("Rate limited - too many requests to Spotify API")
                return None, None, None, None
            elif response.status_code == 204:
                print("No content - no music currently playing")
                return None, None, None, None
            
            if response.status_code == 200 and response.text:
                return self._parse_track_response(response)
                
        except requests.RequestException as e:
            print(f"Network error getting current playing: {e}")
            return None, None, None, None
        
        return None, None, None, None
    
    def _parse_track_response(self, response):
        """Parse the Spotify API response for track information."""
        try:
            data = response.json()
            print(f"Spotify response data keys: {data.keys() if data else 'No data'}")
            
            if data and data.get("is_playing"):
                track = data["item"]
                if track:
                    song_name = track["name"]
                    artist_name = ", ".join([artist["name"] for artist in track["artists"]])
                    album_name = track["album"]["name"]
                    print(f"Found track: {song_name} by {artist_name} from {album_name}")
                    
                    # Get album cover
                    album_images = track["album"]["images"]
                    if album_images:
                        image_url = album_images[-1]["url"]  # Get the smallest image
                        image = self._fetch_album_image(image_url)
                        return song_name, artist_name, album_name, image
                    
                    return song_name, artist_name, album_name, None
                else:
                    print("Track item is None")
            else:
                print(f"Not playing or no data. is_playing: {data.get('is_playing') if data else 'No data'}")
        except json.JSONDecodeError as e:
            print(f"JSON decode error: {e}")
        except Exception as e:
            print(f"Error parsing Spotify response: {e}")
        
        return None, None, None, None
    
    def _fetch_album_image(self, image_url):
        """Fetch and return album cover image."""
        try:
            img_response = requests.get(image_url)
            if img_response.status_code == 200:
                return Image.open(BytesIO(img_response.content))
        except requests.RequestException as e:
            print(f"Error fetching album image: {e}")
        return None
    
    def should_skip_update(self):
        """Check if we should skip update due to recent network error."""
        current_time = time.time()
        return (self.last_error_time and 
                current_time - self.last_error_time < DisplayConfig.SPOTIFY_NETWORK_ERROR_DELAY)
    
    def mark_network_error(self):
        """Mark that a network error occurred."""
        self.last_error_time = time.time()


class SpotifyUpdateThread:
    """Handles background Spotify updates using a separate thread."""
    
    def __init__(self, spotify_client, update_queue):
        self.spotify_client = spotify_client
        self.update_queue = update_queue
        self.running = True
    
    def stop(self):
        """Stop the background thread."""
        self.running = False
    
    def run(self):
        """Background thread that continuously updates Spotify data."""
        while self.running:
            try:
                current_song, current_artist, current_album, current_image = self.spotify_client.get_current_playing()
                if current_song and current_artist:
                    # Prepare the update data
                    update_data = {
                        'song_name': current_song,
                        'artist_name': current_artist.upper(),
                        'album_name': current_album if current_album else "Album Name",
                        'image': None
                    }
                    
                    if current_image:
                        # Resize image to configured size
                        resample_mode = getattr(Image, "Resampling", Image).LANCZOS
                        current_image.thumbnail((DisplayConfig.IMAGE_SIZE, DisplayConfig.IMAGE_SIZE), resample=resample_mode)
                        update_data['image'] = current_image.convert('RGB')
                    
                    # Put update in queue (non-blocking)
                    try:
                        self.update_queue.put_nowait(update_data)
                        print(f"Queued update: {current_song} by {current_artist} from {current_album}")
                    except queue.Full:
                        pass  # Skip if queue is full
                else:
                    # Put default data in queue
                    default_data = {
                        'song_name': "No music playing",
                        'artist_name': "SPOTIFY",
                        'album_name': "Album Name",
                        'image': None
                    }
                    try:
                        self.update_queue.put_nowait(default_data)
                    except queue.Full:
                        pass
                
                time.sleep(DisplayConfig.SPOTIFY_UPDATE_INTERVAL)
            except Exception as e:
                print(f"Background thread error: {e}")
                time.sleep(DisplayConfig.SPOTIFY_ERROR_RETRY_DELAY)