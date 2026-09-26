# PVHeatCtrl-HA

ESP32-Firmware für die PV-Überschussregelung eines Heizstabs über Home Assistant.

## Projektstruktur

```text
PVHeatCtrl-HA/
├── .gitignore
├── README.md
├── platformio.ini
├── doc/
│   └── ES32C14 MANUAL/
│       .....
|
├── homeassistant/
│   ├── README.md
│   ├── automations-pvheatctrl.yaml
│   ├── configuration.yaml.example
│   └── lovelace-pvheatctrl.yaml
├── include/
│   ├── secrets.h (per .gitignore ausgeschlossen !)
│   └── secrets.h.example
└── src/
    └── main.cpp
```

## Hardware

- Board: Eletechsup ES32C14, Versorgung 24 V
- PT1000: PTDNC04 über Modbus RTU
- RS485 am ES32C14:
  - GPIO1 / IO1: TX
  - GPIO3 / IO3: RX
  - GPIO22: DE/RE
- Der erste integrierte 0–10-V-Analogausgang `Vo1` wird verwendet. Laut
  Herstellerhandbuch ist `Vo1` mit ESP32 `IO25` (DAC1) verbunden. Die Firmware
  nutzt deshalb den internen 8-Bit-DAC mit `dacWrite()`; ein externer Wandler
  ist nicht erforderlich.
- Für die Spannungsausgänge muss der DIP-Schalter laut Herstellerbeispiel so
  stehen: `SW1: 1-OFF, 2-OFF, 3-ON, 4-ON`.

UART0 wird für RS485 mit 9600, 8N1 verwendet. Debug-Ausgaben laufen über
Serial2 auf GPIO16/17 mit 115200 Baud.

## Einrichtung

1. `include/secrets.h.example` nach `include/secrets.h` kopieren und Zugangsdaten
   eintragen. `secrets.h` wird nicht versioniert.
2. `platformio run` zum Kompilieren ausführen.
3. Mit `platformio run -t upload` flashen.

## Modbus

Der PTDNC04 wird mit Slave-Adresse 5 abgefragt:

- CH0: Register `0x0000`
- CH1: Register `0x0001`
- signed 16 bit, Einheit 0,1 °C

## MQTT-Schnittstelle

Home Assistant veröffentlicht:

- `pvheatctrl/config/max_power_w` (retained)
- `pvheatctrl/cmd/target_w`

Die Firmware veröffentlicht:

- `pvheatctrl/state/max_power_w`
- `pvheatctrl/state/target_w`
- `pvheatctrl/state/output_v`
- `pvheatctrl/state/temp1_c`
- `pvheatctrl/state/temp2_c`
- `pvheatctrl/state/status`

Zusätzlich veröffentlicht die Firmware MQTT Discovery-Konfigurationen unter
`homeassistant/number/pvheatctrl/.../config` und
`homeassistant/sensor/pvheatctrl/.../config`. Home Assistant erkennt damit
automatisch die Entitäten für Ziel-/Maximalleistung, Ausgangsspannung, beide
Temperaturen und den Status. Die Discovery-Nachrichten sind retained und
werden bei jeder MQTT-Verbindung erneut veröffentlicht.

Bei positiver Wirkleistung des HA-Sensors
`sensor.stromzahler_wirkleistung` gilt: positiver Wert = PV-Überschuss bzw.
Netzeinspeisung. Home Assistant sollte daher
`target_w = clamp(power_w, 0, max_power_w)` publizieren. Die
Standard-Maximalleistung beträgt 3000 W und wird im ESP32 persistent
gespeichert.

## Home-Assistant-Dateien

Unter `homeassistant/` liegen ein Lovelace-Dashboard, die Automation für die
PV-Überschussregelung sowie eine Einbindungsanleitung.

## Sollwertverlauf in Home Assistant

Der von der Automation vorgegebene Heizstab-Sollwert ist als
`number.target_power` in Home Assistant verfügbar. Die Zustandsänderungen
werden vom Home-Assistant-Recorder gespeichert und können in Lovelace mit
einer `history-graph`-Karte angezeigt werden. Das mitgelieferte Dashboard
`homeassistant/lovelace-pvheatctrl.yaml` enthält bereits einen 24-Stunden-
Verlauf für diese Entität.

Eine Änderung der ESP32-Firmware ist dafür nicht erforderlich. Die
Aufbewahrungsdauer wird über die Recorder-Konfiguration festgelegt. Beispiel
für 30 Tage Aufbewahrung:

```yaml
recorder:
  purge_keep_days: 30
```

Falls bereits eine `recorder:`-Konfiguration vorhanden ist, muss der Wert dort
ergänzt werden. Nach Änderungen an der Home-Assistant-Konfiguration ist ein
Neustart beziehungsweise ein Neuladen der betroffenen Lovelace-Konfiguration
erforderlich.

