# 🚀 Creating GitHub Release Instructions

This guide shows how to create a GitHub Release for Drawing App v1.0.0 with the binary file.

---

## 📋 Prerequisites

- GitHub repository created: https://github.com/AndyAiCardputer/drawing-app-cardputer
- Binary file ready: `build/m5stack.esp32.m5stack_cardputer/draw-2.ino.bin`

---

## 🌐 Method 1: Via GitHub Website (Recommended)

### Step 1: Navigate to Releases

1. Go to your repository: https://github.com/AndyAiCardputer/drawing-app-cardputer
2. Click **"Releases"** (right sidebar, or click **"Releases"** tab)
3. Click **"Create a new release"** or **"Draft a new release"**

### Step 2: Fill Release Information

**Tag version:**
- Type: `v1.0.0`
- Click **"Create new tag: v1.0.0"** (if tag doesn't exist)

**Release title:**
```
🎨 Drawing App v1.0.0 - Initial Release
```

**Description:**
```markdown
# 🎨 Drawing App v1.0.0

Initial release of Drawing App for M5Stack Cardputer with Joystick2Unit support.

## ✨ Features

- 5 Brush Shapes: Circle, Square, Triangle, Star, Maple Leaf
- Rainbow Mode with automatic color cycling
- Ghost Cursor mode (Ctrl/Space) - preview without drawing
- Tube Mode - drawing with outline borders
- Velocity-based cursor - smooth movement
- SD Card support - save drawings as BMP files
- 8 Color Palette with LED visualization

## 📦 Installation

### Option 1: Upload Binary (Easier)

1. Download `draw-2.ino.bin` from this release
2. Use [esptool.py](https://github.com/espressif/esptool) to flash:
   ```bash
   esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x0 draw-2.ino.bin
   ```

### Option 2: Compile from Source

1. Clone this repository
2. Open `draw-2.ino` in Arduino IDE
3. Install required libraries (M5Cardputer, M5GFX)
4. Compile and upload

## 🎮 Controls

- **1-5**: Select brush shape
- **Joystick**: Move cursor
- **Joystick Button**: Change color (or toggle Rainbow LED speed)
- **R**: Toggle Rainbow mode
- **T**: Toggle Tube mode
- **Ctrl/Space**: Ghost cursor (preview)
- **S**: Save drawing to SD card
- **BtnA**: Clear screen
- **+/-**: Brush size

## 📸 Screenshots

See screenshots folder for examples.

## 📚 Documentation

Full documentation available in [README.md](README.md)

---

**Author:** AndyAiCardputer  
**License:** MIT  
**Hardware:** M5Stack Cardputer + Joystick2Unit
```

**Target:**
- Select: **"main"** branch

**Release:**
- ☑️ **"Set as the latest release"** (if first release)
- ☐ **"Set as a pre-release"** (uncheck for stable release)

### Step 3: Attach Binary File

1. Scroll down to **"Attach binaries"** section
2. Click **"Select your files"** or drag and drop
3. Navigate to: `build/m5stack.esp32.m5stack_cardputer/draw-2.ino.bin`
4. Select the binary file
5. Wait for upload to complete

### Step 4: Publish Release

1. Review all information
2. Click **"Publish release"** button

**Done!** Release is now live at: https://github.com/AndyAiCardputer/drawing-app-cardputer/releases

---

## 💻 Method 2: Via GitHub CLI (Advanced)

If you have GitHub CLI (`gh`) installed:

### Step 1: Create Release

```bash
cd /Users/a15/A_AI_Project/Arduino/draw-2

# Create release with binary
gh release create v1.0.0 \
  --title "🎨 Drawing App v1.0.0 - Initial Release" \
  --notes "Initial release of Drawing App for M5Stack Cardputer" \
  build/m5stack.esp32.m5stack_cardputer/draw-2.ino.bin
```

### Alternative: With detailed notes from file

```bash
# Create notes file
cat > RELEASE_NOTES.md << 'EOF'
# 🎨 Drawing App v1.0.0

Initial release...

(Your release notes here)
EOF

# Create release
gh release create v1.0.0 \
  --title "🎨 Drawing App v1.0.0 - Initial Release" \
  --notes-file RELEASE_NOTES.md \
  build/m5stack.esp32.m5stack_cardputer/draw-2.ino.bin
```

---

## ✅ Verification

After creating release, verify:

1. ✅ Release appears at: https://github.com/AndyAiCardputer/drawing-app-cardputer/releases
2. ✅ Tag `v1.0.0` is created
3. ✅ Binary file `draw-2.ino.bin` is attached
4. ✅ Release description is complete
5. ✅ Download link works

---

## 📦 Binary File Location

Binary file is located at:
```
build/m5stack.esp32.m5stack_cardputer/draw-2.ino.bin
```

**Size:** ~537 KB  
**Format:** ESP32-S3 binary (for M5Stack Cardputer)

---

## 🔗 Quick Links

- **Repository:** https://github.com/AndyAiCardputer/drawing-app-cardputer
- **Releases:** https://github.com/AndyAiCardputer/drawing-app-cardputer/releases
- **Latest Release:** https://github.com/AndyAiCardputer/drawing-app-cardputer/releases/latest

---

**Author:** AndyAiCardputer  
**Last Updated:** October 31, 2025

