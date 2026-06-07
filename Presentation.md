# GitHub – Einfach erklärt

## Das große Bild

```
        [ Dein Computer ]                [ GitHub (Internet) ]
        ─────────────────                ────────────────────
                           git push ──►
        Dateien ändern                   Repository (Projektordner)
        git commit                       = Code + gesamte History
                           git pull ◄──
                        (Änderungen holen)
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
  main.c        ──►  übersetzt    ──►  zephyr.uf2   ──►  Pico W
  sensor.c           (für den                            blinkt!
  config.h           Computer)
  (für Menschen 
  lesbar)                     
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
  → Wo? Keine Ahnung!
         → Man kann reinschauen:
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



# Das Projekt – Raum-Sensor

## Was macht das Gerät?

```
                        ┌─────────────────┐
                        │    Pico W       │
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
  ┌─────────────┐                        ┌─────────────────────────────┐
  │  BME280 A   │                        │           Pico W            │
  │  Adresse    │                        │                             │
  │   0x76      │──SDA──► GP4            │  I2C Bus 0  (GP4 + GP5)     │
  │             │──SCL──► GP5 ───────────│  → liest Sensor A & B       │
  ├─────────────┤                        │                             │
  │  BME280 B   │──SDA──► GP4            │                             │
  │  Adresse    │──SCL──► GP5            │                             │
  │   0x77      │                        │                             │
  └─────────────┘                        │  I2C Bus 1  (GP6 + GP7)     │
                                         │  → steuert das Display      │
  ┌─────────────┐                        │                             │
  │  PCF8574T   │──SDA──► GP6 ───────────│                             │
  │  (Zwischen- │──SCL──► GP7            │                             │
  │   stecker)  │                        └─────────────────────────────┘
  │  Adresse    │
  │   0x27      │──► LCD 16x2
  └─────────────┘    ┌──────────────────┐
                     │ A: 24.5C 1013hPa │
                     │ B: 23.2C 1012hPa │
                     └──────────────────┘
```

```
  Jedes Kabel hat eine Aufgabe:

  SDA  →  Datenleitung  (hier fließen die Messwerte)
  SCL  →  Taktleitung   (gibt den Rhythmus vor, wann Daten kommen)
  VCC  →  Strom (Plus)
  GND  →  Masse (Minus)
```

## Was misst der BME280?

```
  BME280  →  ein kleiner Chip von Bosch

  ┌─────────────────────────────────┐
  │           BME280                │
  │                                 │
  │  Temperatur   →  z.B. 24.5 °C   │
  │  Luftdruck    →  z.B. 1013 hPa  │
  └─────────────────────────────────┘

  Im Projekt: 2 Sensoren gleichzeitig  →  Sensor A & Sensor B
  Beide Werte werden jede Sekunde vom Programm abgefragt und auf dem Display angezeigt
```

## Was ist I2C?

```
  I2C = eine vereinbarte "Sprache" zwischen Bauteilen
        funktioniert über nur 2 Kabel

  Pico W fragt Sensor A:
  ┌──────────┐  "Hey 0x76, gib mir die Temperatur!"  ┌──────────┐
  │  Pico W  │ ────────────────────────────────────► │ BME280 A │
  │          │ ◄──────────────────────────────────── │ (0x76)   │
  └──────────┘            "24.5 °C !"                └──────────┘

  Pico W fragt Sensor B:
  ┌──────────┐  "Hey 0x77, gib mir die Temperatur!"  ┌──────────┐
  │  Pico W  │ ─────────────────────────────────────►│ BME280 B │
  │          │ ◄─────────────────────────────────────│ (0x77)   │
  └──────────┘            "23.2 °C !"                └──────────┘

  → Pico W fragt   = Controller
  → Sensoren antworten = Peripheral
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

```
  SW-Architektur – wo sitzt was?

  ┌─────────────────────────────────────────────┐
  │                 Anwendung                   │  ← unser Code
  │           sensor_reader.c                   │     "lies Temperatur"
  └───────────────────┬─────────────────────────┘
                      │ benutzt
  ┌───────────────────▼─────────────────────────┐
  │              Zephyr RTOS                    │  ← Betriebssystem
  │         sensor_fetch / sensor_get           │
  └───────────────────┬─────────────────────────┘
                      │ ruft auf
  ┌───────────────────▼─────────────────────────┐
  │            BME280-Treiber                   │  ← Übersetzer
  │        (mitgeliefert von Zephyr)            │
  └───────────────────┬─────────────────────────┘
                      │ spricht über I2C
  ┌───────────────────▼─────────────────────────┐
  │             BME280-Sensor                   │  ← Hardware
  │          (echte Elektronik)                 │
  └─────────────────────────────────────────────┘
```
