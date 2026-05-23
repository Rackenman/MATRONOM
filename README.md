# PROSTY METRONOM NA ARDUINO

Ten projekt to w pełni funkcjonalny, cyfrowy metronom oparty na platformie Arduino. Pozwala na regulację tempa oraz zmianę metrum za pomocą enkodera obrotowego. Parametry wyświetlane są na ekranie LCD 16x2, a uderzenia sygnalizowane są za pomocą dwóch diod LED: zwykłe uderzenie oraz akcent na pierwszą miarę taktu.

Projekt powstał na zajęcia laboratoryjne z przedmiotu **Systemy wbudowane** na **Uniwersytecie Vizja**.

Obecna wersja opiera się wyłącznie na sygnalizacji wizualnej, czyli diodach LED. Projekt nie generuje dźwięku ani nie wysyła komunikatów MIDI.

-----------------------------------------

## GŁÓWNE FUNKCJE

-----------------------------------------

- Regulacja tempa w zakresie 30 - 300 BPM.
- Pełna edycja metrum: górna i dolna liczba od 1 do 16.
- Intuicyjny interfejs oparty na enkoderze obrotowym z przyciskiem.
- Niezależne przyciski PLAY / STOP oraz RESET.
- Natychmiastowy powrót do pierwszego uderzenia taktu.
- Wizualny wskaźnik uderzeń:
  - dioda akcentu, czerwona - pierwsze uderzenie w takcie,
  - dioda tempa, zielona - pozostałe uderzenia.
- Niestandardowe ikony Play i Stop na ekranie LCD.

-----------------------------------------

## WYMAGANE ELEMENTY

-----------------------------------------

1. Płytka Arduino, np. Uno lub Nano.
2. Wyświetlacz LCD 16x2 z konwerterem I2C.
3. Enkoder obrotowy, np. moduł KY-040.
4. 2x przycisk typu tact-switch, dla Play/Stop i Reset.
5. 2x dioda LED, sugerowane kolory: czerwona i zielona.
6. 2x rezystor 220 Ohm do diod LED.
7. Płytka stykowa i przewody połączeniowe.

-----------------------------------------

## SCHEMAT POŁĄCZEŃ

-----------------------------------------

Wszystkie przyciski oraz enkoder podłączamy między odpowiedni pin cyfrowy a masę GND. Kod wykorzystuje wewnętrzne rezystory podciągające Arduino, czyli `INPUT_PULLUP`, dlatego nie są potrzebne zewnętrzne rezystory dla przełączników.

### Enkoder obrotowy

- CLK / A -> D2
- DT / B -> D3
- SW / przycisk -> D4
- VCC / + -> 5V
- GND -> GND

### Przycisk PLAY/STOP

- Nóżka 1 -> D5
- Nóżka 2 -> GND

### Przycisk RESET

- Nóżka 1 -> D6
- Nóżka 2 -> GND

### LED tempa, zielona

- Anoda + -> D7
- Katoda - -> rezystor 220 Ohm -> GND

### LED akcentu, czerwona

- Anoda + -> D8
- Katoda - -> rezystor 220 Ohm -> GND

### LCD 16x2 I2C

- SDA -> A4
- SCL -> A5
- VCC -> 5V
- GND -> GND

-----------------------------------------

## WYMAGANE BIBLIOTEKI

-----------------------------------------

Aby skompilować kod, należy upewnić się, że w Arduino IDE są zainstalowane następujące biblioteki:

- `Wire.h` - wbudowana w Arduino IDE.
- `LiquidCrystal_I2C` - autor: Frank de Brabander, dostępna w Menedżerze Bibliotek Arduino IDE.

Domyślny adres I2C wyświetlacza w kodzie to `0x27`. Jeśli ekran nic nie wyświetla, można spróbować zmienić adres w kodzie na `0x3F`.

-----------------------------------------

## INSTRUKCJA OBSŁUGI

-----------------------------------------

1. **Uruchomienie**

   Po włączeniu metronom jest zatrzymany. Na ekranie widoczne jest domyślne tempo 120 BPM i metrum 4/4.

2. **Nawigacja w menu**

   Nawigacja odbywa się przez przycisk wbudowany w enkoder. Naciśnięcie przycisku przełącza tryby edycji:

   - `[ Tempo ]` - edycja BPM.
   - `< Metr góra >` - edycja liczby uderzeń w takcie.
   - `< Metr dół >` - edycja dolnej wartości metrum.

   Aktywny tryb edycji jest oznaczony nawiasami `[]` lub `<>` w pierwszej linii wyświetlacza oraz opisem w drugiej linii.

3. **Zmiana wartości**

   Obrót gałką enkodera w lewo lub w prawo zmniejsza albo zwiększa wartość aktualnie wybranego parametru.

4. **Odtwarzanie**

   Naciśnięcie przycisku PLAY/STOP na pinie D5 uruchamia metronom. Diody zaczynają migać zgodnie z ustawionym tempem i metrum. Ponowne naciśnięcie zatrzymuje odliczanie.

5. **Reset / synchronizacja**

   W trakcie odtwarzania można nacisnąć przycisk RESET na pinie D6. Metronom natychmiast wymusi pierwsze uderzenie taktu, czyli akcent. Jest to przydatne do zsynchronizowania urządzenia z grającym już zespołem.

-----------------------------------------

## UWAGA MUZYCZNA

-----------------------------------------

W tym algorytmie wartość BPM określa szybkość pojedynczego, fizycznego kliknięcia lub błysku diody.

- Dla metrum 4/4 przy 120 BPM zobaczysz 120 uderzeń ćwierćnutowych na minutę.
- Dla metrum 7/8 przy tym samym ustawieniu zobaczysz 7 uderzeń w jednym cyklu taktu z tą samą szybkością.

Algorytm nie przelicza wartości nutowej metrum na czas absolutny. Metrum określa przede wszystkim liczbę uderzeń w cyklu i miejsce akcentu.

-----------------------------------------

## MOŻLIWE KIERUNKI ROZWOJU

-----------------------------------------

Projekt można dalej rozwijać i modyfikować. Szczególnie przydatne byłoby dodanie:

- obsługi MIDI, aby metronom mógł synchronizować się z zewnętrznymi urządzeniami,
- możliwości wysyłania komunikatów MIDI Clock,
- możliwości odbierania zewnętrznego zegara MIDI,
- funkcji obrotu ekranu lub zmiany układu informacji na wyświetlaczu,
- funkcji nabijania tempa, czyli ustawiania BPM przez kilkukrotne naciśnięcie przycisku w rytmie,
- sygnalizacji dźwiękowej, np. przez buzzer lub mały głośnik,
- dodatkowych trybów metrum,
- zapisywania ustawień po wyłączeniu urządzenia,
- bardziej rozbudowanego menu użytkownika.

-----------------------------------------

## LICENCJA

-----------------------------------------

Projekt udostępniam na licencji **Creative Commons**.

Zachęcam do swobodnego analizowania, modyfikowania i rozwijania projektu w celach edukacyjnych. Mile widziane są dalsze edycje, eksperymenty z kodem oraz dodawanie własnych funkcji.
```
