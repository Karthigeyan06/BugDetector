// Multi-Protocol Bug Detector
// ESP32 + NRF24L01 + 0.96" OLED

#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <RF24.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NRF24L01 Pins
#define CE_PIN   17
#define CSN_PIN  5
RF24 radio(CE_PIN, CSN_PIN);

// BLE
BLEScan* pBLEScan;

void setup() {
  Serial.begin(115200);
  
  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Bug Detector Init");
  display.display();

  // WiFi Init
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // BLE Init
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);

  // NRF24L01 Init
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(100); // Channel 2.4 GHz range (100 = 2.5 GHz)
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setRetries(0, 0);
  radio.openReadingPipe(1, 0xF0F0F0F0E1LL);
  radio.startListening();
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0);

  // --- WiFi Scan ---
  int n = WiFi.scanNetworks();
  display.println("WiFi:");
  for (int i = 0; i < n && i < 3; ++i) {
    display.print(WiFi.SSID(i));
    display.print(" (");
    display.print(WiFi.RSSI(i));
    display.println(" dBm)");
  }

  // --- BLE Scan ---
  display.println("\nBLE:");
  BLEScanResults foundDevices = pBLEScan->start(3, false);
  for (int i = 0; i < foundDevices.getCount() && i < 3; i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    display.print(device.getName().c_str());
    display.print(" (");
    display.print(device.getRSSI());
    display.println(" dBm)");
  }

  // --- RF Noise Detection ---
  display.println("\nRF:");
  if (radio.testRPD() > 0) {
    display.setTextColor(SSD1306_WHITE);
    display.println("Unknown 2.4GHz Signal!");
  } else {
    display.println("No RF Activity");
  }

  display.display();
  delay(4000);
}
