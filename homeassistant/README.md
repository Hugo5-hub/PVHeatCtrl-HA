# Home Assistant

Die ESP32-Firmware verwendet MQTT Discovery. Nach dem ersten erfolgreichen MQTT-
Connect legt Home Assistant die PVHeatCtrl-Entitaeten automatisch an.

## Voraussetzungen

- MQTT-Integration in Home Assistant ist eingerichtet.
- MQTT Discovery ist im verwendeten MQTT-Broker bzw. in Home Assistant aktiviert.
- Der ESP32 ist mit dem MQTT-Broker verbunden.
- Der Leistungssensor heisst `sensor.stromzahler_wirkleistung`.

## Dashboard einbinden

1. In Home Assistant zu Einstellungen > Dashboards wechseln.
2. Ein neues Dashboard im YAML-Modus anlegen oder die Datei
   `lovelace-pvheatctrl.yaml` als Dashboard-Konfiguration verwenden.
3. Die Datei aus diesem Verzeichnis in die Home-Assistant-Konfiguration kopieren.

Das Dashboard verwendet diese automatisch entdeckten Entity IDs:

- `number.target_power`
- `number.maximum_power`
- `sensor.output_voltage`
- `sensor.temperature_1`
- `sensor.temperature_2`
- `sensor.status`

Falls Home Assistant andere Entity IDs vergeben hat, muessen nur die IDs in der
Dashboard-Datei angepasst werden.

## Automation einbinden

Den Inhalt von `automations-pvheatctrl.yaml` in Home Assistant unter
Einstellungen > Automationen und Szenen importieren oder in `automations.yaml`
uebernehmen. Bei negativer Wirkleistung wird der Betrag als PV-Ueberschuss an den
ESP32 gesendet. Positive Netzleistung setzt den Heizstab auf 0 W. Der Wert wird
auf `number.maximum_power` begrenzt.

Nach Aenderungen an YAML-Dateien die Home-Assistant-Konfiguration pruefen und
die Automationen neu laden.
