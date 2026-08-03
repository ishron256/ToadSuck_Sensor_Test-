#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_BME280.h>

// ESP32 <-> BME280 SPI wiring
constexpr uint8_t PIN_BME_CS = 10;
constexpr uint8_t PIN_BME_MOSI = 11;
constexpr uint8_t PIN_BME_SCK = 12;
constexpr uint8_t PIN_BME_MISO = 13;

// Conservative bus rate for initial bring-up; can be increased after validation.
constexpr uint32_t BME_SPI_HZ = 1000000;
constexpr uint32_t READ_INTERVAL_MS = 2000;

Adafruit_BME280 bme(PIN_BME_CS, &SPI);

unsigned long lastReadMs = 0;

void printReadings() {
  SPI.beginTransaction(SPISettings(BME_SPI_HZ, MSBFIRST, SPI_MODE0));
  const float temperatureC = bme.readTemperature();
  const float humidityPct = bme.readHumidity();
  const float pressureHpa = bme.readPressure() / 100.0F;
  SPI.endTransaction();

  if (isnan(temperatureC) || isnan(humidityPct) || isnan(pressureHpa)) {
    Serial.println("Read failed (NaN). Check wiring and sensor power.");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperatureC, 2);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidityPct, 2);
  Serial.println(" %");

  Serial.print("Pressure: ");
  Serial.print(pressureHpa, 2);
  Serial.println(" hPa");

  Serial.println("---");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("BME280 SPI validation test (ESP32)");
  Serial.println("CPU set to 80 MHz via platformio.ini");

  SPI.begin(PIN_BME_SCK, PIN_BME_MISO, PIN_BME_MOSI, PIN_BME_CS);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(BME_SPI_HZ);

  const bool ok = bme.begin();
  if (!ok) {
    Serial.println("BME280 not found. Verify CS/MOSI/MISO/SCK and 3.3V/GND.");
    while (true) {
      delay(1000);
    }
  }

  const uint8_t id = bme.sensorID();
  Serial.print("BME280 detected, chip ID: 0x");
  Serial.println(id, HEX);

  // Fixed sampling settings to make test output stable and repeatable.
  bme.setSampling(
      Adafruit_BME280::MODE_NORMAL,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::SAMPLING_X1,
      Adafruit_BME280::FILTER_OFF,
      Adafruit_BME280::STANDBY_MS_1000);
}

void loop() {
  const unsigned long now = millis();
  if (now - lastReadMs >= READ_INTERVAL_MS) {
    lastReadMs = now;
    printReadings();
  }
}
