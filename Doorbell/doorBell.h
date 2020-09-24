#include "ACS712.h"

//ACS712  ACS;
// ACS712  ACS(A0, 3.3, 1023, 185);
ACS712 ACS(ACS_PIN, ACS_MAX_VOLTAGE, ACS_MAX_ADC, ACS_MV_PER_A);

int lastMa = 0;

int currentState = 0;
unsigned long currentStateStart = millis();
unsigned long lastAcsRead = millis();

void acs_setup() {
        ACS.autoMidPoint();
}

void acs_loop() {

        unsigned long now = millis();
        // Only sample once every 100 millis
        if (now - lastAcsRead < 50) {
                return;
        }
        lastAcsRead = now;
        int mA = ACS.mA_AC();

        rmsCurrent = mA;
        // After button has been pressed send a message with length of button press

        // Serial.print("mA: ");
        // Serial.println(mA);

        if ( mA >  500) { // Pressed
          if ( currentState == 1 ) { // already pressed
            return;
          } else {
            currentState = 1;
            currentStateStart = millis();
            return;
          }
        } else {  // not pressed

          if ( currentState == 1 ) { // already pressed

            doorbellLength = now - currentStateStart;
            doorbellPressed = "Yes";
            doorbellTime = NTP.getTimeDateString();
            currentState = 0;
          } else {  // Not pressed and no state change

            currentState = 0;
            doorbellPressed = "No";
            if (now - lastMsgMQTT < message_interval) {
              return;
            }
          }
        }

        mainsVoltage = analogRead(ACS_PIN) * 3.3 / 1024;
        lastMsgMQTT = now;
        String payload = build_payload();

        Serial.print(" [METER] - Payload: ");
        Serial.println(payload);

        if (!mqtt_enabled) {
                return;
        }

        // Check if we're still connected to MQTT broker
        if (!mqtt_client.connected()) {
                // reconnect if not
                initMqtt();
        }

        // Publish a MQTT message with the payload
        if (mqtt_client.publish(mqtt_topic.c_str(), (char*) payload.c_str(), 0)) {
  
        } else {
                Serial.print("ERROR MQTT Topic not Published: ");
                Serial.println(mqtt_topic);
        }
}
