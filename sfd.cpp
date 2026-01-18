#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  

const int trigPin = 9;
const int echoPin = 8;
const int ledGreen = 2;
const int ledYellow = 3;
const int ledRed = 4;
const int buzzerPin = 5;

// --- Variabel Logika ---
int distance;
unsigned long stateStartTime = 0; 
String currentStatus = "";
String lastL1 = "", lastL2 = "";

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  
  showWelcome();
}

void loop() {
  distance = getDistance();
  unsigned long currentTime = millis();

  // A. JARAK 0 - 20 cm
  if (distance >= 0 && distance <= 20) {
    currentStatus = "OFF";
    updateOutput(LOW, LOW, LOW, LOW, "JARAK TERLALU", "DEKAT");
  }

  // B. JARAK 21 - 40 cm (HIJAU)
  else if (distance >= 21 && distance <= 40) {
    if (currentStatus != "FOKUS") {
      if (currentTime - stateStartTime >= 3000) {
        currentStatus = "FOKUS";
      }
      updateOutput(HIGH, LOW, LOW, LOW, "STABILISASI...", "Tahan Posisi");
    } else {
      updateOutput(HIGH, LOW, LOW, LOW, "FOKUS", "Sangat Baik!");
    }
  }

  // C. JARAK 41 - 55 cm (KUNING)
  else if (distance >= 41 && distance <= 55) {
    currentStatus = "KURANG FOKUS";
    stateStartTime = currentTime; 
    updateOutput(LOW, HIGH, LOW, LOW, "KURANG FOKUS", "AYO FOKUS LAGI");
  }

  // D. JARAK 56 - 80 cm (MERAH - TIDAK FOKUS)
  else if (distance >= 56 && distance <= 80) {
    if (currentTime - stateStartTime < 2000
    ) {
       // Fase tunggu 5 detik
       updateOutput(LOW, LOW, HIGH, LOW, "HEY KAMU!", "JANGAN JAUH JAUH");
    } 
    else {
       currentStatus = "TIDAK FOKUS";
       
       // Kontrol LED & LCD secara manual agar tidak bentrok dengan buzzer
       digitalWrite(ledGreen, LOW);
       digitalWrite(ledYellow, LOW);
       digitalWrite(ledRed, HIGH);
       
       if (lastL1 != "TIDAK FOKUS") {
         lcd.setCursor(0, 0); lcd.print("TIDAK FOKUS     ");
         lcd.setCursor(0, 1); lcd.print("KEMBALI TEGAK!  ");
         lastL1 = "TIDAK FOKUS";
       }

       // LOGIKA BEEP-BEEP AKTIF
       if ((millis() / 250) % 2 == 0) {
         digitalWrite(buzzerPin, HIGH);
       } else {
         digitalWrite(buzzerPin, LOW);
       }
    }
  }

  // E. JARAK > 80 cm
  else {
    currentStatus = "STANDBY";
    stateStartTime = currentTime;
    updateOutput(LOW, LOW, LOW, LOW, "DI LUAR JANGKAU", "KAMU DIMANA?");
  }
  
  delay(50); 
}

// --- FUNGSI PENDUKUNG ---

int getDistance() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long dur = pulseIn(echoPin, HIGH, 30000);
  int d = dur * 0.034 / 2;
  if (d == 0) return 999;
  return d;
}

void updateOutput(int g, int y, int r, int b, String l1, String l2) {
  digitalWrite(ledGreen, g);
  digitalWrite(ledYellow, y);
  digitalWrite(ledRed, r);
  
  // Perbaikan utama: Jangan matikan buzzer jika statusnya sedang TIDAK FOKUS
  if (currentStatus != "TIDAK FOKUS") {
    digitalWrite(buzzerPin, b);
  }

  if (l1 != lastL1 || l2 != lastL2) {
    lcd.setCursor(0, 0); lcd.print("                ");
    lcd.setCursor(0, 0); lcd.print(l1);
    lcd.setCursor(0, 1); lcd.print("                ");
    lcd.setCursor(0, 1); lcd.print(l2);
    lastL1 = l1; lastL2 = l2;
  }
}

void showWelcome() {
  lcd.print("Smart Focus");
  lcd.setCursor(0, 1);
  lcd.print("By: Timboikun");
  delay(2000);
  lcd.clear();
}