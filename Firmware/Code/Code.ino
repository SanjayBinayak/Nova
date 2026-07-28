// importing packedges
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "esp_camera.h"
#include "mbedtls/base64.h"

// wifi connection details to be filled
const char* wifi_ssid = "name of wifi";
const char* wifi_password = "password of wifi";
// api key
const char* api_key = "ai api key";

//I am using gemini as ai and it's 2.5-flash model, you can put your own
const char* ai_model = "gemini-2.5-flash";

//pin configuration

#define CAM_PIN_PWDN    -1 //NOT BROKEN OUT IN BOARD I AM USING
#define CAM_PIN_RESET   -1 //NOT BROKEN OUT IN BOARD I AM USING
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5
#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

#define SHARED_SCK      41
#define SHARED_MOSI     42
#define SHARED_MISO     47
#define TFT_CS_PIN      21
#define TOUCH_CS_PIN    1
#define TOUCH_IRQ_PIN 255

//GLOBALS
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen touch(TOUCH_CS_PIN, TOUCH_IRQ_PIN);

//RUN A SIMPLE TOUCH-TEST FIRST TO FILL THESE VALUES
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 450
#define TS_MAXY 4000

String questionText = "";
bool cameraModeOn = false;
bool cameraReady = false;

// on-screen keyboard layout
const char* KB_ROW1 = "1234567890";
const char* KB_ROW2 = "QWERTYUIOP";
const char* KB_ROW3 = "ASDFGHJKL";
const char* KB_ROW4 = "ZXCVBNM";

int screenW, screenH;
int kbTop;      //y-coordinate of keyboard start
int keyW, keyH; //size of each key

void setup() {
    Serial.begin(115200);
    delay(300);
    // Start the shared SPI bus on our chosen pins
    SPI.begin(SHARED_SCK, SHARED_MISO, SHARED_MOSI, TFT_CS_PIN);

    tft.init();
    tft.setRotation(1);
    screenW = tft.width();
    screenH = tft.height();
    tft.fillScreen(TFT_BLACK);

    touch.begin(SPI);
    touch.setRotation(1);

    showStatus("Connecting to WiFi...");
    connectWiFi();

    showStatus("Starting camera...");
    cameraReady = initCamera();
    if (!cameraReady) {
        showStatus("Camera init failed");
        delay(1500);
    }

    drawMainScreen();

}

void loop() {
    if (touch.touched()) {
        TS_Point p = touch.getPoint();
        int x = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
        int y = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
        handleTouch(x, y);
        delay(180);
    }
}

//wifi connecting
void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 40) {
        delay(250);
        tries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        showStatus("WIfi connected");
    } else {
        showStatus("WIfi connection failed");
    }
    delay(800);

}

bool initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = CAM_PIN_D0;
    config.pin_d1 = CAM_PIN_D1;
    config.pin_d2 = CAM_PIN_D2;
    config.pin_d3 = CAM_PIN_D3;
    config.pin_d4 = CAM_PIN_D4;
    config.pin_d5 = CAM_PIN_D5;
    config.pin_d6 = CAM_PIN_D6;
    config.pin_d7 = CAM_PIN_D7;
    config.pin_xclk = CAM_PIN_XCLK;
    config.pin_pclk = CAM_PIN_PCLK;
    config.pin_vsync = CAM_PIN_VSYNC;
    config.pin_href = CAM_PIN_HREF;
    config.pin_sscb_sda = CAM_PIN_SIOD;
    config.pin_sscb_scl = CAM_PIN_SIOC;
    config.pin_pwdn = CAM_PIN_PWDN;
    config.pin_reset = CAM_PIN_RESET;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;
        config.fb_count = 2;
        config.fb_location = CAMERA_FB_IN_PSRAM;
    } else {
        config.frame_size = FRAMESIZE_QQVGA;
        config.jpeg_quality = 15;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error %d\n", err);
        return false;
    }
    return true;
}

String captureImageBase64() {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return "";
    }

    size_t outLen = 0;
    mbedtls_base64_encode(NULL, 0, &outLen, fb->buf, fb->len);
    unsigned char* outBuf = (unsigned char*)malloc(outLen + 1);
    if (!outBuf) {
        esp_camera_fb_return(fb);
        return "";
    }
    size_t written = 0;
    mbedtls_base64_encode(outBuf, outLen, &written, fb->buf, fb->len);
    outBuf[written] = '\0';

    String result = String((char*)outBuf);
    free(outBuf);
    esp_camera_fb_return(fb);
    return result;
}
//Ai call ( I am using gemini AI API in my case)
String askGemini(String question, bool withImage) {
  if (WiFi.status() != WL_CONNECTED) {
    return "(No WiFi connection)";
  }

  String imageB64 = "";
  if (withImage && cameraReady) {
    imageB64 = captureImageBase64();
    if (imageB64 == "") {
      return "(Camera capture failed - try again)";
    }
  }

    DynamicJsonDocument doc(200000);
  JsonArray contents = doc.createNestedArray("contents");
  JsonObject content0 = contents.createNestedObject();
  JsonArray parts = content0.createNestedArray("parts");

  JsonObject textPart = parts.createNestedObject();
  textPart["text"] = question;

  if (imageB64 != "") {
    JsonObject imgPart = parts.createNestedObject();
    JsonObject inlineData = imgPart.createNestedObject("inline_data");
    inlineData["mime_type"] = "image/jpeg";
    inlineData["data"] = imageB64;
  }

  String requestBody;
  serializeJson(doc, requestBody);

  WiFiClientSecure client;
  client.setInsecure();

    HTTPClient https;
  String url = String("https://generativelanguage.googleapis.com/v1beta/models/")
               + ai_model + ":generateContent?key=" + api_key;

  String answer = "(No response)";
  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST(requestBody);

    if (httpCode == 200) {
      String response = https.getString();
      DynamicJsonDocument respDoc(100000);
      DeserializationError err = deserializeJson(respDoc, response);
      if (!err) {
        answer = respDoc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
      } else {
        answer = "(Failed to parse response)";
      }
    } else {
      answer = "(HTTP error " + String(httpCode) + ")";
      Serial.println(https.getString());
    }
    https.end();
  } else {
    answer = "(Could not connect to Gemini)";
  }

  return answer;
}

// UI display
void showStatus(String msg) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, screenH / 2 - 10);
  tft.print(msg);
}

void drawMainScreen() {
  tft.fillScreen(TFT_BLACK);

  // Text input box at top
  tft.drawRect(5, 5, screenW - 10, 30, TFT_WHITE);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 15);
  tft.print(questionText);

  // Camera toggle button
  drawButton(screenW - 70, 40, 60, 25, cameraModeOn ? "CAM:ON" : "CAM:OFF",
             cameraModeOn ? TFT_GREEN : TFT_RED);

  // Send button
  drawButton(5, 40, 60, 25, "SEND", TFT_BLUE);

  // Clear button
  drawButton(70, 40, 60, 25, "CLR", TFT_ORANGE);

  drawKeyboard();
}

void drawButton(int x, int y, int w, int h, const char* label, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, 4, color);
  tft.drawRoundRect(x, y, w, h, 4, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, color);
  tft.setTextSize(1);
  tft.setCursor(x + 6, y + h / 2 - 4);
  tft.print(label);
}

void drawKeyboard() {
  kbTop = 75;
  keyW = screenW / 10;
  keyH = (screenH - kbTop) / 4;

  drawKeyRow(KB_ROW1, kbTop, 0);
  drawKeyRow(KB_ROW2, kbTop + keyH, 0);
  drawKeyRow(KB_ROW3, kbTop + keyH * 2, keyW / 2);
  drawKeyRow(KB_ROW4, kbTop + keyH * 3, keyW / 2);

  // Space bar + backspace on the last row's remaining space
  int spaceX = keyW / 2 + strlen(KB_ROW4) * keyW;
  tft.fillRoundRect(spaceX, kbTop + keyH * 3, screenW - spaceX - keyW, keyH - 2, 3, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setCursor(spaceX + 10, kbTop + keyH * 3 + keyH / 2 - 4);
  tft.print("SPACE");

  tft.fillRoundRect(screenW - keyW, kbTop + keyH * 3, keyW - 2, keyH - 2, 3, TFT_MAROON);
  tft.setTextColor(TFT_WHITE, TFT_MAROON);
  tft.setCursor(screenW - keyW + 4, kbTop + keyH * 3 + keyH / 2 - 4);
  tft.print("DEL");
}

void drawKeyRow(const char* row, int y, int xOffset) {
  int len = strlen(row);
  for (int i = 0; i < len; i++) {
    int x = xOffset + i * keyW;
    tft.fillRoundRect(x, y, keyW - 2, keyH - 2, 3, TFT_NAVY);
    tft.drawRoundRect(x, y, keyW - 2, keyH - 2, 3, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setCursor(x + keyW / 2 - 4, y + keyH / 2 - 4);
    tft.print(row[i]);
  }
}

//handeling touch input
void handleTouch(int x, int y) {
  // Top control buttons
  if (y >= 40 && y <= 65) {
    if (x >= 5 && x <= 65) {
      sendQuestion();
      return;
    }
    if (x >= 70 && x <= 130) {
      questionText = "";
      drawMainScreen();
      return;
    }
    if (x >= screenW - 70 && x <= screenW) {
      cameraModeOn = !cameraModeOn;
      drawMainScreen();
      return;
    }
  }

  
  if (y >= kbTop) {
    char c = getKeyAt(x, y);
    if (c == '\b') {
      if (questionText.length() > 0) questionText.remove(questionText.length() - 1);
    } else if (c == ' ') {
      questionText += ' ';
    } else if (c != 0) {
      questionText += c;
    }
    
    tft.fillRect(6, 6, screenW - 12, 28, TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(10, 15);
    tft.print(questionText);
  }
}

char getKeyAt(int x, int y) {
  int row = (y - kbTop) / keyH;
  if (row == 0) return charInRow(KB_ROW1, x, 0);
  if (row == 1) return charInRow(KB_ROW2, x, 0);
  if (row == 2) return charInRow(KB_ROW3, x, keyW / 2);
  if (row == 3) {
    int spaceX = keyW / 2 + strlen(KB_ROW4) * keyW;
    if (x >= spaceX && x < screenW - keyW) return ' ';
    if (x >= screenW - keyW) return '\b';
    return charInRow(KB_ROW4, x, keyW / 2);
  }
  return 0;
}

char charInRow(const char* row, int x, int xOffset) {
  int idx = (x - xOffset) / keyW;
  int len = strlen(row);
  if (idx >= 0 && idx < len) return row[idx];
  return 0;
}


// send flow
void sendQuestion() {
  if (questionText.length() == 0) return;

  showStatus("Asking Gemini...");
  String answer = askGemini(questionText, cameraModeOn);
  displayAnswer(answer);
}

void displayAnswer(String text) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.setCursor(5, 5);
  tft.print(text);

  
  drawButton(screenW - 70, screenH - 30, 65, 25, "BACK", TFT_BLUE);

  
  while (true) {
    if (touch.touched()) {
      TS_Point p = touch.getPoint();
      int x = map(p.x, TS_MINX, TS_MAXX, 0, screenW);
      int y = map(p.y, TS_MINY, TS_MAXY, 0, screenH);
      if (x >= screenW - 70 && y >= screenH - 30) {
        questionText = "";
        drawMainScreen();
        break;
      }
      delay(200);
    }
    delay(20);
  }
}
