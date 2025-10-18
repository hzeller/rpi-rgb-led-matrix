#!/usr/bin/env python3
"""
Spotify OAuth helper to get refresh token
Run this once to get your refresh token, then add it to your .env file
"""

import requests
import base64
import urllib.parse
import webbrowser
import os
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

CLIENT_ID = os.getenv("SPOTIFY_CLIENT_ID")
CLIENT_SECRET = os.getenv("SPOTIFY_CLIENT_SECRET")

# Required scopes for the music display
SCOPES = "user-read-currently-playing user-read-playback-state"
REDIRECT_URI = "http://localhost:8888/callback"

def get_auth_url():
    """Generate the authorization URL"""
    params = {
        "client_id": CLIENT_ID,
        "response_type": "code",
        "redirect_uri": REDIRECT_URI,
        "scope": SCOPES,
        "show_dialog": "true"
    }
    
    auth_url = "https://accounts.spotify.com/authorize?" + urllib.parse.urlencode(params)
    return auth_url

def get_tokens(auth_code):
    """Exchange authorization code for access and refresh tokens"""
    auth_str = f"{CLIENT_ID}:{CLIENT_SECRET}"
    auth_bytes = auth_str.encode("ascii")
    auth_b64 = base64.b64encode(auth_bytes).decode("ascii")
    
    headers = {
        "Authorization": f"Basic {auth_b64}",
        "Content-Type": "application/x-www-form-urlencoded"
    }
    
    data = {
        "grant_type": "authorization_code",
        "code": auth_code,
        "redirect_uri": REDIRECT_URI
    }
    
    response = requests.post("https://accounts.spotify.com/api/token", headers=headers, data=data)
    
    if response.status_code == 200:
        tokens = response.json()
        return tokens["access_token"], tokens["refresh_token"]
    else:
        print(f"Error getting tokens: {response.status_code}")
        print(response.text)
        return None, None

def main():
    if not CLIENT_ID or not CLIENT_SECRET:
        print("Error: Please set SPOTIFY_CLIENT_ID and SPOTIFY_CLIENT_SECRET in your .env file first!")
        print("\nTo get these:")
        print("1. Go to https://developer.spotify.com/dashboard")
        print("2. Create a new app")
        print("3. Copy the Client ID and Client Secret")
        print("4. Add them to your .env file")
        return
    
    print("Spotify OAuth Setup")
    print("===================")
    print(f"Client ID: {CLIENT_ID}")
    print(f"Redirect URI: {REDIRECT_URI}")
    print(f"Scopes: {SCOPES}")
    print()
    
    # Step 1: Get authorization URL
    auth_url = get_auth_url()
    print("Step 1: Opening browser for Spotify authorization...")
    print(f"If browser doesn't open, go to: {auth_url}")
    print()
    
    webbrowser.open(auth_url)
    
    # Step 2: Get authorization code from user
    print("Step 2: After authorizing, you'll be redirected to a localhost URL that won't load.")
    print("Copy the ENTIRE URL from your browser's address bar and paste it here.")
    print("It should look like: http://localhost:8888/callback?code=...")
    print()
    
    redirect_url = input("Paste the redirect URL here: ").strip()
    
    # Extract the code from the URL
    if "code=" in redirect_url:
        auth_code = redirect_url.split("code=")[1].split("&")[0]
        print(f"Extracted code: {auth_code[:20]}...")
        
        # Step 3: Exchange code for tokens
        print("\nStep 3: Exchanging code for tokens...")
        access_token, refresh_token = get_tokens(auth_code)
        
        if refresh_token:
            print("\n✅ Success! Here's your refresh token:")
            print("=" * 50)
            print(refresh_token)
            print("=" * 50)
            print("\nAdd this to your .env file as:")
            print(f"SPOTIFY_REFRESH_TOKEN={refresh_token}")
            print("\nYour .env file should now contain:")
            print(f"SPOTIFY_CLIENT_ID={CLIENT_ID}")
            print(f"SPOTIFY_CLIENT_SECRET={CLIENT_SECRET}")
            print(f"SPOTIFY_REFRESH_TOKEN={refresh_token}")
        else:
            print("❌ Failed to get refresh token")
    else:
        print("❌ No authorization code found in URL")

if __name__ == "__main__":
    main()