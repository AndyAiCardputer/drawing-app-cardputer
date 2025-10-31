# 🎨 Drawing App for M5Stack Cardputer

A feature-rich drawing application for M5Stack Cardputer with Joystick2Unit support. Create beautiful artwork with 5 different brush shapes, rainbow colors, ghost cursor mode, and save your drawings to SD card!

![Drawing App](https://img.shields.io/badge/M5Stack-Cardputer-blue)
![Arduino](https://img.shields.io/badge/Platform-Arduino-green)
![Version](https://img.shields.io/badge/Version-1.0.0-orange)

---

## ✨ Features

### 🎨 Drawing Tools
- **5 Brush Shapes**: Circle, Square, Triangle, Star, Maple Leaf
- **8 Color Palette** with LED visualization on joystick
- **Brush Size**: Adjustable from 1 to 10 pixels
- **Rainbow Mode**: Automatic color cycling while drawing
- **Tube Mode**: Draw with outline borders
- **Ghost Cursor**: Preview shape without drawing (Ctrl/Space)

### 🎮 Controls
- **Velocity-based Cursor**: Smooth movement that reaches all screen corners
- **Joystick Control**: Precise cursor positioning
- **Keyboard Shortcuts**: Full keyboard support for all features
- **Screen Clearing**: One-button clear (BtnA)

### 💾 Save & Load
- **SD Card Support**: Save drawings as BMP files
- **Auto-increment**: Files named `draw_001.bmp`, `draw_002.bmp`, etc.
- **Standard BMP Format**: Opens on any computer

### 🌈 Rainbow Features
- **Color Cycling**: Colors automatically change while drawing
- **LED Animation**: Joystick LED cycles through all colors
- **3 Speed Levels**: Slow, Medium, Fast (adjustable via joystick button)

---

## 📋 Requirements

### Hardware
- **M5Stack Cardputer v1.1** (ESP32-S3)
- **Joystick2Unit** (I2C 0x63) - connected via Grove port
- **MicroSD Card** (FAT32 formatted) - optional, for saving drawings

### Software
- **Arduino IDE** (1.8.19 or newer) or **PlatformIO**
- **M5Cardputer Library** (via Arduino Library Manager)
- **M5GFX Library** (via Arduino Library Manager)

---

## 🚀 Installation

### 1. Install Arduino IDE

Download and install Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)

### 2. Add M5Stack Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to **Additional Board Manager URLs**:
   ```
   https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "**M5Stack**" and install "**M5Stack Boards**"

### 3. Install Required Libraries

In Arduino IDE, go to **Tools → Manage Libraries** and install:

- **M5Cardputer** (by M5Stack)
- **M5GFX** (by M5Stack)
- **SD** (built-in)
- **SPI** (built-in)

### 4. Upload Code

1. Connect M5Stack Cardputer via USB-C
2. Select board: **Tools → Board → M5Stack Arduino → M5Stack Cardputer**
3. Select port: **Tools → Port → (your COM port)**
4. Open `draw-2.ino`
5. Click **Upload** button

---

## 🎮 Usage

### Starting the App

1. Power on M5Stack Cardputer
2. Wait for startup screen
3. Press **ESC** or **Joystick Button** to enter DRAW mode

### Basic Drawing

- **Move Cursor**: Use joystick
- **Draw**: Move cursor while not in ghost mode
- **Change Color**: Press joystick button (or `R` for Rainbow mode)
- **Select Brush**: Press keys `1`-`5`:
  - `1` = Circle
  - `2` = Square
  - `3` = Triangle
  - `4` = Star
  - `5` = Maple Leaf

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `1`-`5` | Select brush shape |
| `+` / `-` | Increase/decrease brush size |
| `T` | Toggle Tube mode (outline) |
| `R` | Toggle Rainbow mode |
| `X` / `Y` | Invert X/Y axis |
| `Ctrl` / `Space` | Ghost cursor (preview without drawing) |
| `S` | Save drawing to SD card |
| `BtnA` | Clear screen |

### Rainbow Mode

1. Press `R` to enable Rainbow mode
2. Start drawing - colors will cycle automatically
3. Press joystick button to change LED speed:
   - **Slow** (500ms)
   - **Medium** (200ms) - default
   - **Fast** (100ms)

### Saving Drawings

1. Press `S` key while in DRAW mode
2. Drawing is saved to SD card as `draw_XXX.bmp`
3. Files auto-increment: `draw_001.bmp`, `draw_002.bmp`, etc.
4. Files can be opened on any computer (standard BMP format)

---

## 🎯 Advanced Features

### Ghost Cursor Mode

- **Purpose**: Preview shape position without drawing
- **Activation**: Hold `Ctrl` or `Space`
- **Features**:
  - Shows shape outline
  - Doesn't erase background
  - Useful for precise positioning

### Tube Mode

- **Purpose**: Draw shapes with visible outline borders
- **Activation**: Press `T`
- **Effect**: Draws filled shape with outline border

### Velocity-based Cursor

- **Technology**: Professional game development technique
- **Benefit**: Smooth cursor movement that reaches all screen corners
- **How it works**: Converts joystick input to velocity, integrates position over time

---

## 📁 Project Structure

```
draw-2/
├── draw-2.ino          # Main application code
├── README.md           # This file
├── LICENSE             # MIT License
└── _versions/          # Version backups
    └── draw-2_v6_GHOST_OUTLINE_RAINBOW_FIX_20251031_004719.ino
```

---

## 🐛 Troubleshooting

### Joystick Not Found

**Problem**: "Joystick2Unit NOT found!" error

**Solutions**:
1. Check Joystick2Unit is connected to Grove port
2. Verify I2C address is 0x63
3. Try reconnecting the joystick

### SD Card Not Working

**Problem**: Can't save drawings

**Solutions**:
1. Format SD card as FAT32
2. Check SD card is inserted correctly
3. Verify SD card is not write-protected
4. Try a different SD card

### Colors Not Showing

**Problem**: LED colors not changing

**Solutions**:
1. Check joystick button functionality
2. Verify Rainbow mode is enabled (`R` key)
3. Try pressing joystick button to cycle colors manually

---

## 📝 Technical Details

### Screen Resolution
- **Width**: 240 pixels
- **Height**: 135 pixels
- **Color Depth**: RGB565 (16-bit)

### Color Palette
- 8 predefined colors: Red, Green, Blue, Yellow, Magenta, Cyan, White, Black
- Rainbow mode cycles through all colors

### Brush Shapes
1. **Circle**: Filled circle with radius = brush size
2. **Square**: Filled square with side = 2 × brush size
3. **Triangle**: Equilateral triangle pointing up
4. **Star**: 5-pointed star
5. **Maple**: Maple leaf shape (simplified)

### Save Format
- **Format**: BMP (Windows Bitmap)
- **Resolution**: 240 × 135 pixels
- **Color**: 24-bit RGB
- **File Size**: ~97 KB per drawing

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Guidelines
- Follow Arduino coding standards
- Add comments to new code
- Test thoroughly before submitting
- Update documentation if needed

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Credits

- **Created by**: Andy + AI
- **Hardware**: M5Stack Cardputer + Joystick2Unit
- **Libraries**: M5Cardputer, M5GFX by M5Stack
- **Based on**: joystick_complete_test example

---

## 🔗 Links

- **M5Stack Official**: https://m5stack.com
- **M5Stack Cardputer**: https://docs.m5stack.com/en/core/Cardputer
- **GitHub Profile**: https://github.com/AndyAiCardputer

---

## 📸 Screenshots

_Add screenshots of your drawings here!_

---

## 🎉 Enjoy Drawing!

Have fun creating amazing artwork on your M5Stack Cardputer! 🎨✨

---

**Version**: 1.0.0  
**Last Updated**: October 31, 2025  
**Author**: AndyAiCardputer

