#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

// ---------------- WIFI ----------------
const char* ssid = "MAYANK's M06";
const char* password = "6290744728";

// ---------------- WEATHER API ----------------
String city = "Kolkata";
String apiKey = "d1fa16b2c8ebcb3733f22f7ae09cf0d3";

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_DC    2
#define OLED_RES   4
#define OLED_CS   -1
#define OLED_SCK  18
#define OLED_MOSI 23

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RES, OLED_CS);

// ---------------- PINS ----------------
#define TOUCH_PIN 15
#define BUZZER_PIN 25

// ---------------- VARIABLES ----------------
String weatherText = "Loading...";
String currentTime = "--:--";
unsigned long lastWeatherUpdate = 0;
unsigned long lastTimeUpdate = 0;
int faceMode = 0;

// ---------------- FACE FUNCTIONS ----------------
void drawHappyFace() {
  display.clearDisplay();

  display.fillCircle(35, 25, 8, SSD1306_WHITE);
  display.fillCircle(93, 25, 8, SSD1306_WHITE);

  display.fillCircle(35, 25, 3, SSD1306_BLACK);
  display.fillCircle(93, 25, 3, SSD1306_BLACK);

  display.drawRoundRect(40, 42, 48, 10, 5, SSD1306_WHITE);
  display.display();
}

void drawBlinkFace() {
  display.clearDisplay();

  display.drawLine(28, 25, 42, 25, SSD1306_WHITE);
  display.drawLine(86, 25, 100, 25, SSD1306_WHITE);

  display.drawRoundRect(40, 42, 48, 10, 5, SSD1306_WHITE);
  display.display();
}

void drawTouchedFace() {
  display.clearDisplay();

  display.fillCircle(35, 25, 10, SSD1306_WHITE);
  display.fillCircle(93, 25, 10, SSD1306_WHITE);

  display.fillCircle(35, 25, 2, SSD1306_BLACK);
  display.fillCircle(93, 25, 2, SSD1306_BLACK);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 50);
  display.println("Hello Human!");
  display.display();
}

// ---------------- WEATHER ----------------
void getWeather() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();

      DynamicJsonDocument doc(2048);
      deserializeJson(doc, payload);

      float temp = doc["main"]["temp"];
      String condition = doc["weather"][0]["main"].as<String>();

      weatherText = String(temp, 1) + "C " + condition;
    }
    else {
      weatherText = "Weather Error";
    }

    http.end();
  }
}

// ---------------- TIME ----------------
void updateTime() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    currentTime = "No Time";
    return;
  }

  char timeString[10];
  strftime(timeString, sizeof(timeString), "%H:%M", &timeinfo);
  currentTime = String(timeString);
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  SPI.begin(OLED_SCK, -1, OLED_MOSI, -1);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED Failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Starting Desk Bot...");
  display.display();

  WiFi.begin(ssid, password);

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Connecting WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("WiFi Connected!");
  display.display();

  delay(1500);

  configTime(19800, 0, "pool.ntp.org");

  getWeather();
  updateTime();
}

// ---------------- LOOP ----------------
void loop() {
  if (millis() - lastWeatherUpdate > 600000) {
    getWeather();
    lastWeatherUpdate = millis();
  }

  if (millis() - lastTimeUpdate > 1000) {
    updateTime();
    lastTimeUpdate = millis();
  }

  if (digitalRead(TOUCH_PIN) == HIGH) {
    drawTouchedFace();
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(1000);
  }
  else {
    if (faceMode == 0) {
      drawHappyFace();
      faceMode = 1;
    }
    else {
      drawBlinkFace();
      faceMode = 0;
    }

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Time: ");
    display.println(currentTime);

    display.setCursor(0, 54);
    display.print(weatherText);

    display.display();

    delay(1200);
  }
}