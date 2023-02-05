#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// die Netzwerk-SSID (Name)
const char ssid[] = "UBBST-WLAN";  // UBBST-WLAN Vodafone-67A4
// das Netzwerkpasswort
const char pass[] = "Game2016";  //Game2016 AgGccGmL47tfhCcX
const int timeZone = 1;                  // Mitteleuropäische Zeit
// Die NTP-Zeit befindet sich in den ersten 48 Bytes der Nachricht
const int NTP_PACKET_SIZE = 48;          

// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";

// Puffer zum Speichern eingehender und ausgehender Pakete
byte packetBuffer[NTP_PACKET_SIZE];  
WiFiUDP Udp;
unsigned int localPort = 8888;  // lokaler Port, um auf UDP-Pakete zu lauschen
time_t prevDisplay = 0;         // Wenn die Digitaluhr angezeigt wurde

time_t getNtpTime();
String digitsAsString(int digits);
void sendNTPpacket(IPAddress &address);

void timeInit() {
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

void getTime() {
  if (timeStatus() != timeNotSet) {
	// Aktualisiert die Anzeige nur, wenn sich die Zeit geändert hat
    if (now() != prevDisplay) {  
      prevDisplay = now();
    }
  }
}

time_t getNtpTime() {
  IPAddress ntpServerIP;  // NTP server's ip address

  while (Udp.parsePacket() > 0)
    ;  // Alle zuvor empfangenen Pakete verwerfen
  // Holt einen zufälligen Server aus dem Pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // liest Paket in den Puffer 
      unsigned long secsSince1900;
      // Konvertiert vier Bytes, beginnend bei Position 40, in eine lange Ganzzahl
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];

      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }

  return 0;  // gib 0 zurück, wenn du die Zeit nicht bekommen kannst
}

// Sendet eine NTP-Anfrage an den Zeitserver unter der angegebenen Adresse
void sendNTPpacket(IPAddress &address) {
  // setze alle Bytes im Buffer auf 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Werte initialisieren, die zum Bilden einer
  // NTP-Anforderung erforderlich sind
  // (siehe URL oben für Details zu den Paketen)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum oder Art der Uhr
  packetBuffer[2] = 6;           // Abrufintervall
  packetBuffer[3] = 0xEC;        // Peer-Clock-Präzision
  // 8 Byte Null für Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // Alle NTP-Felder wurden jetzt mit Werten versehen
  // man kann ein Paket senden, das einen Zeitstempel anfordert:
  Udp.beginPacket(address, 123);  // NTP-Anfragen gehen an Port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

String digitsAsString(int digits) {
  String toString = String(digits);
  // Dienstprogramm für die Digitaluhranzeige: druckt
  // den vorangestellten Doppelpunkt und die führende 0
  if (digits < 10) {
    toString = "0" + toString;
  }

  return toString;
}
