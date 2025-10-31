/*
 * 🎨 Drawing App by Andy +Ai for M5Stack Cardputer
 * 
 * Clean version - Based on joystick_complete_test
 * DRAW mode only - 5 shapes (Circle, Square, Triangle, Star, Maple)
 * 
 * Features:
 * - 5 Brush Shapes (1-5 keys)
 * - Ghost cursor (Ctrl/Space) - move without drawing
 * - Velocity-based cursor - reaches all corners!
 * - 8 Colors with LED visualization
 * - Tube mode (T key) - drawing with outline
 * 
 * Controls:
 * - 1-5: Select brush shape
 * - Joystick: Move cursor
 * - Joystick Button: Change color
 * - Space/Ctrl: Pause drawing (ghost cursor)
 * - T: Toggle tube mode (outline)
 * - +/-: Brush size
 * - X,Y: Invert axes
 * - BtnA: Clear screen
 * 
 * Hardware: M5Stack Cardputer + Joystick2Unit (I2C 0x63)
 * Created by: Andy + AI
 */

#include <M5Cardputer.h>
#include <Wire.h>
#include <math.h>
#include <SD.h>
#include <SPI.h>

// I2C адрес Joystick2Unit
#define JOYSTICK_ADDR 0x63

// SD Card pins для M5Cardputer
#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

// Регистры Joystick2Unit
#define REG_ADC_X_8   0x10
#define REG_ADC_Y_8   0x11
#define REG_BUTTON    0x20
#define REG_RGB_B     0x30
#define REG_RGB_G     0x31
#define REG_RGB_R     0x32

// 🎨 Формы кисти (5 фигур)
enum BrushShape {
    SHAPE_CIRCLE = 0,   // [1] ⭕ Круг
    SHAPE_SQUARE = 1,   // [2] ⬛ Квадрат
    SHAPE_TRIANGLE = 2, // [3] 🔺 Треугольник
    SHAPE_STAR = 3,     // [4] ⭐ Звезда
    SHAPE_MAPLE = 4     // [5] 🍁 Канадский клен
};

const char* SHAPE_NAMES[] = {"Circle", "Square", "Triangle", "Star", "Maple"};
BrushShape currentShape = SHAPE_CIRCLE;

// Режимы приложения
enum AppMode {
    MODE_STARTUP = 0,
    MODE_DRAW = 1
};
AppMode currentMode = MODE_STARTUP;

// Цветовая палитра
#define COLOR_BG        0x0000
#define COLOR_TEXT      0xFFFF
#define COLOR_RED       0xF800
#define COLOR_YELLOW    0xFFE0
#define COLOR_GREEN     0x07E0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F

// Палитра цветов (8 цветов)
struct LEDColor {
    uint8_t r, g, b;
    const char* name;
    uint16_t displayColor;
};

LEDColor ledPalette[] = {
    {255, 0, 0, "RED", COLOR_RED},
    {0, 255, 0, "GREEN", COLOR_GREEN},
    {0, 0, 255, "BLUE", 0x001F},
    {255, 255, 0, "YELLOW", COLOR_YELLOW},
    {255, 0, 255, "MAGENTA", COLOR_MAGENTA},
    {0, 255, 255, "CYAN", COLOR_CYAN},
    {255, 255, 255, "WHITE", COLOR_TEXT},
    {0, 0, 0, "BLACK", COLOR_BG}
};
int currentColorIndex = 0;
const int paletteSize = 8;

// Переменные джойстика (из теста)
uint8_t joystickX = 127;
uint8_t joystickY = 127;
bool buttonState = false;
bool lastButtonState = false;

// Инверсия осей
bool invertX = false;
bool invertY = false;

// Рисовалка
int brushSize = 3;  // Размер кисти (1-10)
int lastDrawX = -1;
int lastDrawY = -1;
int lastOutlineX = -1;
int lastOutlineY = -1;
bool tubeMode = false;  // Режим "трубы"
bool rainbowMode = false;  // Режим Rainbow - все цвета по очереди
int rainbowColorIndex = 0;  // Индекс текущего цвета в Rainbow
int rainbowLEDSpeed = 1;  // Скорость LED Rainbow: 0=медленно, 1=средне, 2=быстро (по умолчанию средний)
int rainbowLEDIndex = 0;  // Индекс цвета для LED в Rainbow
unsigned long lastRainbowLEDUpdate = 0;  // Время последнего обновления LED Rainbow

// 🎯 Velocity-based cursor system (из теста)
float cursorX = 120.0;  // Плавная позиция курсора (float!)
float cursorY = 67.0;
float cursorSpeed = 300.0;  // Пикселей в секунду
float deadZone = 0.15;  // 15% мёртвая зона
unsigned long lastDrawTime = 0;
bool drawingPaused = false;  // Ctrl/Space: перемещение без рисования

// Буфер для сохранения пикселей под окантовкой
#define MAX_OUTLINE_POINTS 150
struct OutlinePixel {
    int x, y;
    uint16_t color;
};
OutlinePixel savedOutlinePixels[MAX_OUTLINE_POINTS];
int savedOutlineCount = 0;

// Функции
void setupJoystick();
bool saveDrawingToSD();
int getNextFileNumber();
bool readJoystick();
void setJoystickLED(uint8_t r, uint8_t g, uint8_t b);
void handleButtonPress();
void handleKeyboard();
void updateDrawMode();
void drawFilledSquare(int x, int y, int size, uint16_t color);
void drawFilledTriangle(int x, int y, int size, uint16_t color);
void drawFilledStar(int x, int y, int size, uint16_t color);
void drawFilledMaple(int x, int y, int size, uint16_t color);
void drawSmartOutlineShape(int x, int y, int size, BrushShape shape, uint16_t color);
void drawSmartOutlineCircle(int x, int y, int radius, uint16_t color);
void restoreOutline();
uint8_t applyInversion(uint8_t value, bool invert);
void showStartupScreen();
void enterDrawMode();

void setup() {
    Serial.begin(115200);
    
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(COLOR_BG);
    M5Cardputer.Display.setTextSize(1);
    
    Serial.println("\n==================================================");
    Serial.println("🎨 Drawing App by Andy +Ai");
    Serial.println("==================================================");
    
    // Инициализация SD карты
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
        Serial.println("❌ SD card initialization failed!");
    } else {
        Serial.println("✅ SD card initialized!");
    }
    
    setupJoystick();
    showStartupScreen();
    
    Serial.println("Ready!\n");
}

void loop() {
    M5Cardputer.update();
    
    if (!readJoystick()) {
        delay(10);
        return;
    }
    
    // Обработка режимов
    if (currentMode == MODE_STARTUP) {
        // Кнопка джойстика → начать рисовать
        if (buttonState && !lastButtonState) {
            enterDrawMode();
        }
        lastButtonState = buttonState;
        
        handleKeyboard();
        delay(50);
    }
    else if (currentMode == MODE_DRAW) {
        // Проверяем BtnA для очистки экрана
        if (M5Cardputer.BtnA.wasPressed()) {
            M5Cardputer.Display.fillScreen(COLOR_BG);
            lastDrawX = -1;
            lastDrawY = -1;
            lastOutlineX = -1;
            lastOutlineY = -1;
            cursorX = 120.0;
            cursorY = 67.0;
            lastDrawTime = millis();
            Serial.println("Screen cleared by BtnA!");
        }
        
        unsigned long currentMillis = millis();
        static unsigned long lastUpdate = 0;
        const long updateInterval = 33;  // ~30 FPS
        
        if (currentMillis - lastUpdate >= updateInterval) {
            lastUpdate = currentMillis;
            
            // Обрабатываем кнопку джойстика
            if (buttonState && !lastButtonState) {
                handleButtonPress();
            }
            lastButtonState = buttonState;
            
            // Обрабатываем клавиатуру
            handleKeyboard();
            
            // Обновляем рисование
            updateDrawMode();
        }
    }
}

// ========================================
// JOYSTICK FUNCTIONS (из теста)
// ========================================

void setupJoystick() {
    Wire.begin(2, 1, 100000);
    delay(100);
    
    Serial.print("Checking Joystick2 at 0x63... ");
    Wire.beginTransmission(JOYSTICK_ADDR);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("✅ FOUND!");
        setJoystickLED(0, 0, 255);
        delay(200);
    } else {
        Serial.printf("❌ NOT FOUND (error: %d)\n", error);
        M5Cardputer.Display.setTextColor(COLOR_RED, COLOR_BG);
        M5Cardputer.Display.setCursor(10, 60);
        M5Cardputer.Display.print("Joystick2 NOT found!");
        // Не блокируем - пусть работает без джойстика (для тестирования)
    }
}

bool readJoystick() {
    Wire.beginTransmission(JOYSTICK_ADDR);
    Wire.write(REG_ADC_X_8);
    if (Wire.endTransmission(false) != 0) return false;
    
    Wire.requestFrom(JOYSTICK_ADDR, 1);
    if (Wire.available() >= 1) {
        joystickX = 255 - Wire.read();  // Инвертируем X ось (из теста)
    }
    
    Wire.beginTransmission(JOYSTICK_ADDR);
    Wire.write(REG_ADC_Y_8);
    Wire.endTransmission(false);
    
    Wire.requestFrom(JOYSTICK_ADDR, 1);
    if (Wire.available() >= 1) {
        joystickY = 255 - Wire.read();  // Инвертируем Y ось (из теста)
    }
    
    Wire.beginTransmission(JOYSTICK_ADDR);
    Wire.write(REG_BUTTON);
    Wire.endTransmission(false);
    
    Wire.requestFrom(JOYSTICK_ADDR, 1);
    if (Wire.available() >= 1) {
        uint8_t btn = Wire.read();
        buttonState = (btn == 0);
    }
    
    return true;
}

void setJoystickLED(uint8_t r, uint8_t g, uint8_t b) {
    Wire.beginTransmission(JOYSTICK_ADDR);
    Wire.write(REG_RGB_B);
    Wire.write(b);
    Wire.endTransmission();
    
    Wire.beginTransmission(JOYSTICK_ADDR);
    Wire.write(REG_RGB_G);
    Wire.write(g);
    Wire.endTransmission();
    
    Wire.beginTransmission(JOYSTICK_ADDR);
    Wire.write(REG_RGB_R);
    Wire.write(r);
    Wire.endTransmission();
}

uint8_t applyInversion(uint8_t value, bool invert) {
    return invert ? (255 - value) : value;
}

// ========================================
// BUTTON HANDLING
// ========================================

void handleButtonPress() {
    if (rainbowMode) {
        // В режиме Rainbow - переключаем скорость LED
        rainbowLEDSpeed = (rainbowLEDSpeed + 1) % 3;  // 0=медленно, 1=средне, 2=быстро
        const char* speedNames[] = {"SLOW", "MEDIUM", "FAST"};
        Serial.printf("Rainbow LED speed: %s\n", speedNames[rainbowLEDSpeed]);
    } else {
        // Обычный режим - меняем цвет LED
        currentColorIndex = (currentColorIndex + 1) % paletteSize;
        LEDColor& color = ledPalette[currentColorIndex];
        setJoystickLED(color.r, color.g, color.b);
        Serial.printf("Button pressed! Color: %s\n", color.name);
    }
}

// ========================================
// KEYBOARD HANDLING
// ========================================

void handleKeyboard() {
    M5Cardputer.update();
    
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            
            // ESC (backtick) - переход в DRAW режим (из STARTUP)
            if (currentMode == MODE_STARTUP) {
                for (auto i : status.word) {
                    if (i == '`') {  // ESC
                        enterDrawMode();
                        return;
                    }
                }
            }
            // Обработка клавиш в DRAW режиме
            else if (currentMode == MODE_DRAW) {
                for (auto i : status.word) {
                    // Выбор фигуры (1-5)
                    if (i >= '1' && i <= '5') {
                        currentShape = (BrushShape)(i - '1');
                        Serial.printf("Brush shape: %s\n", SHAPE_NAMES[currentShape]);
                    }
                    // Инверсия осей
                    else if (i == 'x' || i == 'X') {
                        invertX = !invertX;
                        Serial.printf("X axis: %s\n", invertX ? "INVERTED" : "NORMAL");
                    }
                    else if (i == 'y' || i == 'Y') {
                        invertY = !invertY;
                        Serial.printf("Y axis: %s\n", invertY ? "INVERTED" : "NORMAL");
                    }
                    // Размер кисти
                    else if (i == '+' || i == '=') {
                        if (brushSize < 10) {
                            brushSize++;
                            Serial.printf("Brush size: %d\n", brushSize);
                        }
                    }
                    else if (i == '-' || i == '_') {
                        if (brushSize > 1) {
                            brushSize--;
                            Serial.printf("Brush size: %d\n", brushSize);
                        }
                    }
                    // Режим "трубы"
                    else if (i == 't' || i == 'T') {
                        tubeMode = !tubeMode;
                        Serial.printf("Tube mode: %s\n", tubeMode ? "ON" : "OFF");
                    }
                    // Режим Rainbow (все цвета по очереди)
                    else if (i == 'r' || i == 'R') {
                        rainbowMode = !rainbowMode;
                        Serial.printf("Rainbow mode: %s\n", rainbowMode ? "ON" : "OFF");
                        if (rainbowMode) {
                            // Включаем Rainbow - сбрасываем LED индекс
                            rainbowLEDIndex = 0;
                            lastRainbowLEDUpdate = 0;
                        } else {
                            // Выключаем Rainbow - возвращаем LED к текущему цвету
                            LEDColor& color = ledPalette[currentColorIndex];
                            setJoystickLED(color.r, color.g, color.b);
                        }
                    }
                    // Сохранение рисунка на SD карту
                    else if (i == 's' || i == 'S') {
                        saveDrawingToSD();
                    }
                }
            }
        }
    }
    
    // Проверяем УДЕРЖАНИЕ Ctrl/Space для паузы рисования (только в DRAW)
    if (currentMode == MODE_DRAW) {
        bool ctrlOrSpaceHeld = false;
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            if (status.ctrl) {
                ctrlOrSpaceHeld = true;
            }
            for (auto i : status.word) {
                if (i == ' ') {  // Space
                    ctrlOrSpaceHeld = true;
                    break;
                }
            }
        }
        
        // Обновляем флаг паузы
        if (ctrlOrSpaceHeld && !drawingPaused) {
            drawingPaused = true;
            lastDrawX = -1;
            lastDrawY = -1;
        } else if (!ctrlOrSpaceHeld && drawingPaused) {
            drawingPaused = false;
            restoreOutline();  // Восстанавливаем пиксели перед выходом из паузы
            lastDrawX = -1;
            lastDrawY = -1;
        }
    }
}

// ========================================
// DRAWING MODE UPDATE (из теста)
// ========================================

void updateDrawMode() {
    // 🎯 Velocity-based cursor: Джойстик → Скорость → Интегрирование позиции
    
    // 1. Время и deltaTime
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - lastDrawTime) / 1000.0;
    if (lastDrawTime == 0) deltaTime = 0.016;
    lastDrawTime = currentTime;
    
    // 2. Нормализуем джойстик в [-1..1] с инверсией
    uint8_t displayX = applyInversion(joystickX, invertX);
    uint8_t displayY = applyInversion(joystickY, invertY);
    
    float nx = (displayX - 127.5) / 127.5;
    float ny = (displayY - 127.5) / 127.5;
    
    // 3. Дедзона
    if (abs(nx) < deadZone) nx = 0;
    if (abs(ny) < deadZone) ny = 0;
    
    // 4. Нормализация диагонали
    float magnitude = sqrt(nx * nx + ny * ny);
    if (magnitude > 1.0) {
        nx /= magnitude;
        ny /= magnitude;
    }
    
    // 5. Вектор скорости
    float vx = nx * cursorSpeed;
    float vy = ny * cursorSpeed;
    
    // 6. Интегрируем позицию
    cursorX += vx * deltaTime;
    cursorY += vy * deltaTime;
    
    // 7. Ограничиваем границами экрана
    cursorX = constrain(cursorX, 0, 239);
    cursorY = constrain(cursorY, 0, 134);
    
    // 8. Конвертируем в целые координаты
    int screenX = (int)cursorX;
    int screenY = (int)cursorY;
    
    // Определяем цвет для рисования (радуга или обычный)
    int colorIndex = rainbowMode ? rainbowColorIndex : currentColorIndex;
    LEDColor& color = ledPalette[colorIndex];
    
    // 🌈 В режиме Rainbow: обновляем LED джойстика с заданной скоростью
    if (rainbowMode) {
        unsigned long intervals[] = {500, 200, 100};  // медленно, средне, быстро (мс)
        unsigned long interval = intervals[rainbowLEDSpeed];
        
        if (currentTime - lastRainbowLEDUpdate >= interval) {
            LEDColor& ledColor = ledPalette[rainbowLEDIndex];
            setJoystickLED(ledColor.r, ledColor.g, ledColor.b);
            rainbowLEDIndex = (rainbowLEDIndex + 1) % paletteSize;
            lastRainbowLEDUpdate = currentTime;
        }
    }
    
    // В режиме радуги меняем цвет при каждом рисовании точки
    // (не на каждом кадре, а только когда действительно рисуем)
    static int lastRainbowX = -1;
    static int lastRainbowY = -1;
    if (rainbowMode && !drawingPaused) {
        if (lastRainbowX != screenX || lastRainbowY != screenY) {
            rainbowColorIndex = (rainbowColorIndex + 1) % paletteSize;
            lastRainbowX = screenX;
            lastRainbowY = screenY;
        }
    } else {
        lastRainbowX = -1;
        lastRainbowY = -1;
    }
    
    // Определяем цвет окантовки
    uint16_t outlineColor = COLOR_TEXT;
    if (colorIndex == 6) outlineColor = 0x7BEF;  // Серая для белого цвета
    
    // 🚫 РЕЖИМ ПАУЗЫ (Ctrl/Space): показываем "умную" окантовку фигуры, НЕ рисуем!
    if (drawingPaused) {
        restoreOutline();
        drawSmartOutlineShape(screenX, screenY, brushSize, currentShape, outlineColor);
        lastOutlineX = screenX;
        lastOutlineY = screenY;
        return;  // Выходим, не рисуем!
    }
    
    if (tubeMode) {
        // === РЕЖИМ "ТРУБЫ" С ОКАНТОВКОЙ ===
        
        // Рисуем линию для "труб" (только для круга, но НЕ в Rainbow режиме)
        if (currentShape == SHAPE_CIRCLE && lastDrawX >= 0 && lastDrawY >= 0 && !rainbowMode) {
            for (int offset = -brushSize; offset <= brushSize; offset++) {
                M5Cardputer.Display.drawLine(
                    lastDrawX + offset, lastDrawY,
                    screenX + offset, screenY,
                    color.displayColor
                );
                M5Cardputer.Display.drawLine(
                    lastDrawX, lastDrawY + offset,
                    screenX, screenY + offset,
                    color.displayColor
                );
            }
        }
        
        // Рисуем фигуру
        if (currentShape == SHAPE_CIRCLE) {
            M5Cardputer.Display.fillCircle(screenX, screenY, brushSize, color.displayColor);
        } else if (currentShape == SHAPE_SQUARE) {
            drawFilledSquare(screenX, screenY, brushSize, color.displayColor);
        } else if (currentShape == SHAPE_TRIANGLE) {
            drawFilledTriangle(screenX, screenY, brushSize, color.displayColor);
        } else if (currentShape == SHAPE_STAR) {
            drawFilledStar(screenX, screenY, brushSize, color.displayColor);
        } else if (currentShape == SHAPE_MAPLE) {
            drawFilledMaple(screenX, screenY, brushSize, color.displayColor);
        }
        
        // Рисуем ОБЫЧНУЮ окантовку фигуры
        if (currentShape == SHAPE_CIRCLE) {
            M5Cardputer.Display.drawCircle(screenX, screenY, brushSize, outlineColor);
        } else if (currentShape == SHAPE_SQUARE) {
            M5Cardputer.Display.drawRect(screenX - brushSize, screenY - brushSize, brushSize * 2, brushSize * 2, outlineColor);
        } else if (currentShape == SHAPE_TRIANGLE) {
            int x1 = screenX, y1 = screenY - brushSize;
            int x2 = screenX - brushSize, y2 = screenY + brushSize;
            int x3 = screenX + brushSize, y3 = screenY + brushSize;
            M5Cardputer.Display.drawTriangle(x1, y1, x2, y2, x3, y3, outlineColor);
        } else if (currentShape == SHAPE_STAR || currentShape == SHAPE_MAPLE) {
            // Для звезды и клена - просто круг окантовка
            M5Cardputer.Display.drawCircle(screenX, screenY, brushSize, outlineColor);
        }
        
        lastDrawX = screenX;
        lastDrawY = screenY;
        
    } else {
        // === ОБЫЧНЫЙ РЕЖИМ (сплошные линии БЕЗ окантовки) ===
        
        // Рисуем линию без пропусков (только для круга, но НЕ в Rainbow режиме)
        if (currentShape == SHAPE_CIRCLE && lastDrawX >= 0 && lastDrawY >= 0 && !rainbowMode) {
            for (int offset = -brushSize; offset <= brushSize; offset++) {
                M5Cardputer.Display.drawLine(
                    lastDrawX + offset, lastDrawY,
                    screenX + offset, screenY,
                    color.displayColor
                );
                M5Cardputer.Display.drawLine(
                    lastDrawX, lastDrawY + offset,
                    screenX, screenY + offset,
                    color.displayColor
                );
            }
        }
        
        // Рисуем фигуру
        if (currentShape == SHAPE_CIRCLE) {
            M5Cardputer.Display.fillCircle(screenX, screenY, brushSize, color.displayColor);
        } else if (currentShape == SHAPE_SQUARE) {
            drawFilledSquare(screenX, screenY, brushSize, color.displayColor);
        } else if (currentShape == SHAPE_TRIANGLE) {
            drawFilledTriangle(screenX, screenY, brushSize, color.displayColor);
        } else if (currentShape == SHAPE_STAR) {
            drawFilledStar(screenX, screenY, brushSize, color.displayColor);
        } else if (currentShape == SHAPE_MAPLE) {
            drawFilledMaple(screenX, screenY, brushSize, color.displayColor);
        }
        
        lastDrawX = screenX;
        lastDrawY = screenY;
    }
}

// ========================================
// SHAPE DRAWING FUNCTIONS (из теста)
// ========================================

void drawFilledSquare(int x, int y, int size, uint16_t color) {
    M5Cardputer.Display.fillRect(x - size, y - size, size * 2, size * 2, color);
}

void drawFilledTriangle(int x, int y, int size, uint16_t color) {
    int x1 = x;
    int y1 = y - size;
    int x2 = x - size;
    int y2 = y + size;
    int x3 = x + size;
    int y3 = y + size;
    M5Cardputer.Display.fillTriangle(x1, y1, x2, y2, x3, y3, color);
}

void drawFilledStar(int x, int y, int size, uint16_t color) {
    float outerRadius = size;
    float innerRadius = size * 0.4;
    
    for (int i = 0; i < 5; i++) {
        float angle1 = i * 72 - 90;
        float angle2 = (i + 0.5) * 72 - 90;
        
        int x1 = x + outerRadius * cos(angle1 * PI / 180.0);
        int y1 = y + outerRadius * sin(angle1 * PI / 180.0);
        int x2 = x + innerRadius * cos(angle2 * PI / 180.0);
        int y2 = y + innerRadius * sin(angle2 * PI / 180.0);
        
        int nextAngle = ((i + 1) % 5) * 72 - 90;
        int x3 = x + outerRadius * cos(nextAngle * PI / 180.0);
        int y3 = y + outerRadius * sin(nextAngle * PI / 180.0);
        
        M5Cardputer.Display.fillTriangle(x, y, x1, y1, x2, y2, color);
        M5Cardputer.Display.fillTriangle(x, y, x2, y2, x3, y3, color);
    }
}

void drawFilledMaple(int x, int y, int size, uint16_t color) {
    // Канадский клен (упрощенная версия)
    // Центр
    M5Cardputer.Display.fillCircle(x, y, size / 2, color);
    // 5 "листьев" (упрощенно как круги)
    for (int i = 0; i < 5; i++) {
        float angle = i * 72 - 90;
        int leafX = x + size * 0.8 * cos(angle * PI / 180.0);
        int leafY = y + size * 0.8 * sin(angle * PI / 180.0);
        M5Cardputer.Display.fillCircle(leafX, leafY, size / 3, color);
    }
}

// ========================================
// SMART OUTLINE (GHOST CURSOR) (из теста)
// ========================================

void drawSmartOutlineCircle(int x, int y, int radius, uint16_t color) {
    savedOutlineCount = 0;
    
    // Для маленьких размеров (1-4) - итерируемся по всем пикселям в квадрате
    if (radius <= 4) {
        // Проверяем все пиксели в квадрате вокруг курсора
        for (int py = y - radius - 1; py <= y + radius + 1 && savedOutlineCount < MAX_OUTLINE_POINTS; py++) {
            for (int px = x - radius - 1; px <= x + radius + 1 && savedOutlineCount < MAX_OUTLINE_POINTS; px++) {
                if (px >= 0 && px < 240 && py >= 0 && py < 135) {
                    // Вычисляем расстояние от центра до пикселя
                    float dist = sqrt((px - x) * (px - x) + (py - y) * (py - y));
                    // Проверяем, находится ли пиксель на окружности (с небольшой погрешностью)
                    if (abs(dist - radius) < 0.8) {
                        savedOutlinePixels[savedOutlineCount].x = px;
                        savedOutlinePixels[savedOutlineCount].y = py;
                        savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px, py);
                        savedOutlineCount++;
                        M5Cardputer.Display.drawPixel(px, py, color);
                    }
                }
            }
        }
    } else {
        // Для больших размеров - обычный шаг по углам
        int angleStep = 10;
        for (int angle = 0; angle < 360 && savedOutlineCount < MAX_OUTLINE_POINTS; angle += angleStep) {
            float rad = angle * PI / 180.0;
            int px = x + radius * cos(rad);
            int py = y + radius * sin(rad);
            
            if (px >= 0 && px < 240 && py >= 0 && py < 135) {
                savedOutlinePixels[savedOutlineCount].x = px;
                savedOutlinePixels[savedOutlineCount].y = py;
                savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px, py);
                savedOutlineCount++;
                M5Cardputer.Display.drawPixel(px, py, color);
            }
        }
    }
}

void drawSmartOutlineShape(int x, int y, int size, BrushShape shape, uint16_t color) {
    savedOutlineCount = 0;
    
    if (shape == SHAPE_CIRCLE) {
        drawSmartOutlineCircle(x, y, size, color);
    }
    else if (shape == SHAPE_SQUARE) {
        int halfSize = size;
        
        // Для маленьких размеров (1-4) - проверяем все пиксели в квадрате
        if (size <= 4) {
            for (int py = y - halfSize - 1; py <= y + halfSize + 1 && savedOutlineCount < MAX_OUTLINE_POINTS; py++) {
                for (int px = x - halfSize - 1; px <= x + halfSize + 1 && savedOutlineCount < MAX_OUTLINE_POINTS; px++) {
                    if (px >= 0 && px < 240 && py >= 0 && py < 135) {
                        // Проверяем, находится ли пиксель на периметре квадрата
                        bool onPerimeter = (px == x - halfSize || px == x + halfSize || 
                                           py == y - halfSize || py == y + halfSize);
                        if (onPerimeter) {
                            savedOutlinePixels[savedOutlineCount].x = px;
                            savedOutlinePixels[savedOutlineCount].y = py;
                            savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px, py);
                            savedOutlineCount++;
                            M5Cardputer.Display.drawPixel(px, py, color);
                        }
                    }
                }
            }
        } else {
            // Для больших размеров - проверяем все пиксели на периметре (шаг 1)
            for (int i = -halfSize; i <= halfSize && savedOutlineCount < MAX_OUTLINE_POINTS; i++) {
                // Верхняя сторона
                int px1 = x + i, py1 = y - halfSize;
                if (px1 >= 0 && px1 < 240 && py1 >= 0 && py1 < 135 && savedOutlineCount < MAX_OUTLINE_POINTS) {
                    savedOutlinePixels[savedOutlineCount].x = px1;
                    savedOutlinePixels[savedOutlineCount].y = py1;
                    savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px1, py1);
                    savedOutlineCount++;
                    M5Cardputer.Display.drawPixel(px1, py1, color);
                }
                // Нижняя сторона
                int px2 = x + i, py2 = y + halfSize;
                if (px2 >= 0 && px2 < 240 && py2 >= 0 && py2 < 135 && savedOutlineCount < MAX_OUTLINE_POINTS) {
                    savedOutlinePixels[savedOutlineCount].x = px2;
                    savedOutlinePixels[savedOutlineCount].y = py2;
                    savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px2, py2);
                    savedOutlineCount++;
                    M5Cardputer.Display.drawPixel(px2, py2, color);
                }
                // Левая сторона (только внутренние точки, углы уже сохранены)
                if (i > -halfSize && i < halfSize) {
                    int px3 = x - halfSize, py3 = y + i;
                    if (px3 >= 0 && px3 < 240 && py3 >= 0 && py3 < 135 && savedOutlineCount < MAX_OUTLINE_POINTS) {
                        savedOutlinePixels[savedOutlineCount].x = px3;
                        savedOutlinePixels[savedOutlineCount].y = py3;
                        savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px3, py3);
                        savedOutlineCount++;
                        M5Cardputer.Display.drawPixel(px3, py3, color);
                    }
                    // Правая сторона
                    int px4 = x + halfSize, py4 = y + i;
                    if (px4 >= 0 && px4 < 240 && py4 >= 0 && py4 < 135 && savedOutlineCount < MAX_OUTLINE_POINTS) {
                        savedOutlinePixels[savedOutlineCount].x = px4;
                        savedOutlinePixels[savedOutlineCount].y = py4;
                        savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px4, py4);
                        savedOutlineCount++;
                        M5Cardputer.Display.drawPixel(px4, py4, color);
                    }
                }
            }
        }
    }
    else if (shape == SHAPE_TRIANGLE) {
        int x1 = x, y1 = y - size;
        int x2 = x - size, y2 = y + size;
        int x3 = x + size, y3 = y + size;
        
        // Для маленьких размеров (1-4) - проверяем все пиксели в треугольнике
        if (size <= 4) {
            // Проверяем все пиксели в квадрате вокруг треугольника
            int minX = min(min(x1, x2), x3) - 1;
            int maxX = max(max(x1, x2), x3) + 1;
            int minY = min(min(y1, y2), y3) - 1;
            int maxY = max(max(y1, y2), y3) + 1;
            
            for (int py = minY; py <= maxY && savedOutlineCount < MAX_OUTLINE_POINTS; py++) {
                for (int px = minX; px <= maxX && savedOutlineCount < MAX_OUTLINE_POINTS; px++) {
                    if (px >= 0 && px < 240 && py >= 0 && py < 135) {
                        // Проверяем, находится ли пиксель на одной из сторон треугольника
                        bool onEdge = false;
                        
                        // Проверяем расстояние до каждой стороны треугольника
                        // Сторона 1: (x1,y1) -> (x2,y2)
                        float dist1 = abs((y2 - y1) * px - (x2 - x1) * py + x2 * y1 - y2 * x1) / 
                                     sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));
                        // Сторона 2: (x2,y2) -> (x3,y3)
                        float dist2 = abs((y3 - y2) * px - (x3 - x2) * py + x3 * y2 - y3 * x2) / 
                                     sqrt((y3 - y2) * (y3 - y2) + (x3 - x2) * (x3 - x2));
                        // Сторона 3: (x3,y3) -> (x1,y1)
                        float dist3 = abs((y1 - y3) * px - (x1 - x3) * py + x1 * y3 - y1 * x3) / 
                                     sqrt((y1 - y3) * (y1 - y3) + (x1 - x3) * (x1 - x3));
                        
                        // Пиксель на стороне, если расстояние < 0.8
                        onEdge = (dist1 < 0.8 || dist2 < 0.8 || dist3 < 0.8);
                        
                        if (onEdge) {
                            savedOutlinePixels[savedOutlineCount].x = px;
                            savedOutlinePixels[savedOutlineCount].y = py;
                            savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px, py);
                            savedOutlineCount++;
                            M5Cardputer.Display.drawPixel(px, py, color);
                        }
                    }
                }
            }
        } else {
            // Для больших размеров - проверяем все пиксели в квадрате вокруг треугольника
            int minX = min(min(x1, x2), x3) - 1;
            int maxX = max(max(x1, x2), x3) + 1;
            int minY = min(min(y1, y2), y3) - 1;
            int maxY = max(max(y1, y2), y3) + 1;
            
            for (int py = minY; py <= maxY && savedOutlineCount < MAX_OUTLINE_POINTS; py++) {
                for (int px = minX; px <= maxX && savedOutlineCount < MAX_OUTLINE_POINTS; px++) {
                    if (px >= 0 && px < 240 && py >= 0 && py < 135) {
                        // Проверяем, находится ли пиксель на одной из сторон треугольника
                        bool onEdge = false;
                        
                        // Проверяем расстояние до каждой стороны треугольника
                        // Сторона 1: (x1,y1) -> (x2,y2)
                        float dist1 = abs((y2 - y1) * px - (x2 - x1) * py + x2 * y1 - y2 * x1) / 
                                     sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));
                        // Сторона 2: (x2,y2) -> (x3,y3)
                        float dist2 = abs((y3 - y2) * px - (x3 - x2) * py + x3 * y2 - y3 * x2) / 
                                     sqrt((y3 - y2) * (y3 - y2) + (x3 - x2) * (x3 - x2));
                        // Сторона 3: (x3,y3) -> (x1,y1)
                        float dist3 = abs((y1 - y3) * px - (x1 - x3) * py + x1 * y3 - y1 * x3) / 
                                     sqrt((y1 - y3) * (y1 - y3) + (x1 - x3) * (x1 - x3));
                        
                        // Пиксель на стороне, если расстояние < 0.8
                        onEdge = (dist1 < 0.8 || dist2 < 0.8 || dist3 < 0.8);
                        
                        if (onEdge) {
                            savedOutlinePixels[savedOutlineCount].x = px;
                            savedOutlinePixels[savedOutlineCount].y = py;
                            savedOutlinePixels[savedOutlineCount].color = M5Cardputer.Display.readPixel(px, py);
                            savedOutlineCount++;
                            M5Cardputer.Display.drawPixel(px, py, color);
                        }
                    }
                }
            }
        }
    }
    else {
        // Для звезды и клена - умная окантовка круга (с адаптивным шагом)
        drawSmartOutlineCircle(x, y, size, color);
    }
}

void restoreOutline() {
    for (int i = 0; i < savedOutlineCount; i++) {
        M5Cardputer.Display.drawPixel(
            savedOutlinePixels[i].x,
            savedOutlinePixels[i].y,
            savedOutlinePixels[i].color
        );
    }
    savedOutlineCount = 0;
}

// ========================================
// STARTUP SCREEN
// ========================================

void showStartupScreen() {
    currentMode = MODE_STARTUP;
    
    // 🎨 Генерируем яркий паттерн (фон)
    uint16_t colors[] = {
        0x06FF,  // Голубой
        0xFFE0,  // Желтый
        0xFBAE,  // Розовый
        0x07FF,  // Бирюзовый
        0xFED7,  // Бежевый
        0x07E0,  // Зеленый
        0x4E1F,  // Синий
        0xFC00,  // Оранжевый
    };
    
    int tileSize = 40;
    int cols = 6;
    int rows = 4;
    
    // Рисуем сетку цветных квадратов
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int x = col * tileSize;
            int y = row * tileSize;
            
            uint16_t bgColor = colors[(row * cols + col) % 8];
            M5Cardputer.Display.fillRect(x, y, tileSize, tileSize, bgColor);
            
            int centerX = x + tileSize / 2;
            int centerY = y + tileSize / 2;
            uint16_t shapeColor = colors[(row * cols + col + 3) % 8];
            
            int shapeType = (row + col) % 4;
            int shapeSize = 4;
            
            if (shapeType == 0) {
                // Квадрат
                M5Cardputer.Display.fillRect(centerX - shapeSize, centerY - shapeSize, 
                                             shapeSize * 2, shapeSize * 2, shapeColor);
            } else if (shapeType == 1) {
                // Ромб
                for (int i = 0; i < shapeSize; i++) {
                    M5Cardputer.Display.drawLine(centerX - i, centerY - shapeSize + i, 
                                                 centerX + i, centerY - shapeSize + i, shapeColor);
                    M5Cardputer.Display.drawLine(centerX - i, centerY + shapeSize - i, 
                                                 centerX + i, centerY + shapeSize - i, shapeColor);
                }
            } else if (shapeType == 2) {
                // Круг
                M5Cardputer.Display.fillCircle(centerX, centerY, shapeSize, shapeColor);
            } else {
                // Крест
                M5Cardputer.Display.fillRect(centerX - shapeSize, centerY - 1, 
                                             shapeSize * 2, 3, shapeColor);
                M5Cardputer.Display.fillRect(centerX - 1, centerY - shapeSize, 
                                             3, shapeSize * 2, shapeColor);
            }
        }
    }
    
    // Полупрозрачная панель для текста (темный прямоугольник)
    M5Cardputer.Display.fillRect(5, 15, 230, 110, 0x0000);
    M5Cardputer.Display.drawRect(5, 15, 230, 110, COLOR_CYAN);
    
    // Старый заголовок: "Drawing App"
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(COLOR_CYAN);
    M5Cardputer.Display.setCursor(20, 25);
    M5Cardputer.Display.println("Drawing App");
    
    // "for M5Stack Cardputer" (первая)
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(COLOR_TEXT);
    M5Cardputer.Display.setCursor(20, 50);
    M5Cardputer.Display.println("for M5Stack Cardputer");
    
    // "Andy +Ai" (вторая)
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(COLOR_CYAN);
    M5Cardputer.Display.setCursor(20, 65);
    M5Cardputer.Display.println("Andy +Ai");
    
    // Инструкции (две отдельные строки)
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(COLOR_YELLOW);
    M5Cardputer.Display.setCursor(20, 85);
    M5Cardputer.Display.println("Btn A - clear screen");
    M5Cardputer.Display.setCursor(20, 100);
    M5Cardputer.Display.println("Btn Joy - change color");
    
    // Внизу: "Нажми джойстик или ESC" (старая строка)
    M5Cardputer.Display.setCursor(30, 110);
    M5Cardputer.Display.print("Press Joy or ESC");
}

void enterDrawMode() {
    currentMode = MODE_DRAW;
    M5Cardputer.Display.fillScreen(COLOR_BG);
    
    cursorX = 120.0;
    cursorY = 67.0;
    lastDrawTime = 0;
    lastDrawX = -1;
    lastDrawY = -1;
    lastOutlineX = -1;
    lastOutlineY = -1;
    
    Serial.println("=== DRAW MODE ===");
}

// ========================================
// SD SAVE FUNCTIONS
// ========================================

// Находит следующий доступный номер для файла draw_XXX.bmp
int getNextFileNumber() {
    int maxNum = 0;
    
    File root = SD.open("/");
    if (!root) {
        return 1;  // SD не доступна, начинаем с 1
    }
    
    File file = root.openNextFile();
    while (file) {
        String name = file.name();
        if (name.startsWith("draw_") && name.endsWith(".bmp")) {
            // Извлекаем номер (draw_001.bmp → 001)
            int numStart = 5;  // После "draw_"
            int numEnd = name.indexOf(".bmp");
            String numStr = name.substring(numStart, numEnd);
            int num = numStr.toInt();
            if (num > maxNum) maxNum = num;
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();
    
    return maxNum + 1;
}

// Конвертирует RGB565 в RGB (B, G, R)
void rgb565ToRgb(uint16_t rgb565, uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = ((rgb565 >> 11) & 0x1F) << 3;  // 5 бит → 8 бит
    *g = ((rgb565 >> 5) & 0x3F) << 2;   // 6 бит → 8 бит
    *b = (rgb565 & 0x1F) << 3;          // 5 бит → 8 бит
}

// Сохраняет текущий экран в BMP формат на SD карту
bool saveDrawingToSD() {
    if (currentMode != MODE_DRAW) {
        return false;
    }
    
    Serial.println("\n💾 === SAVE DRAWING ===");
    
    // 1) Находим следующий номер файла
    int num = getNextFileNumber();
    char filename[64];
    snprintf(filename, sizeof(filename), "/draw_%03d.bmp", num);
    
    Serial.printf("💾 Saving: %s\n", filename);
    
    // 2) Создаём файл
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("❌ Failed to create file!");
        return false;
    }
    
    // 3) BMP Header (54 байта)
    const int WIDTH = 240;
    const int HEIGHT = 135;
    const int PADDING = (4 - (WIDTH * 3) % 4) % 4;  // BMP требует выравнивание по 4 байта
    const int ROW_SIZE = WIDTH * 3 + PADDING;
    const int IMAGE_SIZE = ROW_SIZE * HEIGHT;
    const int FILE_SIZE = 54 + IMAGE_SIZE;
    
    // BMP File Header (14 bytes) + DIB Header (40 bytes)
    uint8_t bmpHeader[54] = {
        // Signature "BM"
        0x42, 0x4D,
        // File size (4 bytes, little-endian)
        (uint8_t)(FILE_SIZE), (uint8_t)(FILE_SIZE >> 8), (uint8_t)(FILE_SIZE >> 16), (uint8_t)(FILE_SIZE >> 24),
        // Reserved
        0, 0, 0, 0,
        // Data offset (54)
        54, 0, 0, 0,
        // DIB Header size (40)
        40, 0, 0, 0,
        // Width (4 bytes)
        (uint8_t)(WIDTH), (uint8_t)(WIDTH >> 8), (uint8_t)(WIDTH >> 16), (uint8_t)(WIDTH >> 24),
        // Height (4 bytes) - NEGATIVE для top-down
        (uint8_t)(-HEIGHT), (uint8_t)(-HEIGHT >> 8), (uint8_t)(-HEIGHT >> 16), (uint8_t)(-HEIGHT >> 24),
        // Planes (always 1)
        1, 0,
        // Bits per pixel (24)
        24, 0,
        // Compression (0 = none)
        0, 0, 0, 0,
        // Image size
        (uint8_t)(IMAGE_SIZE), (uint8_t)(IMAGE_SIZE >> 8), (uint8_t)(IMAGE_SIZE >> 16), (uint8_t)(IMAGE_SIZE >> 24),
        // X pixels per meter (2835)
        0x13, 0x0B, 0, 0,
        // Y pixels per meter (2835)
        0x13, 0x0B, 0, 0,
        // Colors in palette (0 = default)
        0, 0, 0, 0,
        // Important colors (0 = all)
        0, 0, 0, 0
    };
    
    file.write(bmpHeader, 54);
    
    // 4) Записываем пиксели (BMP хранит снизу вверх, но мы используем negative height = top-down)
    uint8_t rowBuffer[ROW_SIZE];
    
    for (int y = 0; y < HEIGHT; y++) {  // От верхней строки к нижней (top-down)
        int bufPos = 0;
        
        for (int x = 0; x < WIDTH; x++) {
            // Читаем пиксель с экрана (RGB565)
            uint16_t pixel = M5Cardputer.Display.readPixel(x, y);
            
            // Конвертируем RGB565 → RGB (BGR для BMP!)
            uint8_t r, g, b;
            rgb565ToRgb(pixel, &r, &g, &b);
            
            // BMP хранит в порядке BGR!
            rowBuffer[bufPos++] = b;  // B
            rowBuffer[bufPos++] = g;  // G
            rowBuffer[bufPos++] = r;  // R
        }
        
        // Padding
        for (int p = 0; p < PADDING; p++) {
            rowBuffer[bufPos++] = 0;
        }
        
        file.write(rowBuffer, ROW_SIZE);
    }
    
    file.close();
    
    Serial.printf("✅ Drawing saved: %s (%d KB)\n", filename, FILE_SIZE / 1024);
    
    // 5) Показываем "Saved!" на экране
    // Сохраняем область под текстом
    const int msgX = 80;
    const int msgY = 60;
    const int msgW = 80;
    const int msgH = 20;
    uint16_t savedPixels[msgW * msgH];
    int pixelIdx = 0;
    for (int py = msgY; py < msgY + msgH; py++) {
        for (int px = msgX; px < msgX + msgW; px++) {
            if (px < 240 && py < 135) {
                savedPixels[pixelIdx++] = M5Cardputer.Display.readPixel(px, py);
            }
        }
    }
    
    // Рисуем "Saved!"
    M5Cardputer.Display.fillRect(msgX, msgY, msgW, msgH, COLOR_BG);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(COLOR_GREEN, COLOR_BG);
    M5Cardputer.Display.setCursor(msgX + 5, msgY + 2);
    M5Cardputer.Display.println("Saved!");
    
    // Показываем сообщение 1.5 секунды (non-blocking для основной логики)
    unsigned long startTime = millis();
    while (millis() - startTime < 1500) {
        M5Cardputer.update();
        delay(10);
    }
    
    // Восстанавливаем сохранённые пиксели
    pixelIdx = 0;
    for (int py = msgY; py < msgY + msgH; py++) {
        for (int px = msgX; px < msgX + msgW; px++) {
            if (px < 240 && py < 135 && pixelIdx < msgW * msgH) {
                M5Cardputer.Display.drawPixel(px, py, savedPixels[pixelIdx++]);
            }
        }
    }
    
    return true;
}