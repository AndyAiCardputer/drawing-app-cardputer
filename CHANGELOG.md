# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-31

### Added
- Initial release of Drawing App for M5Stack Cardputer
- 5 brush shapes: Circle, Square, Triangle, Star, Maple Leaf
- 8-color palette with LED visualization
- Rainbow mode with automatic color cycling
- Ghost cursor mode (Ctrl/Space) - preview without drawing
- Tube mode - drawing with outline borders
- Velocity-based cursor system - smooth movement, reaches all corners
- Brush size adjustment (1-10 pixels)
- SD card support - save drawings as BMP files
- Auto-increment file naming (`draw_001.bmp`, `draw_002.bmp`, etc.)
- Joystick LED rainbow animation with 3 speed levels
- Screen clearing (BtnA)
- Axis inversion (X/Y keys)
- Startup screen with instructions

### Features
- **Drawing Tools**: 5 shapes, 8 colors, adjustable brush size
- **Rainbow Mode**: Automatic color cycling while drawing, LED animation
- **Ghost Cursor**: Preview shape position without drawing
- **Tube Mode**: Draw shapes with visible outline borders
- **Velocity-based Cursor**: Professional smooth movement system
- **SD Card Saving**: Save drawings as standard BMP format

### Technical
- Based on joystick_complete_test example
- Uses M5Cardputer and M5GFX libraries
- Optimized outline saving for ghost cursor (all sizes)
- Fixed rainbow mode stripe artifacts for circle
- RGB565 to RGB24 conversion for BMP saving

---

**Note**: Version history is maintained in `_versions/` folder for rollback purposes.

