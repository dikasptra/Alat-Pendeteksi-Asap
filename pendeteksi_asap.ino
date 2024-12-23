/*
  Proyek: Sensor Asap dengan Kontrol Kipas dan Vape (Manual & Warming Up)
*/

#define BLYNK_TEMPLATE_ID "TMPL6HnXfPL21"
#define BLYNK_TEMPLATE_NAME "Pendeteksi Asap"
#define BLYNK_AUTH_TOKEN "5QC6-Eu17dtxJULUJ1FVkPpI31fHbR-1"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Definisi pin
int smokeSensorPin = 34;   // Pin analog untuk sensor asap MQ-2
int relayFanPin = 19;      // Pin relay untuk kipas
int relayVapePin = 18;     // Pin relay untuk vape

// Variabel kontrol
bool alatOn = false;       // Status alat menyala/mati
bool warmingUpMode = false; // Status mode warming up

// Kredensial WiFi dan Blynk
char auth[] = "5QC6-Eu17dtxJULUJ1FVkPpI31fHbR-1";
char ssid[] = "kelompok6";
char pass[] = "kelompok6";

// Timer untuk tindakan yang dilakukan secara berkala
BlynkTimer timer;

// Ambang batas asap
const int smokeThreshold = 500;

void setup() {
  Serial.begin(9600);
  pinMode(smokeSensorPin, INPUT);
  pinMode(relayFanPin, OUTPUT);
  pinMode(relayVapePin, OUTPUT);

  // Matikan kipas dan vape di awal
  digitalWrite(relayFanPin, HIGH);
  digitalWrite(relayVapePin, HIGH);

  // Koneksi ke Blynk
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Timer untuk mengirim data sensor
  timer.setInterval(1000L, sendSensor);

  Serial.println("Monitoring dimulai...");
}

void loop() {
  Blynk.run();
  timer.run();
}

// Fungsi untuk membaca data sensor asap dan melakukan aksi berdasarkan tingkat asap
void sendSensor() {
  int smokeValue = analogRead(smokeSensorPin);

  // Kirim data ke aplikasi Blynk (Monitor kualitas udara di Virtual Pin V14)
  Blynk.virtualWrite(V14, smokeValue);
  Serial.print("Smoke Value: ");
  Serial.println(smokeValue);

  if (alatOn && !warmingUpMode) {
    // Matikan kipas dan vape jika asap melebihi ambang batas
    if (smokeValue > smokeThreshold) {
      digitalWrite(relayFanPin, HIGH); // Matikan kipas
      digitalWrite(relayVapePin, HIGH); // Matikan vape
      Serial.println("Smoke detected! Kipas dan vape OFF.");
    } else {
      digitalWrite(relayFanPin, LOW); // Hidupkan kipas
      digitalWrite(relayVapePin, LOW); // Hidupkan vape
      Serial.println("Air clear. Kipas dan vape ON.");
    }
  }
}

// Fungsi Blynk untuk tombol On/Off Alat (V0)
BLYNK_WRITE(V0) {
  alatOn = param.asInt(); // Baca status dari Blynk (0: Mati, 1: Nyala)

  if (alatOn) {
    Serial.println("Alat: ON");
    if (!warmingUpMode) {
      digitalWrite(relayFanPin, LOW); // Hidupkan kipas
      digitalWrite(relayVapePin, LOW); // Hidupkan vape
    }
  } else {
    Serial.println("Alat: OFF");
    digitalWrite(relayFanPin, HIGH); // Matikan kipas
    digitalWrite(relayVapePin, HIGH); // Matikan vape
  }
}

// Fungsi Blynk untuk tombol Warming Up (V1)
BLYNK_WRITE(V1) {
  warmingUpMode = param.asInt(); // Baca status dari Blynk (0: Mati, 1: Nyala)

  if (warmingUpMode) {
    Serial.println("Warming Up dimulai...");
    digitalWrite(relayFanPin, HIGH); // Matikan kipas selama warming up
    digitalWrite(relayVapePin, HIGH); // Matikan vape selama warming up

    // Timer untuk mengakhiri warming up setelah 5 menit (300.000 ms)
    timer.setTimeout(300000L, []() {
      warmingUpMode = false;
      Serial.println("Warming Up selesai...");
      if (alatOn) {
        digitalWrite(relayFanPin, LOW); // Hidupkan kipas
        digitalWrite(relayVapePin, LOW); // Hidupkan vape
      }
    });
  } else {
    Serial.println("Warming Up dihentikan...");
    if (alatOn) {
      digitalWrite(relayFanPin, LOW); // Hidupkan kipas
      digitalWrite(relayVapePin, LOW); // Hidupkan vape
    }
  }
}
