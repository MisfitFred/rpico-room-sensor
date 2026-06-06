/*
 * app_start.c – Startpunkt der Anwendung
 *
 * Diese Datei ist der Einstiegspunkt des Programms.
 * Sie richtet alles ein und startet den Sensor-Thread.
 */

/* Zephyr-Bibliotheken für Geräte und Logging */
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

/* Unser Sensor-Modul */
#include "sensor_reader.h"

/* Dieses Modul beim Log-System anmelden */
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* -----------------------------------------------------------------------
 * Sensor-Definitionen
 *
 * DT_NODELABEL liest die Hardware-Konfiguration aus dem Device Tree
 * (prj.overlay). Dort steht, welche Geräte angeschlossen sind und an
 * welcher I2C-Adresse sie sitzen.
 * --------------------------------------------------------------------- */
#define BME280_76_NODE DT_NODELABEL(bme280_76) /* Sensor A an I2C-Adresse 0x76 */
#define BME280_77_NODE DT_NODELABEL(bme280_77) /* Sensor B an I2C-Adresse 0x77 */
#define LCD_NODE DT_NODELABEL(auxdisplay_0)    /* HD44780 LCD über PCF8574T */

/* Zeiger auf die Geräte – werden beim Booten vom Betriebssystem befüllt */
static const struct device *sensor76 = DEVICE_DT_GET(BME280_76_NODE);
static const struct device *sensor77 = DEVICE_DT_GET(BME280_77_NODE);
static const struct device *lcd = DEVICE_DT_GET(LCD_NODE);

/* Die Schachtel für den sensor_reader – speichert alle Geräte-Zeiger */
static struct sensor_reader_dev reader;

/* -----------------------------------------------------------------------
 * app_start – wird einmalig beim Programmstart aufgerufen
 *
 * Gibt 0 zurück wenn alles geklappt hat, sonst eine negative Fehlerzahl.
 * --------------------------------------------------------------------- */
int app_start(void)
{
	LOG_INF("Anwendung gestartet");

	/* Schritt 1: sensor_reader initialisieren.
	 * Wir übergeben die Schachtel, die zwei Sensor-Zeiger und das LCD.
	 * Das & vor "reader" bedeutet "Adresse von reader" (Zeiger darauf). */
	int ret = sensor_reader_init(&reader, sensor76, sensor77, lcd);
	if (ret != 0) {
		LOG_ERR("Initialisierung fehlgeschlagen: %d", ret);
		return ret;
	}

	/* Schritt 2: Den Hintergrund-Thread starten.
	 * Ab jetzt laufen die Sensormessungen automatisch alle 1 Sekunde
	 * und die Werte werden auf dem LCD angezeigt. */
	ret = sensor_reader_start(&reader);
	if (ret != 0) {
		LOG_ERR("Thread-Start fehlgeschlagen: %d", ret);
		return ret;
	}

	return 0; /* Alles gut – der Thread läuft jetzt selbstständig */
}
