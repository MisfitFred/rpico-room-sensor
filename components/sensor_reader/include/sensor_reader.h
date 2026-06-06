/*
 * sensor_reader.h – Öffentliche Schnittstelle des Sensor-Moduls
 *
 * Diese Datei beschreibt, WAS der sensor_reader kann.
 * WIE er es tut, steht in sensor_reader.c.
 *
 * Andere Dateien (z.B. app_start.c) binden diese Datei ein,
 * um die Funktionen nutzen zu können.
 */

/* Dieser "Include-Guard" verhindert, dass die Datei versehentlich
 * doppelt eingebunden wird. */
#ifndef SENSOR_READER_H
#define SENSOR_READER_H

/* Zephyr-Bibliotheken, die wir brauchen:
 * - device.h      → Zugriff auf angeschlossene Geräte (Sensoren, LCD)
 * - kernel.h      → Zephyr-Betriebssystem-Funktionen (z.B. Threads, Sleep)
 * - auxdisplay.h  → Funktionen für Text-Displays (HD44780 über PCF8574T) */
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/auxdisplay.h>

/* -----------------------------------------------------------------------
 * Einstellungen (können hier leicht geändert werden)
 * --------------------------------------------------------------------- */

/* Wie viel Speicher (in Byte) der Hintergrund-Thread bekommt.
 * 2048 Byte reichen für unsere einfache Aufgabe. */
#define SENSOR_READER_STACK_SIZE 2048

/* Priorität des Threads. Niedrigere Zahl = höhere Priorität.
 * 5 ist ein normaler Wert für Hintergrundaufgaben. */
#define SENSOR_READER_PRIORITY 5

/* Wie oft die Sensoren abgefragt werden (in Millisekunden).
 * 1000 ms = 1 Sekunde */
#define SENSOR_READER_INTERVAL_MS 1000

/* -----------------------------------------------------------------------
 * Datenstruktur (struct)
 *
 * Ein "struct" ist wie eine Schachtel, in der mehrere Werte
 * zusammen aufbewahrt werden.
 * --------------------------------------------------------------------- */
struct sensor_reader_dev {
	const struct device *sensor_a; /* Zeiger auf den ersten Sensor  (Adresse 0x76) */
	const struct device *sensor_b; /* Zeiger auf den zweiten Sensor (Adresse 0x77) */
	const struct device *lcd;      /* Zeiger auf das LCD-Display (kann NULL sein) */
};

/* -----------------------------------------------------------------------
 * Funktions-Deklarationen
 * --------------------------------------------------------------------- */

/*
 * Initialisiert den sensor_reader.
 * Muss als Erstes aufgerufen werden.
 *
 *   dev      – die Schachtel, in der die Zeiger gespeichert werden
 *   sensor_a – Zeiger auf Sensor A (I2C-Adresse 0x76)
 *   sensor_b – Zeiger auf Sensor B (I2C-Adresse 0x77)
 *   lcd      – Zeiger auf das LCD-Display (oder NULL wenn kein Display)
 */
int sensor_reader_init(struct sensor_reader_dev *dev, const struct device *sensor_a,
                       const struct device *sensor_b, const struct device *lcd);

/*
 * Startet den Hintergrund-Thread, der die Sensoren jede Sekunde ausliest.
 * Muss nach sensor_reader_init() aufgerufen werden.
 *
 *   dev – die vorher initialisierte Schachtel
 */
int sensor_reader_start(struct sensor_reader_dev *dev);

#endif /* SENSOR_READER_H */
