# 📦 Binary File Information

This document explains where to find and how to use the compiled binary file.

---

## 🔍 Binary Location

The compiled binary file is located at:

```
build/m5stack.esp32.m5stack_cardputer/draw-2.ino.bin
```

**File Size:** ~537 KB  
**Format:** ESP32-S3 binary (for M5Stack Cardputer)  
**Target:** M5Stack Cardputer v1.1

---

## 📥 Download Options

### Option 1: From GitHub Release (Recommended)

The binary file will be available in the [Releases](https://github.com/AndyAiCardputer/drawing-app-cardputer/releases) section once v1.0.0 release is created.

**How to download:**
1. Go to: https://github.com/AndyAiCardputer/drawing-app-cardputer/releases
2. Find **v1.0.0** release
3. Download `draw-2.ino.bin` from **Assets**

### Option 2: Build from Source

If you prefer to compile from source:

```bash
# Clone repository
git clone https://github.com/AndyAiCardputer/drawing-app-cardputer.git
cd drawing-app-cardputer

# Open in Arduino IDE
# Install required libraries
# Compile and upload
```

---

## 🔧 Flashing Instructions

### Using esptool.py

**Install esptool:**
```bash
pip install esptool
```

**Flash binary:**
```bash
esptool.py --chip esp32s3 \
  --port /dev/ttyUSB0 \
  --baud 921600 \
  write_flash \
  0x0 draw-2.ino.bin
```

**Windows:**
```bash
esptool.py --chip esp32s3 ^
  --port COM3 ^
  --baud 921600 ^
  write_flash ^
  0x0 draw-2.ino.bin
```

### Using Arduino IDE

1. Open Arduino IDE
2. Select board: **M5Stack Cardputer**
3. Select port: Your COM port
4. Go to **Sketch → Export compiled Binary**
5. Binary will be saved in `build/` folder

### Using PlatformIO

```bash
pio run --target upload
```

---

## ⚠️ Important Notes

- **Binary size:** ~537 KB (fits in ESP32-S3 flash)
- **Partition table:** Uses default M5Stack Cardputer partition scheme
- **Flash mode:** QIO
- **Flash frequency:** 80MHz
- **Flash size:** 8MB (default)

---

## 🔗 Quick Links

- **Releases:** https://github.com/AndyAiCardputer/drawing-app-cardputer/releases
- **Latest Release:** https://github.com/AndyAiCardputer/drawing-app-cardputer/releases/latest
- **Repository:** https://github.com/AndyAiCardputer/drawing-app-cardputer

---

**Note:** Binary file is NOT included in Git repository (see `.gitignore`). It will be available in GitHub Releases.

