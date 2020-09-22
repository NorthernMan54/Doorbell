## Doorbell

This is a fork of [@alcar21](https://github.com/alcar21/WemosEM) which is a fork of [@nephiel](https://github.com/Nephiel/MQTT-Power-Sensor) and [@Mottramlabs](https://github.com/Mottramlabs/MQTT-Power-Sensor) projects.

This is a Doorbell current sensor used to detect button presses and send MQTT messages when it detects that the doorbell button has been pressed and released, and a heartbeat message with status every 10 seconds.  For a current sensor I'm using a ACS712 Hall-Effect-Based Linear Current Sensor to detect the current draw of my 24 VAC Doorbell.  For this I'm using this [one](https://www.amazon.ca/gp/product/B07YP3G4N8/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1) from Amazon.  For the record this is the 5 amp version based on this chip ACS712ELC-05A.  I had checked my doorbell before ordering and determined that it drew 1 amp at 24 VAC when the door bell was running.

## Circuit

The circuit for this was pretty simple just the ESP8266 and ACS712. ![simple](docs/IMG_5339.jpg) That was my working prototype before I soldered up the final version. I powered the ACS712 from the 5v pin on the ESP8266, and connected the output of the ACS712 to the ADC pin of the ESP8266.  And here is my final device.

![Final](docs/IMG_5361.jpg)

## MQTT Message Structure

```
{
  "current": "0.00",           // This is the current reported by the ACS712 Sensor
  "voltage": "2.53",           // This is the current voltage measured by the ADC pin on the ESP8266
  "doorbellLength": "182",     // This is the length in milli seconds of the most recent button press
  "doorbellTime": "21:12:38 08/09/2020", // This is the time of the most recent button press
  "doorbellPressed": "No",     // Set to "Yes" when the button is released
  "mqttreconnected": "1",
  "wifidb": "70% Poor",
  "uptime": "   0 days 00:41:39",
  "time": "21:39:39 08/09/2020",
  "freemem": 32488,
  "version": "WemosDB 1.3"
}
```

## Integration in Homebridge and Alexa

I integrated this into homebridge and the homebridge-camera-ffmpeg plugin using node-red to catch the doorbell pressed event message and trigger the doorbell trigger accessory.

The flow looks like this. ![Node Red Flow](docs/node-red-doorbell.png)

The "Doorbell Doorbell Trigger" node is from node-red homebridge-automation module.

Here is my final implementation of the door bell with integration into [Alexa](https://youtu.be/PhGbc_TO8pk).

## Rebuild WEB package

1 - Install GULP package ( npm install -g gulp )

2 - cd ~/Code/Doorbell

3 - npm install

4 - ~/npm/bin/gulp

# Need to cleanup below this line, this is the original README from WemosEM Codebase / Repository



Thanks to Dawies [www.domology.es](https://domology.es/medidor-de-consumo-no-invasivo-con-sct013-wemos-d1/) for his contributions and his tutorial

The sensor uses a Wemos D1 Mini and PRO and a Non-invasive Split Core Current Transformer type 100A SCT-013-000, available on eBay.
You can use Current Transformer type calibrated SCT-013-030 (30A), SCT-013-050 (50A) and others. Recomended Clamp 30A or higger.

The sensor samples the current value every 10 seconds and publishes it to a topic via an MQTT broker. you can change this value up to 5 seconds
If the connection fails, it attempts to reconnect to the WiFi and/or the MQTT Broker as needed.

Original Gerber files for the PCB layout and the source files for Kicad are included, although there may be issues with the custom libraries used.

**Software features:**

- Easy setup.
- Responsive design.
- Home Asisstant Discovery.
- All setup in web browser: Wifi, MQTT, Calibrate, timezone, update and system.
- Update Voltaje by MQTT topic.
- NTP to setup timezone.
- Blynk Integration
- ThingSpeak integration
- OTA updates
- Optional reset Kwh counter one day of the month
- Reset Kwh in MQTT Topic

#### Components:
- Wemos D1 Mini / D1 Mini Pro
- [ESP8266 Mains Current Sensor](https://www.ebay.es/itm/ESP8266-Mains-Current-Sensor-Wemos-Current-transformer-SCT013-100A-50mA/133077015640)
- Current transformer SCT013 (Ebay or Aliexpress)
- Optional case: https://www.thingiverse.com/thing:3544702

**WARNING:** If you use calibrated clamp, **remove resistance R1** from mains current sensor.

## Firmware installation and first setup

#### Flashing:

<img src="images/installation01.jpg" alt="installation01" align="right" width="200"/>

Download latest [release](https://github.com/alcar21/WemosEM/releases)
You can flash using software [nodemcu-pyflasher](https://github.com/marcelstoer/nodemcu-pyflasher/releases)

1. Select Serial port (COM# port) where your serial-to-USB or NodeMCU/D1 mini is connected. Leave on auto-select if not sure.
2. Browse to the binary you downloaded from WemosEM releases.
3. Set Baud rate to 115200 and **Flash mode to DOUT**. Erase flash to **yes** if it is the first time flashing WemosEM on the device or you're experiencing issues with existing flash. If you're upgrading WemosEM set to no.
4. Click Flash NodeMCU and watch the progress in console.

If the flash was successful the console will display:
  Firmware successfully flashed. unplug/replug or reset device to switch back to normal boot mode

Unplug your adapter or device and plug it back in or connect to another power source. Your device is now ready for Initial configuration.

You can see in Tasmota project other options to flash: https://github.com/arendst/Sonoff-Tasmota/wiki/Flashing

#### Initial configuration

WemosEM provides a wireless access point for easy Wi-Fi configuration.

<img src="images/installation02.jpg" alt="installation02" align="right" width="200"/>
<img src="images/installation03.jpg" alt="installation03" align="right" width="200"/>
<img src="images/installation04.jpg" alt="installation04" align="right" width="200"/>

Connect your device to a power source and grab your smartphone (or tablet or laptop or any other web and Wi-Fi capable device). Search for a Wi-Fi AP named **wemosDB-xxxxxx** (where x is a last part of MAC) and connect to it with password **infinito&masalla**. In this example the Wi-Fi AP is named **wemosDB-AB128A**. When it connects to the network, you may get a warning that there is no Internet connection and be prompted to connect to a different network. Do not allow the mobile device to select a different network.

After you have connected to the wemosDB Wi-Fi AP, open http://192.168.4.1 in a web browser on the smartphone (or whatever device you used). Some devices might prompt you to sign in to Wi-Fi network which should also open the above address.

At this page you can have wemosDB scan for available Wi-Fi networks. Select and setup you wifi network. Click save and you wemos will connect to your wifi.

<img src="images/installation05.jpg" alt="installation05" align="right" width="200"/>
Finally, connect with IP or http://wemosDB-xxxxxx.local/ and enter default user and password:

* **Default user: wemosem**
* **Default password: infinito&masalla**

Password is updatable in configuration system tab.

## Setup in Home Assistant (without MQTT autodiscovery)

    sensor:
    - platform: mqtt
      state_topic: "wemos/wemosEM-XXXXXX/power"
      name: WemosEM-Amperaje
      icon: mdi:current-ac
      unit_of_measurement: "A"
      value_template: "{{ value_json.current }}"
    - platform: mqtt
      state_topic: "wemos/wemosEM-XXXXXX/power"
      name: WemosEM-Consumo Actual
      icon: mdi:power-plug
      unit_of_measurement: "W"
      value_template: "{{ value_json.watios }}"
    - platform: mqtt
      state_topic: "wemos/wemosEM-XXXXXX/power"
      name: WemosEM-KWh
      icon: mdi:power-plug
      unit_of_measurement: "KWh"
      value_template: "{{ value_json.kwh }}"nt Cell  | Content Cell  |

## Update Voltaje with Home Assistant

if you have a Shelly EM, add a automation in Home assistant:

      - alias: Update voltage WemosEM
        initial_state: 'on'
        trigger:
        - platform: mqtt
          topic: shellies/shellyem-B9E2E9/emeter/0/voltage

        action:
        - service: mqtt.publish
          data_template:
            topic: "wemos-cmd/wemosEM-XXXXXX/voltage"
            payload: '{{ trigger.payload }}'

if you have a device with Tasmota, add a automation in Home assistant:

      - alias: Update voltage Tasmota-WemosEM
        initial_state: 'on'
        trigger:
        - platform: mqtt
          topic: tele/sonoff01/SENSOR

        action:
        - service: mqtt.publish
          data_template:
            topic: "wemos-cmd/wemosEM-XXXXXX/voltage"
            payload: "{{ trigger.payload_json['ENERGY'].Voltage }}"

## Blynk Integration

Go to Blynk app on your phone and layout the blynk project with virtual pin number according:
- Virtual PIN V0: Voltage (V)
- Virtual PIN V1: Current (A)
- Virtual PIN V2: Live Watios (w)
- Virtual PIN V3: Energy (kwh)
- Virtual PIN V4: Energy before last reset (kwh)

Setup in web interface token auth, optional you can setup server and port server.

## Thingspeak Integration
Setup in web interface token auth (Write API Key) and Channel number.
- Field 1: Voltage (V)
- Field 2: Current (A)
- Field 3: Live Watios (w)
- Field 4: Energy (kwh)
- Field 5: Energy before last reset (kwh)

## MQTT Topics
Topic  | Description
------------- | -------------
wemos-cmd/wemosEM-XXXXXX/voltage | Set/update voltage (from sonoff, shelly or other devices)
wemos-cmd/wemosDB-XXXXXX/status | Get Status of wemosDB
wemos-cmd/wemosDB-XXXXXX/resetkwh | reset kwh and store and beforeResetkwh variable

## OTA Updates
OTA updates from platformio windows with espota:
~/.platformio/packages/tool-espotapy/espota.py --progress --ip 192.168.4.1 --auth "infinito&masalla" --file wemosEM-1.3.1_core-2.4.bin

## Developers
This project is developed with **platformio**. WemosEM not compile with core 2.3 because blynk library require core 2.4.0 or higher.
- Recomended compile this project with Arduino Core 2.4.2.

**Commands:**
- Compile: pio run
- Compile and upload Wemos D1: pio run -t upload

Other options:
- Compile with core 2.5: pio run -t upload -e d1_mini_2_5
- Compile for Wemos D1 Mini PRO (**only core 2.4**): pio run -t upload -e d1_mini_pro_2_4

## Tests

## Screenshots

<img src="images/screenshot01.jpg" style="max-width: 100%;height: auto;" align="center" />
<img src="images/screenshot02.jpg" style="max-width: 100%;height: auto;" align="center" />
<img src="images/screenshot03.jpg" style="max-width: 100%;height: auto;" align="center" />
<img src="images/screenshot04.jpg" style="max-width: 100%;height: auto;" align="center" />
<img src="images/screenshot05.jpg" style="max-width: 100%;height: auto;" align="center" />
