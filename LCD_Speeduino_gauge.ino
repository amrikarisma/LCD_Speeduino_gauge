#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <SPI.h>
#include <SD.h>

MCUFRIEND_kbv tft;
#define SD_CS 10  // Pin chip select untuk SD card
#define TFT_WIDTH 320
#define TFT_HEIGHT 480

File file;
char filename[12];
byte serialData[40];
uint16_t parsedData[40];
unsigned long serialMillis = 0;
int gaugeMap[6] = { 5, 6, 16, 15, 20, 4 };
int gaugePosMap[6][2] = { {10, 10}, {10, 100}, {10, 190}, {160, 10}, {160, 100}, {160, 190} };
int gaugeCurr = 0;

const char *gaugeNames[] = {"SecL", "Squirt", "RPM", "CLT", "MAP", "IAT"};
unsigned long LogUpdateMillis = 0;
#define LOG_UPDATE 50

void setup() {
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(1);
  tft.fillScreen(0x0000);  // Black background
  
  Serial.begin(115200);

  // if (!SD.begin(SD_CS)) {
  //   tft.setCursor(10, 10);
  //   tft.setTextColor(0xF800); // Red
  //   tft.setTextSize(2);
  //   tft.println("SD Card Error!");
  //   while (1);
  // }
  // createNewLogFile();
}

void loop() {
  updateGauge();
}

void createNewLogFile() {
  int fnum = 0;
  while (true) {
    sprintf(filename, "LOG%03d.CSV", fnum);
    if (!SD.exists(filename)) break;
    fnum++;
  }
  file = SD.open(filename, FILE_WRITE);
  if (file) {
    file.println("Time,SecL,Squirt,RPM,CLT,MAP,IAT");
    file.flush();
  } else {
    tft.setCursor(10, 10);
    tft.setTextColor(0xF800); // Red
    tft.setTextSize(2);
    tft.println("File Error!");
    while (1);
  }
}

void updateGauge() {
  if (millis() - LogUpdateMillis >= LOG_UPDATE) {
    LogUpdateMillis = millis();
    resetData();
    Serial.print("n"); // Request data
    serialMillis = millis();
    while (Serial.available() == 0) {
      if (millis() - serialMillis > 100) return;
    }

    int i = 0;
    while (Serial.available() > 0) {
      serialData[i] = Serial.read();
      i++;
      if (i >= 40) break;
    }
    parseData();
    displayGauge();
    logData();
  }
}

void parseData() {
  for (int i = 0; i < 40; i++) {
    parsedData[i] = serialData[i];  // Replace with actual parsing logic
  }
}

void displayGauge() {
  tft.fillScreen(0x0000);  // Clear screen
  tft.setTextSize(2);
  for (int i = 0; i < 6; i++) {
    tft.setCursor(gaugePosMap[i][0], gaugePosMap[i][1]);
    tft.setTextColor(0x07E0);  // Green
    tft.print(gaugeNames[gaugeMap[i]]);
    tft.setCursor(gaugePosMap[i][0], gaugePosMap[i][1] + 20);
    tft.setTextColor(0xFFFF);  // White
    tft.print(parsedData[gaugeMap[i]]);
  }
}

void logData() {
  if (file) {
    file.print(millis() / 1000.0);
    for (int i = 0; i < 6; i++) {
      file.print(",");
      file.print(parsedData[gaugeMap[i]]);
    }
    file.println();
    file.flush();
  }
}

void resetData() {
  for (int i = 0; i < 40; i++) {
    serialData[i] = 0;
    parsedData[i] = 0;
  }
}