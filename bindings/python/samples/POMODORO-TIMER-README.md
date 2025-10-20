# 🍅 Pomodoro Timer for RGB LED Matrix

A visual 25-minute pomodoro timer designed for RGB LED matrices, perfect for focused work sessions with break reminders.

## Features

- **25-minute work sessions** with visual countdown
- **5-minute break timers** for proper rest
- **Color-coded display**:
  - 🟢 Green: Focus time (normal)
  - 🟠 Orange: Last 5 minutes warning
  - 🔴 Red: Last minute urgent
  - 🔵 Blue: Break time
  - ⚪ Gray: Paused
- **Progress bar** showing session completion
- **Session counter** to track completed pomodoros
- **Visual notifications** when sessions complete
- **Keyboard controls** for easy management

## Files

- `pomodoro-timer.py` - Full version for LED matrix hardware
- `pomodoro-timer-test.py` - Console test version (10s work, 5s break for testing)

## Controls

- **SPACE** - Start/Pause timer
- **R** - Reset timer to work session
- **B** - Start break timer (available after completing work session)
- **H** - Show help
- **Q** - Quit

## Usage

### On LED Matrix Hardware
```bash
cd /path/to/rpi-rgb-led-matrix/bindings/python/samples
sudo python3 pomodoro-timer.py
```

### Testing (Console Version)
```bash
python3 pomodoro-timer-test.py
```

## Display Layout

```
┌─────────────────────────────┐
│ SPACE=Start R=Reset Q=Quit  │  <- Controls (when paused)
│          FOCUS              │  <- Session status
│         24:59               │  <- Countdown timer
│ Sessions: 2                 │  <- Completed sessions
│ ████████████████░░░░        │  <- Progress bar
└─────────────────────────────┘
```

## Pomodoro Technique

The Pomodoro Technique is a time management method:

1. **Work** for 25 minutes with full focus
2. **Take a short break** (5 minutes)
3. **Repeat** the cycle
4. After 4 pomodoros, take a longer break (15-30 minutes)

This timer helps you implement the technique with clear visual feedback!

## Requirements

- RGB LED Matrix (32x64 recommended)
- Raspberry Pi with rpi-rgb-led-matrix library
- Python 3 with rgbmatrix bindings

## Customization

You can modify the timer durations in the code:
- `work_duration = 25 * 60` (25 minutes)
- `break_duration = 5 * 60` (5 minutes)

Color schemes and display layout can also be customized in the respective class variables.