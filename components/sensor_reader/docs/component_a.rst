component\_a
===========

.. TODO: Kurzbeschreibung der Komponente (1–2 Sätze)

Übersicht
---------

.. TODO: Was macht diese Komponente? Welches Problem löst sie?

API
---

Initialisierung
~~~~~~~~~~~~~~~

.. code-block:: c

   struct component_a_dev dev;
   int ret = component_a_init(&dev);
   if (ret != 0) {
       /* Fehlerbehandlung */
   }

Funktionen
~~~~~~~~~~

.. TODO: Weitere Funktionen dokumentieren

Datenstrukturen
---------------

``component_a_dev``
~~~~~~~~~~~~~~~~~~~

.. TODO: Felder beschreiben

Fehlerbehandlung
----------------

Alle Funktionen geben ``0`` bei Erfolg zurück.
Negative Werte entsprechen Zephyr-Fehlercodes (``-EINVAL``, ``-EIO``, …).

Abhängigkeiten
--------------

.. TODO: Benötigte Zephyr-Subsysteme / Kconfig-Optionen auflisten,
   z.B.:
   - ``CONFIG_I2C=y``
