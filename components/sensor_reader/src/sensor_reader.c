/*
 * sensor_reader.c – Implementierung des Sensor-Moduls
 *
 * Diese Datei liest jede Sekunde Temperatur und Luftdruck
 * von zwei Bosch-Sensoren (BMP280 oder BME280) aus,
 * gibt die Werte im Log aus und zeigt sie auf dem LCD an.
 */

/* Unsere eigene Schnittstellen-Datei einbinden */
#include "sensor_reader.h"

/* Zephyr-Bibliotheken:
 * - sensor.h     → Funktionen zum Auslesen von Sensoren
 * - kernel.h     → Thread- und Sleep-Funktionen
 * - log.h        → Log-Ausgaben
 * - auxdisplay.h → LCD-Anzeige-Funktionen */
#include <zephyr/drivers/auxdisplay.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <stdio.h> /* snprintf */

/* Registriert dieses Modul beim Log-System */
LOG_MODULE_REGISTER(sensor_reader, LOG_LEVEL_INF);

/* -----------------------------------------------------------------------
 * Speicher für den Hintergrund-Thread
 * --------------------------------------------------------------------- */
K_THREAD_STACK_DEFINE(sensor_reader_stack, SENSOR_READER_STACK_SIZE);
static struct k_thread sensor_reader_thread;

/* -----------------------------------------------------------------------
 * Hilfsfunktion: einen Sensor auslesen
 *
 *   dev        – der Sensor
 *   label      – Name für die Log-Ausgabe
 *   temperatur – hier wird der Temperaturwert gespeichert
 *   luftdruck  – hier wird der Luftdruckwert gespeichert
 *
 * Gibt 0 zurück bei Erfolg, sonst einen Fehlercode.
 * --------------------------------------------------------------------- */
static int sensor_werte_lesen(const struct device *dev, const char *label,
                              struct sensor_value *temperatur, struct sensor_value *luftdruck)
{
	/* Schritt 1: Sensor auffordern, einen neuen Messwert zu erfassen */
	if (sensor_sample_fetch(dev) != 0) {
		LOG_ERR("%s: Messung fehlgeschlagen", label);
		return -EIO;
	}

	/* Schritt 2: Temperaturwert aus dem Sensor holen */
	if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, temperatur) != 0) {
		LOG_ERR("%s: Temperatur konnte nicht gelesen werden", label);
		return -EIO;
	}

	/* Schritt 3: Luftdruckwert aus dem Sensor holen */
	if (sensor_channel_get(dev, SENSOR_CHAN_PRESS, luftdruck) != 0) {
		LOG_ERR("%s: Luftdruck konnte nicht gelesen werden", label);
		return -EIO;
	}

	/* Schritt 4: Werte im Log ausgeben.
	 *
	 * Das Format "%d.%02d" gibt eine Dezimalzahl mit 2 Nachkommastellen aus.
	 *   val1         = ganzzahliger Teil
	 *   val2 / 10000 = die ersten 2 Nachkommastellen (aus Millionstel)
	 *   abs()        = Absolutwert (damit bei negativen Zahlen kein
	 *                  doppeltes Minus erscheint) */
	LOG_INF("%s: Temperatur = %d.%02d °C  |  Luftdruck = %d.%02d kPa", label, temperatur->val1,
	        abs(temperatur->val2) / 10000, luftdruck->val1, abs(luftdruck->val2) / 10000);

	return 0;
}

/* -----------------------------------------------------------------------
 * Hilfsfunktion: eine Zeile auf dem LCD anzeigen
 *
 * Format: "X: 24.3C 1013hPa" (genau 16 Zeichen für ein 16x2-Display)
 *
 *   lcd        – das LCD-Gerät
 *   name       – 'A' oder 'B' (Sensorname)
 *   temperatur – Messwert Temperatur in °C
 *   luftdruck  – Messwert Luftdruck in kPa
 *   zeile      – Zeilennummer (0 = oben, 1 = unten)
 * --------------------------------------------------------------------- */
static void lcd_zeile_anzeigen(const struct device *lcd, char name,
                               const struct sensor_value *temperatur,
                               const struct sensor_value *luftdruck, int zeile)
{
	/* Textpuffer: 16 Zeichen + Nullbyte am Ende */
	char buf[17];

	/* Luftdruck von kPa in hPa umrechnen:
	 *   kPa * 10 = hPa  (z.B. 101,3 kPa → 1013 hPa)
	 *   val1 = ganzzahliger kPa-Anteil (z.B. 101)
	 *   val2 = Nachkomma in Millionstel (z.B. 300000 für 0,3 kPa)
	 *   hPa  = val1 * 10  +  erste Stelle hinter dem Komma */
	int32_t hpa = luftdruck->val1 * 10 + abs(luftdruck->val2) / 100000;

	/* Anzeigetext zusammenstellen – genau 16 Zeichen:
	 *   %c    = 'A' oder 'B'
	 *   %3d   = Temperatur (3-stellig, rechtsbündig, z.B. " 24")
	 *   .%d   = eine Nachkommastelle der Temperatur (z.B. ".3")
	 *   C     = Grad-Celsius-Einheit
	 *   %4d   = Luftdruck in hPa (4-stellig, z.B. "1013")
	 *   hPa   = Druckeinheit
	 * Zusammen: 1+1+3+1+1+1+4+3 = 16 Zeichen */
	snprintf(buf, sizeof(buf), "%c:%3d.%dC %4dhPa", name, temperatur->val1,
	         abs(temperatur->val2) / 100000, hpa);

	/* Cursor an den Anfang der gewünschten Zeile setzen
	 * (x=0 = erste Spalte, y=zeile = Zeilennummer) */
	int rc_pos = auxdisplay_cursor_position_set(lcd, AUXDISPLAY_POSITION_ABSOLUTE, 0, zeile);

	/* Text auf das LCD schreiben */
	int rc_write = auxdisplay_write(lcd, (const uint8_t *)buf, sizeof(buf) - 1);

	/* rc_pos und rc_write müssen beide 0 sein – hier Breakpoint setzen! */
	ARG_UNUSED(rc_pos);
	ARG_UNUSED(rc_write);
}

/* -----------------------------------------------------------------------
 * Thread-Funktion: läuft dauerhaft im Hintergrund
 * --------------------------------------------------------------------- */
static void sensor_reader_thread_fn(void *p1, void *p2, void *p3)
{
	struct sensor_reader_dev *dev = (struct sensor_reader_dev *)p1;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	LOG_INF("Sensor-Thread gestartet – lese alle %d ms", SENSOR_READER_INTERVAL_MS);

	/* Endlos-Schleife: der Thread läuft solange das Gerät eingeschaltet ist */
	while (1) {
		struct sensor_value temp_a, druck_a;
		struct sensor_value temp_b, druck_b;

		/* Beide Sensoren auslesen */
		int ok_a = sensor_werte_lesen(dev->sensor_a, "Sensor A (0x76)", &temp_a, &druck_a);
		int ok_b = sensor_werte_lesen(dev->sensor_b, "Sensor B (0x77)", &temp_b, &druck_b);

		/* Wenn ein LCD vorhanden ist, die Werte dort anzeigen */
		if (dev->lcd != NULL) {
			if (ok_a == 0) {
				lcd_zeile_anzeigen(dev->lcd, 'A', &temp_a, &druck_a, 0);
			}
			if (ok_b == 0) {
				lcd_zeile_anzeigen(dev->lcd, 'B', &temp_b, &druck_b, 1);
			}
		}

		/* 1 Sekunde warten, bevor die nächste Messung startet */
		k_sleep(K_MSEC(SENSOR_READER_INTERVAL_MS));
	}
}

/* -----------------------------------------------------------------------
 * Öffentliche Funktionen (werden von app_start.c aufgerufen)
 * --------------------------------------------------------------------- */

int sensor_reader_init(struct sensor_reader_dev *dev, const struct device *sensor_a,
                       const struct device *sensor_b, const struct device *lcd)
{
	/* Sicherheitsprüfung: wurden gültige Zeiger übergeben? */
	if (dev == NULL || sensor_a == NULL || sensor_b == NULL) {
		return -EINVAL;
	}

	/* Prüfen, ob Sensor A vom Betriebssystem erkannt und bereit ist */
	if (!device_is_ready(sensor_a)) {
		LOG_ERR("Sensor A (0x76) ist nicht bereit");
		return -ENODEV;
	}

	/* Prüfen, ob Sensor B vom Betriebssystem erkannt und bereit ist */
	if (!device_is_ready(sensor_b)) {
		LOG_ERR("Sensor B (0x77) ist nicht bereit");
		return -ENODEV;
	}

	/* LCD prüfen und Hintergrundbeleuchtung einschalten (falls vorhanden) */
	if (lcd != NULL && device_is_ready(lcd)) {
		dev->lcd = lcd;
		auxdisplay_backlight_set(dev->lcd, 1);
		auxdisplay_clear(dev->lcd);
		auxdisplay_cursor_position_set(dev->lcd, AUXDISPLAY_POSITION_ABSOLUTE, 0, 0);
		auxdisplay_write(dev->lcd, (const uint8_t *)"Starte...       ", 16);
		auxdisplay_cursor_position_set(dev->lcd, AUXDISPLAY_POSITION_ABSOLUTE, 0, 1);
		auxdisplay_write(dev->lcd, (const uint8_t *)"Sensoren pruefen", 16);

		LOG_INF("LCD bereit");
	} else {
		dev->lcd = NULL;
		if (lcd != NULL) {
			LOG_WRN("LCD nicht bereit – Anzeige deaktiviert");
		}
	}

	/* Sensor-Zeiger in unserer Schachtel speichern */
	dev->sensor_a = sensor_a;
	dev->sensor_b = sensor_b;

	LOG_INF("sensor_reader bereit");
	return 0;
}

int sensor_reader_start(struct sensor_reader_dev *dev)
{
	if (dev == NULL) {
		return -EINVAL;
	}

	k_thread_create(&sensor_reader_thread, sensor_reader_stack,
	                K_THREAD_STACK_SIZEOF(sensor_reader_stack), sensor_reader_thread_fn, dev,
	                NULL, NULL, SENSOR_READER_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(&sensor_reader_thread, "sensor_reader");

	LOG_INF("Sensor-Thread gestartet");
	return 0;
}
