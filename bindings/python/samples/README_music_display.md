# Music Display for RGB LED Matrix

A modular music display application for Raspberry Pi RGB LED matrices that shows currently playing Spotify tracks or static content with smooth scrolling text and album art.

## File Structure

### Core Modules

- **`music_display.py`** - Main application entry point and orchestration
- **`spotify_client.py`** - Spotify API integration and background updates  
- **`display.py`** - LED matrix display management and text rendering
- **`config.py`** - Configuration constants and settings

### Legacy Files

- **`image-viewer.py`** - Original monolithic implementation (deprecated)

## Features

- 🎵 **Spotify Integration**: Real-time display of currently playing music
- 🖼️ **Album Art**: Automatic album cover fetching and display
- 📜 **Smooth Scrolling**: Seamless text scrolling for long titles
- ⚙️ **Configurable**: Easy customization via config file
- 🎯 **Static Mode**: Display custom images and text without Spotify

## Usage

### Spotify Mode (Default)
```bash
python music_display.py
```
*Requires `.env` file with Spotify credentials*

### Static Mode
```bash
python music_display.py <image_file> [song_name] [artist_name] [album_name]
```

## Configuration

All settings can be customized in `config.py`:

- Matrix hardware settings (rows, cols, hardware mapping)
- Display layout (padding, image size, colors)
- Timing (refresh rate, scroll speed, delays)
- Font paths and Spotify update intervals

## Architecture

The application follows a modular, object-oriented design:

- **Separation of Concerns**: Each module has a single responsibility
- **DRY Principles**: No code duplication between text rendering
- **Error Handling**: Robust network error management and retry logic
- **Threading**: Non-blocking Spotify updates via background thread
- **Configurability**: Centralized settings for easy customization

## Classes

### `MusicDisplay`
Main application orchestrator that coordinates all components.

### `SpotifyClient` 
Handles Spotify API authentication and track information retrieval.

### `SpotifyUpdateThread`
Manages background Spotify updates without blocking the display.

### `MatrixDisplay`
Encapsulates LED matrix operations and rendering logic.

### `ScrollableText`
Reusable component for smooth text scrolling with configurable styling.

### `DisplayConfig`
Centralized configuration constants for the entire application.

## Requirements

- Raspberry Pi with RGB LED matrix
- Python 3.7+
- Required packages: `rgbmatrix`, `PIL`, `requests`, `python-dotenv`
- Spotify Developer Account (for Spotify mode)

## Migration from image-viewer.py

The original `image-viewer.py` has been refactored into this modular structure. The new implementation:

- ✅ Eliminates ~200+ lines of duplicated code
- ✅ Improves maintainability and readability  
- ✅ Provides better error handling
- ✅ Enables easier feature additions
- ✅ Maintains all original functionality