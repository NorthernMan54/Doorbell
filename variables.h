
/*
  Global variables and constants

  Alfonso C. Alvarez (Alcar), 14nd September 2019

  @author <a href="mailto:alcar21@gmail.com">Alfonso Carlos Alvarez Reyes</a>

  Compile with Arduino 2.4.2
*/

// I/O
#define Status_LED 2 // D4
/*
#define Status_LED_On digitalWrite(Status_LED, HIGH)
#define Status_LED_Off digitalWrite(Status_LED, LOW)
*/
Ticker ticker;

#define MIN_READ_EM_INTERVAL 1000        // Minimum seconds read sensor Clamp of energy meter
#define NUM_SAMPLES 1480 // measured by Tom in Mui Wo

#define MAXLEN_MQTT_ENABLED 1
#define MAXLEN_MQTT_SERVER 100
#define MAXLEN_MQTT_PORT 5
#define MAXLEN_MQTT_USERNAME 30
#define MAXLEN_MQTT_PASSWORD 30
#define MAXLEN_SYSTEM_PASSWORD 30

#define MAXLEN_VOLTAGE 7
#define MAXLEN_ICAL 6
#define MAXLEN_TIMEZONE 2
#define MIN_MESSAGE_INTERVAL 5000
#define MAXLEN_MESSAGE_INTERVAL 7

#define HOSTNAME_PREFIX "wemosDB-"

#define TOPIC_VOLTAGE "voltage"
#define TOPIC_STATUS "status"
#define TOPIC_RESET_KWH "resetkwh"

// BLYNK Virtual Pins
#define VPIN_VOLTAGE V0
#define VPIN_CURRENT V1
#define VPIN_WATIOS V2
#define VPIN_ENERGY V3
#define VPIN_ENERGY_LAST_RESET V4

// THINGSPEAK
#define TIME_BEETWEEN_REQUEST 15000 //15 seconds

// Device Configuration, uncomment only one

// #define ENERGY           // Energy Monitor
// #define MOTION           // MPU6050 and Energy Monitor
#define DOORBELL         // Current sensing door bell monitor

const char TOPIC_HA_KWTOTAL[] PROGMEM = "homeassistant/sensor/%s/kwTotal/config";
const char VERSION[] PROGMEM = "WemosDB 1.3";
const char TOPIC_HA_STATUS[] PROGMEM = "homeassistant/sensor/%s_status/config";
#ifdef MOTION
const PROGMEM char MODEL[] = "ESP8266 + SCT013 + MPU6050";
#endif

#ifdef ENERGY
const PROGMEM char TOPIC_HA_CURRENT[] = "homeassistant/sensor/%s/current/config";
const PROGMEM char TOPIC_HA_POWER[] = "homeassistant/sensor/%s/power/config";
const PROGMEM char TOPIC_HA_KWH[] = "homeassistant/sensor/%s/kwh/config";
const PROGMEM char MODEL[] = "ESP8266 + SCT013";
const PROGMEM char MESSAGE_HA_CURRENT[] = "{\"unit_of_measurement\":\"A\",\"state_topic\":\"wemos/%s/power\",\"value_template\":\"{{ value_json.current }}\",\"json_attributes_topic\":\"wemos/%s/power\",\"name\":\"%s-current\",\"unique_id\":\"%s-current-wemosem\",\"device\":{\"identifiers\":[\"%s\"],\"name\":\"%s\",\"sw_version\":\"WemosEM 1.2\",\"model\":\"Wemos D1 Mini + SCT013\",\"manufacturer\":\"Wemos\"},\"icon\":\"mdi:counter\",\"availability_topic\":\"wemos/%s/status\"}";
const PROGMEM char MESSAGE_HA_POWER[] = "{\"unit_of_measurement\":\"w\",\"state_topic\":\"wemos/%s/power\",\"device_class\":\"power\",\"value_template\":\"{{ value_json.watios }}\",\"json_attributes_topic\":\"wemos/%s/power\",\"name\":\"%s-watios\",\"unique_id\":\"%s-watios-wemosem\",\"device\":{\"identifiers\":[\"%s\"],\"name\":\"%s\",\"sw_version\":\"WemosEM 1.2\",\"model\":\"Wemos D1 Mini + SCT013\",\"manufacturer\":\"Wemos\"},\"icon\":\"mdi:counter\",\"availability_topic\":\"wemos/%s/status\"}";
const PROGMEM char MESSAGE_HA_KWH[] = "{\"unit_of_measurement\":\"kwh\",\"state_topic\":\"wemos/%s/power\",\"device_class\":\"power\",\"value_template\":\"{{ value_json.kwh }}\",\"json_attributes_topic\":\"wemos/%s/power\",\"name\":\"%s-kwh\",\"unique_id\":\"%s-kwh-wemosem\",\"device\":{\"identifiers\":[\"%s\"],\"name\":\"%s\",\"sw_version\":\"WemosEM 1.2\",\"model\":\"Wemos D1 Mini + SCT013\",\"manufacturer\":\"Wemos\"},\"icon\":\"mdi:counter\",\"availability_topic\":\"wemos/%s/status\"}";
// const PROGMEM char MESSAGE_HA_KWTOTAL[] = "{\"unit_of_measurement\":\"kwTotal\",\"state_topic\":\"wemos/%s/power\",\"device_class\":\"power\",\"value_template\":\"{{ value_json.kwTotal }}\",\"json_attributes_topic\":\"wemos/%s/power\",\"name\":\"%s-kwTotal\",\"unique_id\":\"%s-kwTotal-wemosem\",\"device\":{\"identifiers\":[\"%s\"],\"name\":\"%s\",\"sw_version\":\"WemosEM 1.1\",\"model\":\"Wemos D1 Mini + SCT013\",\"manufacturer\":\"Wemos\"},\"icon\":\"mdi:counter\",\"availability_topic\":\"wemos/%s/status\"}";
#endif

#ifdef DOORBELL
const PROGMEM char MODEL[] = "ESP8266 + ACS712";
const PROGMEM char TOPIC_HA_DOORBELL[] = "homeassistant/binary_sensor/%s/config";
const PROGMEM char MESSAGE_HA_DOORBELL[] = "{\"name\": \"%s\", \
\"stat_t\": \"%s\", \
\"avty_t\":\"%s\", \
\"pl_avail\":\"online\", \
\"pl_not_avail\":\"offline\", \
\"payload_on\":\"Yes\", \
\"payload_off\":\"No\", \
\"device_class\":\"doorbell\", \
\"value_template\":\"{{ value_json.doorbellPressed }}\", \
\"device\":{\"identifiers\":[\"%s\"]}, \
\"off_delay\": 10, \
\"unique_id\":\"%s\"}";
#endif

const PROGMEM char MESSAGE_HA_STATUS[] = "{\"name\": \"%s\", \
\"stat_t\": \"%s\", \
\"avty_t\": \"%s\", \
\"pl_avail\": \"online\", \
\"pl_not_avail\": \"offline\", \
\"val_tpl\": \"{{value_json['RSSI']}}\", \
\"uniq_id\": \"%s\", \
\"device\": {\"identifiers\":[\"%s\"],\"name\":\"%s\",\"model\":\"ESP8266 Doorbell\",\"sw_version\":\"%s\",\"manufacturer\":\"NorthernMan54\"} \
}";

// WIFI AND MQTT
WiFiClient espClient;                  // For ESP8266 boards
PubSubClient mqtt_client(espClient);
WiFiManager wifiManager;
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;  // Event IP Handler
unsigned long uptimeAP = 0;

// UPDATE FIRMWARE
const char* update_path = "/WebFirmwareUpgrade";

// TIME ZONE
//const PROGMEM char ntpServer[] = "pool.ntp.org";
const char *ntpServer = "pool.ntp.org";

// Message timing
int reconnected_count = 0;            // Counter of messages sent
long ms_since_last_message = 0;       // Time elapsed since last message was sent, in ms

// Energy Monitor
EnergyMonitor emon;

// Data of Energy Monitor
double watiosTotal = 0;               // Measured energy in watios/h
double kiloWattHours = 0;             // Measured energy in KWh
double beforeResetKiloWattHours = 0;  // Measured energy in KWh
double rmsCurrent = 0;                // Measured current in A
double rmsPower = 0;                  // Measured power in W
unsigned long lastTimeMeasure = 0;    // Last time measure in ms
boolean isResetEnergyToday = false;    // reset energy on setup day

// MPU 6050 Motion sensor
double motion = 0;
double motionAverage = 0;
double temperature = 0;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// ACS712 Doorbell current sensor
unsigned long doorbellLength = 0;   // Length of door bell press in millis
String doorbellTime = "";
String doorbellPressed = "No";

// ThingSpeak
unsigned long lastTimeMeasureTS = 0;

// NTP
boolean syncEventTriggered = false;   // True if a time even has been triggered
NTPSyncEvent_t ntpEvent;              // Last triggered event

// Web Server
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

String My_MAC = "";                   // MAC address, to be read from ESP8266
String wifi_hostname;
String mqtt_topic = "None";
String mqtt_topic_status = "None";
String mqtt_topic_prefix_subscribe = "";
String mqtt_topic_subscribe = "";
bool wifiFirstConnected = false;

bool resetESP = false;
bool restartESP = false;
