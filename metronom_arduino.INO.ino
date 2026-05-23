/*
  PROSTY METRONOM NA ARDUINO

  Funkcje:
  - regulacja tempa BPM enkoderem
  - edycja górnej liczby metrum od 1 do 16
  - edycja dolnej liczby metrum od 1 do 16
  - przycisk PLAY / STOP
  - przycisk RESET
  - LCD 16x2 przez I2C
  - LED tempa
  - LED akcentu


  Uwaga muzyczna:
  BPM oznacza szybkość pojedynczego kliknięcia metronomu.
  Dla 7/8 metronom kliknie 7 razy w takcie.
  Dla 11/16 metronom kliknie 11 razy w takcie.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

/*
  Wyświetlacz LCD I2C

  Najczęstszy adres modułu LCD I2C to 0x27.
  Jeśli ekran nie działa, możliwy adres to np. 0x3F.
*/
LiquidCrystal_I2C lcd(0x27, 16, 2);

/*
  PINOUT

  Arduino Uno / Nano:

  Enkoder:
  D2 - enkoder A
  D3 - enkoder B
  D4 - przycisk enkodera

  Przyciski:
  D5 - przycisk PLAY / STOP
  D6 - przycisk RESET

  LED:
  D7 - LED zwykłego uderzenia tempa
  D8 - LED akcentu

  LCD I2C:
  A4 - SDA
  A5 - SCL
  5V - zasilanie LCD
  GND - masa

  Przyciski i enkoder podłączamy do masy.
  W kodzie używamy INPUT_PULLUP, więc nie trzeba zewnętrznych rezystorów podciągających.
*/

const int ENC_A = 2;
const int ENC_B = 3;
const int ENC_SW = 4;

const int BTN_PLAY_STOP = 5;
const int BTN_RESET = 6;

const int LED_TEMPO = 7;
const int LED_ACCENT = 8;

/*
  Tryby edycji

  EDIT_TEMPO - enkoder zmienia tempo BPM
  EDIT_METER_TOP - enkoder zmienia górną liczbę metrum
  EDIT_METER_BOTTOM - enkoder zmienia dolną liczbę metrum
*/
enum EditMode {
  EDIT_TEMPO,
  EDIT_METER_TOP,
  EDIT_METER_BOTTOM
};

EditMode editMode = EDIT_TEMPO;

/*
  Parametry metronomu
*/
int bpm = 120;

/*
  Metrum

  meterTop - górna liczba metrum, np. 3 w 3/4
  meterBottom - dolna liczba metrum, np. 4 w 3/4

  Zakres:
  1/1 do 16/16
*/
int meterTop = 4;
int meterBottom = 4;

int currentBeat = 1;

bool playing = false;

/*
  Enkoder
*/
int lastEncA = HIGH;

/*
  Zegar metronomu

  lastBeatTime - czas ostatniego uderzenia
  beatInterval - odstęp między kliknięciami w milisekundach
*/
unsigned long lastBeatTime = 0;
unsigned long beatInterval = 500;

/*
  LED

  LED zapala się tylko na krótki czas,
  a potem jest automatycznie gaszony.
*/
unsigned long ledTempoOnTime = 0;
unsigned long ledAccentOnTime = 0;

const unsigned long ledFlashTime = 60;

/*
  Debounce przycisków

  Prosta ochrona przed drganiami styków.
*/
unsigned long lastEncoderButtonTime = 0;
unsigned long lastPlayButtonTime = 0;
unsigned long lastResetButtonTime = 0;

const unsigned long debounceDelay = 180;

/*
  Własne znaki LCD

  Znak 0 - ikona PLAY
  Znak 1 - ikona STOP
*/
byte playIcon[8] = {
  B00100,
  B00110,
  B00111,
  B00111,
  B00111,
  B00110,
  B00100,
  B00000
};

byte stopIcon[8] = {
  B00000,
  B01110,
  B01110,
  B01110,
  B01110,
  B01110,
  B00000,
  B00000
};

void setup() {
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  pinMode(BTN_PLAY_STOP, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  pinMode(LED_TEMPO, OUTPUT);
  pinMode(LED_ACCENT, OUTPUT);

  digitalWrite(LED_TEMPO, LOW);
  digitalWrite(LED_ACCENT, LOW);

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, playIcon);
  lcd.createChar(1, stopIcon);

  updateBeatInterval();
  drawDisplay();
}

void loop() {
  readEncoder();
  readEncoderButton();
  readPlayStopButton();
  readResetButton();

  runMetronome();
  updateLeds();
}

/*
  Obsługa enkodera

  Obrót enkodera zmienia aktualnie wybraną wartość:
  - tempo BPM
  - górną liczbę metrum
  - dolną liczbę metrum
*/
void readEncoder() {
  int encA = digitalRead(ENC_A);

  if (encA != lastEncA && encA == LOW) {
    int direction;

    if (digitalRead(ENC_B) == HIGH) {
      direction = 1;
    } else {
      direction = -1;
    }

    changeCurrentValue(direction);
    drawDisplay();
  }

  lastEncA = encA;
}

/*
  Przycisk enkodera

  Przełącza tryb edycji:
  TEMPO -> GÓRA METRUM -> DÓŁ METRUM -> TEMPO
*/
void readEncoderButton() {
  if (digitalRead(ENC_SW) == LOW) {
    if (millis() - lastEncoderButtonTime > debounceDelay) {
      nextEditMode();
      drawDisplay();
      lastEncoderButtonTime = millis();
    }
  }
}

/*
  Przycisk PLAY / STOP

  Uruchamia albo zatrzymuje metronom.
*/
void readPlayStopButton() {
  if (digitalRead(BTN_PLAY_STOP) == LOW) {
    if (millis() - lastPlayButtonTime > debounceDelay) {
      playing = !playing;

      if (playing) {
        resetBar();

        // Pierwsze uderzenie pojawi się od razu po PLAY.
        lastBeatTime = millis() - beatInterval;
      } else {
        digitalWrite(LED_TEMPO, LOW);
        digitalWrite(LED_ACCENT, LOW);
      }

      drawDisplay();
      lastPlayButtonTime = millis();
    }
  }
}

/*
  Przycisk RESET

  Resetuje licznik taktu do pierwszego uderzenia.
*/
void readResetButton() {
  if (digitalRead(BTN_RESET) == LOW) {
    if (millis() - lastResetButtonTime > debounceDelay) {
      resetBar();

      if (playing) {
        flashBeat();
      }

      drawDisplay();
      lastResetButtonTime = millis();
    }
  }
}

/*
  Zmiana trybu edycji
*/
void nextEditMode() {
  if (editMode == EDIT_TEMPO) {
    editMode = EDIT_METER_TOP;
  } else if (editMode == EDIT_METER_TOP) {
    editMode = EDIT_METER_BOTTOM;
  } else {
    editMode = EDIT_TEMPO;
  }
}

/*
  Zmiana wartości enkoderem
*/
void changeCurrentValue(int direction) {
  if (editMode == EDIT_TEMPO) {
    bpm += direction;

    if (bpm < 30) bpm = 30;
    if (bpm > 300) bpm = 300;

    updateBeatInterval();
  }

  else if (editMode == EDIT_METER_TOP) {
    meterTop += direction;

    if (meterTop < 1) meterTop = 1;
    if (meterTop > 16) meterTop = 16;

    resetBar();
  }

  else if (editMode == EDIT_METER_BOTTOM) {
    meterBottom += direction;

    if (meterBottom < 1) meterBottom = 1;
    if (meterBottom > 16) meterBottom = 16;

    resetBar();
  }
}

/*
  Obliczanie czasu między uderzeniami

  60000 ms = jedna minuta.
  Przy 120 BPM jedno kliknięcie trwa 500 ms.

  W tej wersji BPM oznacza szybkość kliknięcia,
  niezależnie od dolnej liczby metrum.
*/
void updateBeatInterval() {
  beatInterval = 60000UL / bpm;
}

/*
  Działanie metronomu
*/
void runMetronome() {
  if (!playing) return;

  unsigned long now = millis();

  if (now - lastBeatTime >= beatInterval) {
    lastBeatTime = now;

    flashBeat();

    currentBeat++;
    if (currentBeat > meterTop) {
      currentBeat = 1;
    }

    drawDisplay();
  }
}

/*
  Reset taktu
*/
void resetBar() {
  currentBeat = 1;
}

/*
  Sprawdzenie, czy aktualne uderzenie ma być akcentowane

  W tej wersji podstawowej akcentujemy pierwsze uderzenie taktu.

  Dla metrum ósemkowych i szesnastkowych można później dodać
  dodatkowe akcenty, np. 6/8 jako 1 i 4 albo 7/8 jako 1, 4 i 6.
*/
bool isAccentBeat() {
  return currentBeat == 1;
}

/*
  Błysk LED dla uderzenia

  Uderzenie akcentowane zapala LED_ACCENT.
  Pozostałe uderzenia zapalają LED_TEMPO.
*/
void flashBeat() {
  if (isAccentBeat()) {
    digitalWrite(LED_ACCENT, HIGH);
    ledAccentOnTime = millis();
  } else {
    digitalWrite(LED_TEMPO, HIGH);
    ledTempoOnTime = millis();
  }
}

/*
  Gaszenie LED
*/
void updateLeds() {
  unsigned long now = millis();

  if (digitalRead(LED_TEMPO) == HIGH && now - ledTempoOnTime >= ledFlashTime) {
    digitalWrite(LED_TEMPO, LOW);
  }

  if (digitalRead(LED_ACCENT) == HIGH && now - ledAccentOnTime >= ledFlashTime) {
    digitalWrite(LED_ACCENT, LOW);
  }
}

/*
  Wyświetlanie na LCD

  Linia 1:
  BPM i metrum.

  Linia 2:
  PLAY/STOP, aktualny tryb edycji i aktualne uderzenie.
*/
void drawDisplay() {
  lcd.clear();

  lcd.setCursor(0, 0);

  if (editMode == EDIT_TEMPO) lcd.print("[");
  lcd.print(bpm);
  if (editMode == EDIT_TEMPO) lcd.print("]");

  lcd.print(" BPM ");

  if (editMode == EDIT_METER_TOP || editMode == EDIT_METER_BOTTOM) {
    lcd.print("[");
  }

  if (editMode == EDIT_METER_TOP) lcd.print("<");
  lcd.print(meterTop);
  if (editMode == EDIT_METER_TOP) lcd.print(">");

  lcd.print("/");

  if (editMode == EDIT_METER_BOTTOM) lcd.print("<");
  lcd.print(meterBottom);
  if (editMode == EDIT_METER_BOTTOM) lcd.print(">");

  if (editMode == EDIT_METER_TOP || editMode == EDIT_METER_BOTTOM) {
    lcd.print("]");
  }

  lcd.setCursor(0, 1);

  if (playing) {
    lcd.write(byte(0));
  } else {
    lcd.write(byte(1));
  }

  lcd.print(" ");

  if (editMode == EDIT_TEMPO) {
    lcd.print("Tempo");
  } else if (editMode == EDIT_METER_TOP) {
    lcd.print("Metr gora");
  } else {
    lcd.print("Metr dol");
  }

  lcd.print(" B:");
  lcd.print(currentBeat);
}