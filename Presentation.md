# GitHub – Einfach erklärt

## Das große Bild

```
        [ Dein Computer ]          [ GitHub (Internet) ]
        ─────────────────          ────────────────────
        Dateien ändern    ──────►  Repository (Projektordner)
        git commit                 = Code + gesamte History
        git push
```

---

## Die 4 wichtigsten Begriffe

```
REPOSITORY
┌─────────────────────────────┐
│  Dein gesamtes Projekt      │
│  + alle Änderungen ever     │
└─────────────────────────────┘

COMMIT  =  ein Speicherpunkt
    ●────●────●────●
    v1   v2   v3   v4   ◄── jederzeit zurückspulbar

BRANCH  =  eine parallele Arbeitskopie
    main    ──●──────────────●──
                \            ↑
    feature      ●──●──●──●──  (Merge)

PULL REQUEST  =  "Ich möchte meinen Code einbauen"
    → andere schauen drüber → dann Merge
```

---

## Typischer Ablauf

```
  [1] Änderung machen
         │
  [2] git add .          ← "diese Dateien merken"
         │
  [3] git commit -m "..."← "Speicherpunkt setzen"
         │
  [4] git push           ← "auf GitHub hochladen"
         │
  [5] Pull Request       ← "zum Einbauen freigeben"
```

---

# Build-Prozess & Debugger

## Build-Prozess – Wie wird Code zur Firmware?

```
  [ Quellcode ]      [ Compiler ]      [ Firmware ]      [ Gerät ]
  ─────────────      ────────────      ────────────      ────────
  main.c        ──►  übersetzt    ──►  program.uf2  ──►  Pico W
  sensor.c          (für Menschen       (für den         blinkt!
  config.h           lesbar)            Computer)
```

```
  Schritt für Schritt:

  [1] west build       ← Code wird übersetzt (kompiliert)
         │
         ▼
  [2] Fehler?
      ja ──► Fehlermeldung lesen, Code korrigieren, zurück zu [1]
      nein ─► weiter
         │
         ▼
  [3] west flash       ← Firmware auf den Pico W laden
```

---

## Debugger – Was steckt dahinter?

```
  OHNE Debugger:              MIT Debugger:
  ─────────────               ────────────
  Code läuft durch            Code hält an bestimmten
  → Fehler passiert             Stellen an  (Breakpoint)
  → Wo? Keine Ahnung!         → Man kann reinschauen:
                                  Welchen Wert hat Variable x?
                                  Was passiert als nächstes?
```

```
  [ VS Code ]  ──►  [ Debugger ]  ──►  [ Pico W ]
      │                  │                  │
   Breakpoint        hält an            führt Code
   setzen            der Stelle         schrittweise
   (roter Punkt)     im Code            aus
```

```
  Nützliche Aktionen im Debugger:

  ▶  Weiter          → läuft bis zum nächsten Breakpoint
  ↓  Schritt rein    → geht in eine Funktion rein
  ↷  Schritt drüber  → führt eine Zeile aus, geht nicht rein
  ⏹  Stopp           → Debugging beenden
```

---

# Wie sieht echter Code aus?

## Ein einfaches Beispiel – LED blinkt

```
  Was der Mensch will:       Was der Computer versteht:
  ──────────────────         ──────────────────────────
  "Lass die LED              led_on();
   an und aus gehen"         warte(1 Sekunde);
                             led_off();
                             warte(1 Sekunde);
                             → von vorne ...
```

```c
// So sieht das in echtem Code aus:

while (true) {          // ← "mache das für immer"
    led_on();           // ← LED einschalten
    k_sleep(1000);      // ← 1 Sekunde warten
    led_off();          // ← LED ausschalten
    k_sleep(1000);      // ← 1 Sekunde warten
}
```

> Jede Zeile = ein Befehl. Der Computer führt sie **der Reihe nach** aus.

---

# Das Projekt – Raum-Sensor

## Was macht das Gerät?

```
                        ┌─────────────────┐
                        │    Pico W        │
   ┌──────────┐         │                 │         ┌──────────────┐
   │  Sensor  │──────►  │  liest Daten    │ ──────► │   Bildschirm │
   │          │         │  verarbeitet    │  Kabel  │   zeigt an   │
   │ Temp 22° │         │  gibt aus       │         │   Temp, Hum  │
   │ Hum  55% │         │                 │         └──────────────┘
   └──────────┘         └─────────────────┘
```

## Was misst er?

```
  Temperatur   → Wie warm ist es im Raum?
  Luftfeuchte  → Wie feucht ist die Luft?
```

## Wie hängt alles zusammen?

```
  [Sensor misst]  ──►  [Pico W rechnet]  ──►  [Bildschirm zeigt an]
        │                                               │
        └───────────────────────────────────────────────┘
                     läuft automatisch, immer wieder
```

---

# Hardware-Aufbau, Treiber & I2C

## Wie ist die Hardware aufgebaut?

```
                        ┌──────────────────────────┐
                        │         Pico W            │
                        │                           │
   ┌──────────┐         │  I2C Bus 0                │
   │ BME280 A │─────────│  GP4 (SDA) / GP5 (SCL)   │
   │ (0x76)   │         │                           │
   ├──────────┤         │  I2C Bus 1                │    ┌───────────┐   ┌──────────────────┐
   │ BME280 B │─────────│  GP6 (SDA) / GP7 (SCL)   │───►│ PCF8574T  │──►│ LCD 16x2         │
   │ (0x77)   │         │                           │    │ (0x27)    │   │ A: 24.5C 1013hPa │
   └──────────┘         └──────────────────────────┘    └───────────┘   │ B: 23.2C 1012hPa │
                                                                          └──────────────────┘
  Sensoren:   eigene Adressen  →  0x76  und  0x77
  PCF8574T:   Zwischenstecker für das Display  →  Adresse 0x27
  → so weiß der Pico W immer, wer gerade gemeint ist
```

```
  Alle Teile sind per Kabel verbunden:
  SDA  →  Datenleitung  (schickt die Messwerte)
  SCL  →  Taktleitung   (gibt den Rhythmus vor)
  VCC  →  Strom
  GND  →  Masse (Minus)
```

## Was misst der BME280?

```
  BME280  →  ein kleiner Chip von Bosch

  ┌─────────────────────────────────┐
  │           BME280                │
  │                                 │
  │  Temperatur   →  z.B. 24.5 °C  │
  │  Luftdruck    →  z.B. 1013 hPa │
  └─────────────────────────────────┘

  Im Projekt: 2 Sensoren gleichzeitig  →  Sensor A & Sensor B
  Beide Werte werden jede Sekunde gemessen und auf dem Display angezeigt
```

## Was ist I2C?

```
  I2C = eine vereinbarte "Sprache" zwischen Bauteilen
        funktioniert über nur 2 Kabel

  Pico W fragt Sensor A:
  ┌──────────┐  "Hey 0x76, gib mir die Temperatur!"  ┌──────────┐
  │  Pico W  │ ─────────────────────────────────────► │ BME280 A │
  │          │ ◄───────────────────────────────────── │ (0x76)   │
  └──────────┘            "24.5 °C !"                └──────────┘

  Pico W fragt Sensor B:
  ┌──────────┐  "Hey 0x77, gib mir die Temperatur!"  ┌──────────┐
  │  Pico W  │ ─────────────────────────────────────► │ BME280 B │
  │          │ ◄───────────────────────────────────── │ (0x77)   │
  └──────────┘            "23.2 °C !"                └──────────┘

  → Pico W fragt   = Master
  → Sensoren antworten = Slaves
  → Adresse (0x76 / 0x77) verhindert Verwechslung
```

## Was ist ein Treiber?

```
  PROBLEM:
  Der Pico W versteht nur 0 und 1 (Strom an / aus)
  Der BME280 spricht eine eigene "Sprache"

  LÖSUNG → Treiber  (fertig mitgeliefert von Zephyr)

  ┌──────────┐    ┌─────────────────────┐    ┌──────────┐
  │  Pico W  │ ─► │   BME280-Treiber    │ ─► │  BME280  │
  │          │    │   "Übersetzer"      │    │          │
  │ "messen" │    │   weiß genau wie    │    │  misst   │
  │          │ ◄─ │   man den Chip      │ ◄─ │          │
  │ 24.5 °C  │    │   ansprechen muss   │    │ 24.5 °C  │
  └──────────┘    └─────────────────────┘    └──────────┘

  Treiber aktivieren  →  eine Zeile in der Konfiguration:
  CONFIG_BME280=y     →  "ja, benutze den BME280-Treiber"
```
