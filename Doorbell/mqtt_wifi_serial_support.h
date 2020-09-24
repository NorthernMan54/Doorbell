/*
   MQTT, Wifi and serial functions

   Alfonso C. Alvarez (Alcar), 14nd September 2019

   @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

   Compile with Arduino 2.4.2
 */

// Function defined in power_meter_support.h
void resetKwh();
void tick();
// String buildHADiscovery();
// String buildHAStatus();

void setupOTA() {
        // Hostname defaults to esp8266-[ChipID]
        ArduinoOTA.setHostname(wifi_hostname.c_str());

        // No authentication by default
        // ArduinoOTA.setPassword((const char *)"123");

        ArduinoOTA.onStart([] () {
                Serial.println("[OTA] Start");
        });

        ArduinoOTA.onEnd([] () {
                Serial.println("\n[OTA] End");
        });

        ArduinoOTA.onProgress([] (unsigned int progress, unsigned int total) {
                Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
        });

        ArduinoOTA.onError([] (ota_error_t error) {

                Serial.printf("[OTA] Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

        ArduinoOTA.begin();
        Serial.println("[OTA] Arduino OTA Started");
}

boolean isSTA() {
        return (WiFi.isConnected() && WiFi.localIP() != IPAddress(192,168,4,1));
}

String WifiGetRssiAsQuality(int rssi)
{
        String qualityStr = "0% No signal";
        int quality = 2 * (rssi + 100);

        if (rssi <= -100) {
                qualityStr = "0% No signal";
        } else if (rssi >= -50) {
                qualityStr = "100% Excelent";
        } else if (rssi >= -60) {
                qualityStr = String(quality) + "% Good";
        } else if (rssi >= -70) {
                qualityStr = String(quality) + "% Poor";
        } else if (rssi > -80) {
                qualityStr = String(quality) + "% Bad";
        }  else {
                qualityStr = String(quality) + "% Very weak";
        }
        return qualityStr;
}

void callbackMqtt(char* topic, byte* payload, unsigned int length) {

        char topicWemosEM[25];

        Serial.print(" [MQTT] - Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
        }
        Serial.println();

        char* isTopic = strstr(topic, mqtt_topic_prefix_subscribe.c_str() );
        if (!isTopic) {
                return;
        }

        String command = String(strrchr(topic, '/') + 1);

        String s_payload = String((char *)payload);

        if(command.equals(TOPIC_VOLTAGE)) {

                Serial.println(" [MQTT] - Setting MQTT voltage " + s_payload);
                if (s_payload.length() > 0 && s_payload.toFloat() > 0) {
                        mainsVoltage = s_payload.toFloat();
                }
        } else if (command.equals(TOPIC_STATUS)) {

                Serial.println(" [MQTT] - Processing MQTT status ");
                mqtt_client.publish(mqtt_topic_status.c_str(), (char*) "online");
        } else if (command.equals(TOPIC_RESET_KWH)) {

                Serial.println(" [MQTT] - Processing MQTT reset KWH ");
                resetKwh();
        }

}

// Compose a payload to return as required
// The payload and headers cannot exceed 128 bytes!
String build_payload() {

        DynamicJsonDocument json(512);
        String jsonString;

        // Serial.println("build_payload");

        // WiFi signal strength in dB
        String rssi = WifiGetRssiAsQuality(WiFi.RSSI());

  #ifdef ENERGY
        json["current"] = String(rmsCurrent);
        json["voltage"] = String(mainsVoltage);
        json["watios"] = String(rmsPower);
        json["kwh"] = String(kiloWattHours);
        json["beforeKwh"] = String(beforeResetKiloWattHours);
        json["ical"] = String(Ical);
  #endif
  #ifdef DOORBELL
        // Serial.println(String(rmsCurrent));
        json["current"] = String(rmsCurrent);
        // Serial.println(String(mainsVoltage));
        json["voltage"] = String(mainsVoltage);
        // Serial.println(String(doorbellLength));
        json["doorbellLength"] = String(doorbellLength);
        // Serial.println(String(doorbellTime));
        json["doorbellTime"] = String(doorbellTime);
        // Serial.println(String(doorbellPressed));
        json["doorbellPressed"] = String(doorbellPressed);
  #endif
  #ifdef MOTION
        json["motion"] = String(motionAverage);
        json["temperature"] = String(temperature);
  #endif
        // Serial.println(String(reconnected_count));
        json["mqttreconnected"] = String(reconnected_count);
        // Serial.println(rssi);
        json["wifidb"] = rssi;
        // Serial.println(NTP.getUptimeString());
        json["uptime"] = NTP.getUptimeString();
        // Serial.println(NTP.getTimeDateString());
        json["time"] = NTP.getTimeDateString();
        // Serial.println(ESP.getFreeHeap());
        json["freemem"] = ESP.getFreeHeap();
        // Serial.println(VERSION);
        json["version"] = String(VERSION);

        json["devicename"] = deviceName;

        // Serial.println("serializeJson");
        serializeJson(json, jsonString);
        // Serial.println("serializeJson - complete");
        // Serial.println(jsonString);

        return jsonString;
}

// SERIAL FUNCTIONS
void initSerial() {

        Serial.begin(115200);
        delay(500);
        Serial.println("");
        Serial.println("");
        Serial.print("* Starting up *");
        Serial.println(VERSION);
}

void prepareHostMacAndEvents() {

        gotIpEventHandler = WiFi.onStationModeGotIP([] (const WiFiEventStationModeGotIP& event) {
                Serial.print("Station connected, IP: ");
                Serial.println(WiFi.localIP());
                ticker.detach();
                //keep LED on
                digitalWrite(Status_LED, LOW);
                wifiFirstConnected = true;

                wifi_name = WiFi.SSID();
                wifi_password =  WiFi.psk();
                ip = WiFi.localIP().toString();
                mask =WiFi.subnetMask().toString();
                gateway = WiFi.gatewayIP().toString();
        });

        disconnectedEventHandler = WiFi.onStationModeDisconnected([] (const WiFiEventStationModeDisconnected& event) {
                Serial.println("Station disconnected");
                wifiFirstConnected = false;
                pinMode(Status_LED, OUTPUT);          // Initialize Status LED
                ticker.attach(0.6, tick);
        });

        // Get MAC address of ESP8266, 6 bytes in an array
        byte mac[6];
        WiFi.macAddress(mac);

        My_MAC = "";
        // Build a string of the MAC with "0" padding for each byte, and upper case
        for (int i = 0; i <= 5; i++) {
                String B = String(mac[i], HEX);
                B.toUpperCase();
                if (B.length() < 2) {
                        // Pad with leading zero if needed
                        B = "0" + B;
                }
                My_MAC += B;
        } // End of for

        wifi_hostname = String(HOSTNAME_PREFIX);
        wifi_hostname.concat(My_MAC.substring(6, 12));

        if ( deviceName.isEmpty()) {
          deviceName = wifi_hostname;
        }

        WiFi.hostname(wifi_hostname.c_str());
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
        Serial.println("Entered config mode");
        Serial.println(WiFi.softAPIP());
        //if you used auto generated SSID, print it
        Serial.println(myWiFiManager->getConfigPortalSSID());
        //entered config mode, make led toggle faster
        ticker.attach(0.2, tick);
}

// Try to connect to any of the WiFi networks configured in Custom_Settings.h
void setupWifi() {

        //Set the wifi config portal to only show for 3 minutes, then continue.
        WiFi.hostname(wifi_hostname.c_str());
        WiFi.setSleepMode(WIFI_NONE_SLEEP);

        if (wifi_name.length() > 0 && wifi_password.length() > 0 ) {
                WiFi.mode(WIFI_STA);
                WiFi.begin(wifi_name.c_str(), wifi_password.c_str());
                delay(300);
        }

        wifiManager.setConfigPortalTimeout(180);
        wifiManager.setConnectTimeout(60);
        wifiManager.setAPCallback(configModeCallback);
        Serial.println(" Wifi " + wifi_hostname + ", password " + system_password);
        if (system_password.length() > 0 ) {
                wifiManager.autoConnect(wifi_hostname.c_str(), system_password.c_str());
        } else {
                wifiManager.autoConnect(wifi_hostname.c_str());
        }

        if (isSTA() && ipMode == 1) {
                IPAddress ipa_ip, ipa_gateway, ipa_subnet;
                if (ipa_ip.fromString(ip) && ipa_gateway.fromString(gateway) &&  ipa_subnet.fromString(mask)) {
                        // Static IP Setup
                        WiFi.config(ipa_ip, ipa_gateway, ipa_subnet);
                        Serial.println(" using Static IP " + String(ip));
                } else {
                        Serial.println("Error in static parameters, using DHCP");
                }
        } else {
                Serial.println(" using DHCP");
        }

}

void discoverHA() {

        char topic[60], message[400];
        char unique_id[60];

  #ifdef ENERGY
        // Current (A)
        Serial.println("Preparing topic HA Discover");
        sprintf_P(topic, TOPIC_HA_CURRENT, wifi_hostname.c_str() );
        Serial.println("Preparing message HA Discover");
        sprintf_P(message, MESSAGE_HA_CURRENT, wifi_hostname.c_str(), wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str() );
        mqtt_client.publish(topic, message, true);

        // Power (watios)
        sprintf_P(topic, TOPIC_HA_POWER, wifi_hostname.c_str() );
        sprintf_P(message, MESSAGE_HA_POWER, wifi_hostname.c_str(), wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str() );
        mqtt_client.publish(topic, message, true);

        // Power (kwh)
        sprintf_P(topic, TOPIC_HA_KWH, wifi_hostname.c_str() );
        sprintf_P(message, MESSAGE_HA_KWH, wifi_hostname.c_str(), wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str(),wifi_hostname.c_str() );
        mqtt_client.publish(topic, message, true);

  #endif



  #ifdef DOORBELL
        Serial.print("Preparing topic HA Discover -> ");
        sprintf_P(topic, TOPIC_HA_DOORBELL, wifi_hostname.c_str() );
        Serial.println(topic);
        Serial.print("Preparing message HA Discover -> ");

        sprintf_P(unique_id, "%s_doorbell", wifi_hostname.c_str() );

        sprintf_P(message, MESSAGE_HA_DOORBELL, deviceName.c_str(), mqtt_topic.c_str(),mqtt_topic_status.c_str(),wifi_hostname.c_str(), unique_id );
        Serial.println(message);
        mqtt_client.publish(topic, message, true);
  #endif

        Serial.print("Preparing topic HA Discover -> ");
        sprintf_P(topic, TOPIC_HA_STATUS, wifi_hostname.c_str() );
        Serial.println(topic);
        Serial.print("Preparing message HA Discover -> ");

        String statusDeviceName;
        statusDeviceName = String(deviceName + " status");
        sprintf_P(unique_id, "%s_status", wifi_hostname.c_str() );
        sprintf_P(message, MESSAGE_HA_STATUS, statusDeviceName.c_str(), mqtt_topic.c_str(),mqtt_topic_status.c_str(), unique_id, wifi_hostname.c_str(), wifi_hostname.c_str(), VERSION);
        Serial.println(message);
        mqtt_client.publish(topic, message, true);



}

void initMqtt() {

        String mqtt_topic_prefix = "wemos/" + wifi_hostname + "/";

        mqtt_topic_prefix_subscribe = "wemos-cmd/" + wifi_hostname;
        mqtt_topic_subscribe = mqtt_topic_prefix_subscribe + "/#";
        mqtt_topic = mqtt_topic_prefix + "state";
        mqtt_topic_status = mqtt_topic_prefix + "LWT";

        mqtt_client.setServer(mqtt_server.c_str(), mqtt_port);
        mqtt_client.setCallback(callbackMqtt);

        if (WiFi.status() != WL_CONNECTED) {
                // Attempt to connect
                setupWifi();
        }

        int lwQoS = 1; // send last will at least once
        int lwRetain = 1;
        String lwPayload = "offline";
        bool cleanSession = true;
        int connected = false;

        if (mqtt_enabled) {
                Serial.println("Connecting Mqtt...");
                if (mqtt_username.length() > 0 && mqtt_password.length() > 0) {
                        Serial.println("Connecting MQTT with user/pass");
                        connected = mqtt_client.connect(wifi_hostname.c_str(), (char*)mqtt_username.c_str(), (char*)mqtt_password.c_str(), mqtt_topic_status.c_str(), lwQoS, lwRetain, (char*)lwPayload.c_str(), cleanSession);
                } else {
                        connected = mqtt_client.connect(wifi_hostname.c_str(), mqtt_topic_status.c_str(), lwQoS, lwRetain, (char*)lwPayload.c_str());
                }

                ++reconnected_count;
                if (connected) {
                        Serial.println("MQTT Connected. ");
                        mqtt_client.subscribe((char *)mqtt_topic_subscribe.c_str());
                        // Discover Notify Home Assistant
                        discoverHA();
                        mqtt_client.publish(mqtt_topic_status.c_str(), (char*) "online", 0);
                } else {
                        Serial.println("failed, rc=" + String(mqtt_client.state()) + " Try again in 5 seconds");
                        // Wait 5 seconds before retrying
                        delay(5000);
                }

        } else {
                Serial.println("MQTT Connection is disabled...");
        }
}

bool mqtt_reconnect() {

        if (!mqtt_enabled) {
                return false;
        }
        Serial.print("Status MQTT: ");
        Serial.print(mqtt_client.state());
        Serial.println(" Reconnecting...");
        ++reconnected_count;

        // Loop until we're reconnected
        // (or until max retries)

        if (WiFi.status() != WL_CONNECTED) {
                // Attempt to connect
                setupWifi();
        }

        Serial.print(" Attempting MQTT Broker connection... ");
        int connected = false;

        int lwQoS = 1; // send last will at least once
        int lwRetain = 1;
        String lwPayload = "Offline";
        bool cleanSession = true;

        if (mqtt_username.length() > 0 && mqtt_password.length() > 0) {
                Serial.println("Reconnecting with user/pass");
                connected = mqtt_client.connect(wifi_hostname.c_str(), (char*)mqtt_username.c_str(), (char*)mqtt_password.c_str(), mqtt_topic_status.c_str(), lwQoS, lwRetain, (char*)lwPayload.c_str(), cleanSession);
        } else {
                connected = mqtt_client.connect(wifi_hostname.c_str(), mqtt_topic_status.c_str(), lwQoS, lwRetain, (char*)lwPayload.c_str());
        }

        if (connected) {
                Serial.println(" MQTT Connected. ");
                mqtt_client.subscribe((char *)mqtt_topic_subscribe.c_str());
                discoverHA();
        } else {
                Serial.println("failed, rc=" + String(mqtt_client.state()) + " Try again...");
        }

        return mqtt_client.connected();

} // End of reconnect

void tick()
{
        //toggle state
        int state = digitalRead(Status_LED); // get the current state of GPIO1 pin
        digitalWrite(Status_LED, !state); // set pin to the opposite state
}
