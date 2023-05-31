/* Project     : Wetterstation
   Autors      : Khaldoun Kalash
   Last Update : 29.01.2023
   Version     : 1.0
*/
#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Importiert alle benötigte Header-Dateien durch eine Zeile
#include "all_headers.h"

#define DHT_TYPE DHT11  //DHT-Sensor DHT11
#define DHT_PIN D5      // D5 = Datenpin des DHT-Sensors am ESP8266
#define DHT_POWER D0    // Spannungversorgung für DHT-Sensor

#define SCREEN_WIDTH 128  // Maximale Breite des Monitores in Pixels
#define SCREEN_HEIGHT 64  // Maximale Höhe des Monitores in Pixels
#define OLED_RESET 0      // Reset pin

const int IMAGE_SIZE = 30;  // Bildgrosse
const int IMAGE_POS = 10;   // Bild-Position

const int FEH_TASTER = D3;       // Flash Button
const long INTERVAL = 2000;      // 2000=2Sec; Interval der Datenerfassung
const int HEILIGKEITSENSOR = 0;  // PInN ADC-Anschluss

DHT dht(DHT_PIN, DHT_TYPE);  // Erstellt DHT sensor
// Erstellt display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  pinMode(FEH_TASTER, INPUT_PULLUP);
  pinMode(DHT_POWER, OUTPUT);  // Spannungsversorgung für DHT-Baustein
  digitalWrite(DHT_POWER, HIGH);
  dht.begin();  // Initialisierung der DHT sensor

  // mit I2C-Adresse 0x3c initialisieren
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.setTextColor(WHITE);  // Setzt die Farbe des Texts in Monitor auf Weiß

  timeInit();  // Initialisiert die Zeit
}

void loop() {
  // liest die aktuelle Temperatur in Celsius aus dem DHT11 Sensor
  float celsius = dht.readTemperature();
  // liest die aktuelle Luftfeuchtigkeit aus dem DHT11 Sensor
  float feuchtigkeit = dht.readHumidity();
  //liest die aktuelle Helligkeit aus dem Helligkeit-Sensor
  int helligkeit = analogRead(HEILIGKEITSENSOR);

  display.clearDisplay();  // Löscht den Inhalt auf dem Monitor

  getTime();  // Setzt die Zeit auf jetzt

  drawGrafic(celsius, feuchtigkeit, helligkeit);
  drawTemperatur(celsius);
  drawHelligkeit(helligkeit);
  drawFeuchtigkeit(feuchtigkeit);
  drawDateTime();

  display.display();
  delay(INTERVAL);
}

/*
* Stellt das entsprechene Grafic in Bezug auf die aktuelle Temperatur,
* Feuchtigkeit und Hellihkeit dar.
*/
void drawGrafic(float temperatur, float feuchtigkeit, int helligkeit) {
  if (temperatur < 4) {
    // Stellt das Grafik auf dem Monitor dar
    display.drawXBitmap(IMAGE_POS, IMAGE_POS, schnee, IMAGE_SIZE, IMAGE_SIZE, WHITE);
  } else if (temperatur >= 30) {
    display.drawXBitmap(IMAGE_POS, IMAGE_POS, hitze, IMAGE_SIZE, IMAGE_SIZE, WHITE);
  } else {
    if (feuchtigkeit > 80) {
      display.drawXBitmap(IMAGE_POS, IMAGE_POS, tropfen, IMAGE_SIZE, IMAGE_SIZE, WHITE);
    } else if (helligkeit >= 400) {
      display.drawXBitmap(IMAGE_POS, IMAGE_POS, sonnig, IMAGE_SIZE, IMAGE_SIZE, WHITE);
    } else if (helligkeit >= 180) {
      display.drawXBitmap(IMAGE_POS, IMAGE_POS, teil_sonnig, IMAGE_SIZE, IMAGE_SIZE, WHITE);
    } else if (hour() > 19) {
      display.drawXBitmap(IMAGE_POS, IMAGE_POS, mond_wolkig, IMAGE_SIZE, IMAGE_SIZE, WHITE);
    } else {
      display.drawXBitmap(IMAGE_POS, IMAGE_POS, wolkig, IMAGE_SIZE, IMAGE_SIZE, WHITE);
    }
  }
}

/*
* Convertiert den aktuellen Wert der Temperatur zu einer String
* ohne Nachkommazahl.
* Wenn Flash_btn gedrückt wird, wird den der Temperatur in
* Fehrenheit auf dem Monitor dargestellt. Wenn das Button
* los gelassen wird, wird die Temperatur in Celsius dargestellt.
*/
void drawTemperatur(float celsius) {
  float fahrenheit = dht.readTemperature(true);
  String celsiusAsString = String(int(celsius + 0.5));
  String fahrenheitAsString = String(int(fahrenheit + 0.5));

  // Positioniert die Anzeige auf dem Monition durch x , y
  display.setCursor(50, 17);
  display.setTextSize(2);  // Setzt die Schriftgroße auf 2

  // Wenn Flash-Button gedrükt wird, wird die Value des Tasters 0
  // und dementsprechen die Temperatur in Fahrenheit angezeigt
  // ansonsten wird die Temperatur in Celsius angezeigt
  if (!digitalRead(FEH_TASTER)) {
    display.print(fahrenheitAsString);
    display.setTextSize(1);
    display.print(" F");
  } else {
    display.print(celsiusAsString);
    display.setTextSize(1);
    display.print(" C");
  }
}

/*
* Stellt die Helligkeit mit der Abkürzung "Hell." auf dem Monitor dar.
*/
void drawHelligkeit(int helligkeit) {
  display.setCursor(0, 48);
  display.setTextSize(1);
  display.println("Hell.");
  display.print(helligkeit);
}

/*
* Stellt die Luftfeuchtigkeit mit der Ablürzung "Feu." auf dem Monitor dar.
*/
void drawFeuchtigkeit(float feuchtigkeit) {
  String feuchtigkeitAsString = String(int(feuchtigkeit + 0.5));

  display.setCursor(35, 48);
  display.setTextSize(1);
  display.print("Feu.");
  display.setCursor(35, 56);
  display.print(feuchtigkeitAsString);
  display.print("%");
}

/*
* Stellt das Datum und Uhrzeit auf dem Monitor dar.
*/
void drawDateTime() {
  display.setCursor(83, 48);
  display.setTextSize(1);
  display.print(digitsAsString(hour()));
  display.print(":");
  display.print(digitsAsString(minute()));

  display.setCursor(65, 56);
  display.print(digitsAsString(day()));
  display.print(".");
  display.print(digitsAsString(month()));
  display.print(".");
  display.print(year());
}
