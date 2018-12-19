

#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "FS.h"

#include "ESPAsyncWebServer.h"

/**********************/
/**** DEFINE PINS *****/
/**********************/

int RELAY_HEAT = 25;
int RELAY_COOL = 26;
int RELAY_FAN = 27;
int RELAY_HUMIDIFIER = 14;
int DHT_DATA_PIN = 13;



/**************************/
/***** SPIFFS FILES *****/
/************************/

const char *GPIO_FILE = "/gpio_conifg.json";
const char *CONNECT_FILE = "/connect_config.json";
const char *OPERATION_FILE = "/operation_config.json";

#define DHTTYPE DHT22
DHT *dht;  // define DHT object

#define STATUS_UPDATE_INTERVAL 60000

/***************************/
/**** DEFINE VARIABLES *****/
/***************************/

long MIN_RUNTIME = 300;
long MIN_UPDATE = 60;
long SENSOR_UPDATE = 10;
float CURRENT_TEMP;
float CURRENT_HUMIDITY;
int TARGET_COOL_TEMP = 76;
int TARGET_HEAT_TEMP = 70;
int TARGET_HUMIDITY = 40;
int TARGET_HEAT_AWAY_TEMP = 68;
int TARGET_COOL_AWAY_TEMP = 76;
int TARGET_HOLD_TEMP = 70;
float SWING_TEMP= 0.5;
int SWING_HUMIDITY = 2;
bool AWAY_MODE = false;
bool HOLD_MODE = false;
bool HUMIDIFIER = false;
int CURRENT_MODE = 0; // 0 = off, 1 = heat, 2 = cool
String MODE_OFF = "off";
String MODE_HEAT = "heat";
String MODE_COOL = "cool";
String MODE_ARRAY[] = {MODE_OFF, MODE_HEAT, MODE_COOL};
String FAN_AUTO = "auto";
String FAN_ON = "on";
String TOPIC_NAME = "HVAC";
int FAN_MODE = 0; // 0 = auto, 1 = on
String FAN_ARRAY[] = {FAN_AUTO, FAN_ON};

bool LOW_TRIGGER = true;

const char *CONF_COOL_TEMP = "cool_temp";
const char *CONF_HEAT_TEMP = "heat_temp";
const char *CONF_COOL_AWAY_TEMP = "cool_away_temp";
const char *CONF_HEAT_AWAY_TEMP = "heat_away_temp";
const char *CONF_MODE = "mode";
const char *CONF_FAN = "fan";
const char *CONF_HOLD = "hold";
const char *CONF_AWAY = "away";
const char *CONF_HUMIDITY = "humidity";
const char *CONF_HOLD_TEMP = "hold_temp";
const char *CONF_HUMIDIFIER = "humidifier";
const char *CONF_NAME = "name";
const char *CONF_SSID = "ssid";
const char *CONF_PASSWORD = "password";
const char *CONF_MQTT_SERVER = "mqtt_server";
const char *CONF_MQTT_PORT = "mqtt_port";
const char *CONF_MQTT_USER = "mqtt_user";
const char *CONF_MQTT_PASS = "mqtt_pass";
const char *CONF_RUNTIME = "runtime";
const char *CONF_SWING_TEMP = "swing_temp";
const char *CONF_SWING_HUMIDITY = "swing_humidity";
const char *CONF_SENSOR = "sensor";
const char *CONF_MIN_UPDATE = "min_update";
const char *CONF_RELAY_HEAT = "relay_heat";
const char *CONF_RELAY_COOL = "relay_ac";
const char *CONF_RELAY_FAN = "relay_fan";
const char *CONF_RELAY_HUMIDIFIER = "relay_humidifier";
const char *CONF_DHT22_PIN = "pin_dht22";
const char *CONF_LOW_TRIGGER = "low_trigger";

char GENERAL_SUBSCRIBE_TOPIC[27];
char STATUS_TOPIC[32];
char TARGETTEMP_TOPIC[36];
char CURRENTTEMP_TOPIC[30];
char CURRENTHUMIDITY_TOPIC[34];
char TARGETHUMIDITY_TOPIC[40];
char CURRENTMODE_TOPIC[30];
char AWAY_TOPIC[30];
char HOLD_TOPIC[30];
char FAN_TOPIC[29];
char HUMIDIFIER_TOPIC[35];
char RELAY_HEAT_TOPIC[36];
char RELAY_COOL_TOPIC[36];
char RELAY_FAN_TOPIC[35];
char RELAY_HUMIDIFIER_TOPIC[40];
char *MQTT_COMMAND_PREFIX = "cmnd";
char *MQTT_RESPONSE_PREFIX = "resp";

char PUBLISH_STATUS_TOPIC[32];
char PUBLISH_SENSOR_TOPIC[32];
char PUBLISH_RELAY_TOPIC[31];

// Update these with values suitable for your network.

String ssid = "";
String password = "";
String mqtt_server = "";
int mqtt_port = 1883;
String mqtt_user = "";
String mqtt_pass = "";

// Setup WIFI, MQTT, and WebServer Client
WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);

long LAST_RECONNECT_ATTEMPT = 0;
long LAST_UPDATE = 0;
long LAST_RUNTIME = 0;
long LAST_SENSOR_READ = 0;
long LAST_STATUS_UPDATE = 0;

// DHT dht(DHT_DATA_PIN, DHTTYPE); // SET UP THE DHT22
// DHT dht;


const char *mainhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;font-family:Arial,Helvetica,Sans-Serif;Color:#000}h1,h2,h3{text-align:center}td{text-align:center;width:50%%}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}.alert{background-color:red}.hvac-table{width:80%%;margin:0 auto;padding-bottom:40px}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Current data</h3> <table class=hvac-table> <tr> <td>Mode:<td>%CURRENT_MODE%<tr> <td>Target:<td>%TARGET_TEMP% F<tr> <td>Currently:<td>%CURRENT_TEMP% F<tr> <td>Away mode:<td>%AWAY_MODE%<tr> <td>Hold mode:<td>%HOLD_MODE%<tr> <td>Target Humidity:<td>%TARGET_HUMIDITY% %%<tr> <td>Current Humidity:<td>%CURRENT_HUMIDITY% %% </table> <form><input class=hvac-button onclick='window.location.href=\"operation\"' type=button value=\"OPERATIONS\"></form> <form><input class=hvac-button onclick='window.location.href=\"connectivity\"' type=button value=\"CONNECTIVITY\"></form> <form><input class=hvac-button onclick='window.location.href=\"relay\"' type=button value=\"RELAYS/SENSOR\"></form> <form action=/reset method=POST><input class=\"hvac-button alert\" onclick='return confirm(\"Do you want to reset back to factory settings?\")' type=submit value=\"FACTORY RESET\" name=reset></form>";
const char *connectivityhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{width:80%%;margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;width:50%%;padding:10px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Connectivity Settings</h3> <form action=connectivity/set method=POST> <table> <tr> <td>SSID:<td><input value=\"%SSID%\" name=ssid> <tr> <td>Password:<td><input value=\"%PASSWORD%\" name=password type=password> <tr> <td>MQTT Server:<td><input value=\"%MQTT_SERVER%\" name=mqtt_server> <tr> <td>MQTT Port:<td><input value=\"%MQTT_PORT%\" name=mqtt_port> <tr> <td>MQTT Username:<td><input value=\"%MQTT_USER%\" name=mqtt_user> <tr> <td>MQTT Password:<td><input value=\"%MQTT_PASS%\" name=mqtt_pass type=password> <tr> <td>Topic Name<td><input value=\"%NAME%\" name=name> </table><input value=\"SAVE SETTINGS\" type=submit class=hvac-button> </form> <form><input value=\"MAIN MENU\" type=button class=hvac-button onclick='window.location.href=\"/\"'></form> <h4>*Topic Name is used in the mqtt topics</h4>";
const char *operationhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;padding:10px}input{text-align:center}span{padding-left:30px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Operation Settings</h3> <form action=operation/set method=POST> <table> <tr> <td>Mode:<td colspan=2><span><input type=radio name=mode value=0 %off_mode%>Off</span> <span><input type=radio name=mode value=1 %heat_mode%>Heat</span> <span><input type=radio name=mode value=2 %cool_mode%>Cool</span> <tr><td>Fan:<td><span><input type=radio name=fan value=0 %fan_auto%>Auto</span><span><input type=radio name=fan value=1 %fan_on%>On</span> <tr> <td>Hold:<td><input type=checkbox name=hold %HOLD_CHECK%> <tr> <td>Away:<td><input type=checkbox name=away %AWAY_CHECK%> <tr> <td>Humidifier:<td><input type=checkbox name=humidifier %HUMIDIFIER_CHECK%> <tr> <td> <th>Heat<th>Cool<tr> <td>Target:<td><input type=number name=heat_target value=\"%HEAT_TARGET%\"> <td><input type=number name=cool_target value=\"%COOL_TARGET%\"> <tr> <td>Away Target:<td><input type=number name=heat_away_target value=\"%HEAT_AWAY_TARGET%\"> <td><input type=number name=cool_away_target value=\"%COOL_AWAY_TARGET%\"> <tr> <td>Hold Target:<td><input type=number name=hold_target value=\"%HOLD_TARGET%\"> <tr> <td>Swing Temp:<td><input type=number name=swing_temp value=\"%SWING_TEMP%\" step=0.5> <tr> <td>Swing Humidity:<td><input type=number name=swing_humidity value=\"%SWING_HUMIDITY%\"> <tr> <td>Min Runtime (sec):<td><input type=number name=min_runtime value=\"%MIN_RUNTIME%\"><td>Minimum time furnace/ac will run <tr> <td>Min Receive (sec):<td><input type=number name=min_update value=\"%MIN_UPDATE%\"><td>Minimum time between recieving remote temp to switch to local <tr> <td>Sensor Update (sec):<td><input type=number name=sensor_update value=\"%SENSOR_UPDATE%\"><td>How often to take/publish local reading </table><input type=submit value=\"SAVE SETTINGS\" class=hvac-button> </form> <form><input type=button value=\"MAIN MENU\" class=hvac-button onclick='window.location.href=\"/\"'></form>";
const char *relayhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;padding:10px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Relay/Sensor Settings</h3> <form action=relay/set method=POST> <table> <tr> <td>Low Trigger:<td><input type=\"checkbox\" name=\"low_trigger\" %LOW_TRIGGER%> <tr> <td>Heat:<td><select name=heat_relay> <option value=0 %heat_relay0%>GPIO 0<option value=2 %heat_relay2%>GPIO 2<option value=4 %heat_relay4%>GPIO 4<option value=5 %heat_relay5%>GPIO 5<option value=12 %heat_relay12%>GPIO 12<option value=13 %heat_relay13%>GPIO 13<option value=14 %heat_relay14%>GPIO 14<option value=15 %heat_relay15%>GPIO 15<option value=16 %heat_relay16%>GPIO 16 </select> <tr> <td>Cool:<td><select name=cool_relay> <option value=0 %cool_relay0%>GPIO 0<option value=2 %cool_relay2%>GPIO 2<option value=4 %cool_relay4%>GPIO 4<option value=5 %cool_relay5%>GPIO 5<option value=12 %cool_relay12%>GPIO 12<option value=13 %cool_relay13%>GPIO 13<option value=14 %cool_relay14%>GPIO 14<option value=15 %cool_relay15%>GPIO 15<option value=16 %cool_relay16%>GPIO 16 </select> <tr> <td>Fan:<td><select name=fan_relay> <option value=0 %fan_relay0%>GPIO 0<option value=2 %fan_relay2%>GPIO 2<option value=4 %fan_relay4%>GPIO 4<option value=5 %fan_relay5%>GPIO 5<option value=12 %fan_relay12%>GPIO 12<option value=13 %fan_relay13%>GPIO 13<option value=14 %fan_relay14%>GPIO 14<option value=15 %fan_relay15%>GPIO 15<option value=16 %fan_relay16%>GPIO 16 </select> <tr> <td>Humidifier:<td><select name=humidifier_relay> <option value=0 %humidifier_relay0%>GPIO 0<option value=2 %humidifier_relay2%>GPIO 2<option value=4 %humidifier_relay4%>GPIO 4<option value=5 %humidifier_relay5%>GPIO 5<option value=12 %humidifier_relay12%>GPIO 12<option value=13 %humidifier_relay13%>GPIO 13<option value=14 %humidifier_relay14%>GPIO 14<option value=15 %humidifier_relay15%>GPIO 15<option value=16 %humidifier_relay16%>GPIO 16 </select> <tr> <td>DHT22:<td><select name=dht22_pin> <option value=0 %dht_pin0%>GPIO 0<option value=2 %dht_pin2%>GPIO 2<option value=4 %dht_pin4%>GPIO 4<option value=5 %dht_pin5%>GPIO 5<option value=12 %dht_pin12%>GPIO 12<option value=13 %dht_pin13%>GPIO 13<option value=14 %dht_pin14%>GPIO 14<option value=15 %dht_pin15%>GPIO 15 </select> </table><input class=hvac-button type=submit value=\"SAVE SETTINGS\"> </form> <form><input class=hvac-button type=button value=\"MAIN MENU\" onclick='window.location.href=\"/\"'></form>";
const char *reboothtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;font-family:Arial,Helvetica,Sans-Serif;Color:#000}h1,h2,h3{text-align:center}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Settings have been saved. The unit will reboot and try to connect to %SSID%. Look for it there.</h3>";
const char *clearedhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;font-family:Arial,Helvetica,Sans-Serif;Color:#000}h1,h2,h3{text-align:center}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Setting have been cleared.  The unit reboot and broadcast an access point</h3>";

String processor(const String &var)
{

  // Serial.println(var);

  if (var == "")
  {
    return "%";
  }

  if (var.equalsIgnoreCase("OFF_MODE") && CURRENT_MODE == 0)
  {
    return "checked";
  }
  else if (var.equalsIgnoreCase("HEAT_MODE") && CURRENT_MODE == 1)
  {
    return "checked";
  }
  else if (var.equalsIgnoreCase("COOL_MODE") && CURRENT_MODE == 2)
  {
    return "checked";
  }
  if (var.equalsIgnoreCase("FAN_AUTO") && FAN_MODE == 0)
  {
    return "checked";
  }
  if (var.equalsIgnoreCase("FAN_ON") && FAN_MODE == 1)
  {
    return "checked";
  }
  else if (var.equalsIgnoreCase("SSID"))
  {
    return ssid;
  }
  else if (var.equalsIgnoreCase("PASSWORD"))
  {
    return password;
  }
  else if (var.equalsIgnoreCase("CURRENT_MODE"))
  {
    return MODE_ARRAY[CURRENT_MODE];
  }
  else if (var.equalsIgnoreCase("TARGET_TEMP"))
  {
    if (HOLD_MODE)
    {
      return String(TARGET_HOLD_TEMP);
    }
    else
    {
      if (AWAY_MODE)
      {
        if (CURRENT_MODE == 1)
        { //heating
          return String(TARGET_HEAT_AWAY_TEMP);
        }
        else if (CURRENT_MODE == 2)
        { //cooling
          return String(TARGET_COOL_AWAY_TEMP);
        }
      }
      else
      {
        if (CURRENT_MODE == 1)
        { //heating
          return String(TARGET_HEAT_TEMP);
        }
        else if (CURRENT_MODE == 2)
        { //cooling
          return String(TARGET_COOL_TEMP);
        }
      }
    }
  }
  else if (var.equalsIgnoreCase("CURRENT_TEMP"))
  {
    return String(CURRENT_TEMP);
  }
  else if (var.equalsIgnoreCase("AWAY_MODE"))
  {
    return AWAY_MODE ? "True" : "False";
  }
  else if (var.equalsIgnoreCase("HOLD_MODE"))
  {
    return HOLD_MODE ? "True" : "False";
  }
  else if (var.equalsIgnoreCase("TARGET_HUMIDITY"))
  {
    return String(TARGET_HUMIDITY);
  }
  else if (var.equalsIgnoreCase("CURRENT_HUMIDITY"))
  {
    return String(CURRENT_HUMIDITY);
  }
  else if (var.equalsIgnoreCase("NAME"))
  {
    return TOPIC_NAME;
  }
  else if (var.equalsIgnoreCase("MQTT_SERVER"))
  {
    return mqtt_server;
  }
  else if (var.equalsIgnoreCase("MQTT_USER"))
  {
    return mqtt_user;
  }
  else if (var.equalsIgnoreCase("MQTT_PORT"))
  {
    return String(mqtt_port);
  }
  else if (var.equalsIgnoreCase("MQTT_PASS"))
  {
    return mqtt_pass;
  }
  else if (var.equalsIgnoreCase("HEAT_TARGET"))
  {
    return String(TARGET_HEAT_TEMP);
  }
  else if (var.equalsIgnoreCase("COOL_TARGET"))
  {
    return String(TARGET_COOL_TEMP);
  }
  else if (var.equalsIgnoreCase("HEAT_AWAY_TARGET"))
  {
    return String(TARGET_HEAT_AWAY_TEMP);
  }
  else if (var.equalsIgnoreCase("COOL_AWAY_TARGET"))
  {
    return String(TARGET_COOL_AWAY_TEMP);
  }
  else if (var.equalsIgnoreCase("HOLD_TARGET"))
  {
    return String(TARGET_HOLD_TEMP);
  }
  else if (var.equalsIgnoreCase("SWING_TEMP"))
  {
    return String(SWING_TEMP);
  }
  else if (var.equalsIgnoreCase("SWING_HUMIDITY"))
  {
    return String(SWING_HUMIDITY);
  }
  else if (var.equalsIgnoreCase("MIN_RUNTIME"))
  {
    return String(MIN_RUNTIME);
  }
  else if (var.equalsIgnoreCase("MIN_UPDATE"))
  {
    return String(MIN_UPDATE);
  }
  else if (var.equalsIgnoreCase("SENSOR_UPDATE"))
  {
    return String(SENSOR_UPDATE);
  }
  else if (var.equalsIgnoreCase("HOLD_CHECK") && HOLD_MODE)
  {
    return "checked";
  }
  else if (var.equalsIgnoreCase("AWAY_CHECK") && AWAY_MODE)
  {
    return "checked";
  }
  else if (var.equalsIgnoreCase("HUMIDIFIER_CHECK") && HUMIDIFIER)
  {
    return "checked";
  }
  else if (var.equalsIgnoreCase("LOW_TRIGGER") && LOW_TRIGGER)
  {
    return "checked";
  }
  else if (var.substring(0, 10).equalsIgnoreCase("heat_relay") && var.substring(10).toInt() == RELAY_HEAT)
  {
    return "selected";
  }
  else if (var.substring(0, 10).equalsIgnoreCase("cool_relay") && var.substring(10).toInt() == RELAY_COOL)
  {
    return "selected";
  }
  else if (var.substring(0, 9).equalsIgnoreCase("fan_relay") && var.substring(9).toInt() == RELAY_FAN)
  {
    return "selected";
  }
  else if (var.substring(0, 16).equalsIgnoreCase("humidifier_relay") && var.substring(16).toInt() == RELAY_HUMIDIFIER)
  {
    return "selected";
  }
  else if (var.substring(0, 7).equalsIgnoreCase("dht_pin") && var.substring(7).toInt() == DHT_DATA_PIN)
  {
    return "selected";
  }

  return String();
}

bool loadGPIO_config()
{
  File configFile = SPIFFS.open(GPIO_FILE, "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject(buf.get());

  if (!json.success())
  {
    Serial.println("Failed to parse config file");
    return false;
  }

    json.prettyPrintTo(Serial);
  Serial.println();

  //save variables
  RELAY_COOL = json[CONF_RELAY_COOL];
  RELAY_FAN = json[CONF_RELAY_FAN];
  RELAY_HEAT = json[CONF_RELAY_HEAT];
  RELAY_HUMIDIFIER = json[CONF_RELAY_HUMIDIFIER];
  DHT_DATA_PIN = json[CONF_DHT22_PIN];
  LOW_TRIGGER = json[CONF_LOW_TRIGGER];

  return true;
}

bool loadCONNECT_config()
{
  File configFile = SPIFFS.open(CONNECT_FILE, "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject(buf.get());

  if (!json.success())
  {
    Serial.println("Failed to parse config file");
    return false;
  }

    json.prettyPrintTo(Serial);
  Serial.println();

  //save variables
  ssid = json[CONF_SSID].as<String>();
  password = json[CONF_PASSWORD].as<String>();
  mqtt_server = json[CONF_MQTT_SERVER].as<String>();
  mqtt_port = json[CONF_MQTT_PORT];
  mqtt_user = json[CONF_MQTT_USER].as<String>();
  mqtt_pass = json[CONF_MQTT_PASS].as<String>();
  TOPIC_NAME = json[CONF_NAME].as<String>();

  return true;
  
}

bool loadOPERATION_config()
{
  File configFile = SPIFFS.open(OPERATION_FILE, "r");
  if (!configFile)
  {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<300> jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject(buf.get());

  if (!json.success())
  {
    Serial.println("Failed to parse config file");
    return false;
  }

    json.prettyPrintTo(Serial);
  Serial.println();

  TARGET_COOL_AWAY_TEMP = json[CONF_COOL_AWAY_TEMP]; 
  TARGET_COOL_TEMP = json[CONF_COOL_TEMP];
  TARGET_HEAT_AWAY_TEMP = json[CONF_HEAT_AWAY_TEMP];
  TARGET_HEAT_TEMP = json[CONF_HEAT_TEMP];
  TARGET_HOLD_TEMP = json[CONF_HOLD_TEMP];
  TARGET_HUMIDITY = json[CONF_HUMIDITY];
  CURRENT_MODE = json[CONF_MODE];
  FAN_MODE = json[CONF_FAN];
  HOLD_MODE = json[CONF_HOLD];
  AWAY_MODE = json[CONF_AWAY];
  HUMIDIFIER = json[CONF_HUMIDIFIER];
  MIN_RUNTIME = json[CONF_RUNTIME];
  SENSOR_UPDATE = json[CONF_SENSOR];
  MIN_UPDATE = json[CONF_MIN_UPDATE];
  SWING_TEMP = json[CONF_SWING_TEMP];
  SWING_HUMIDITY = json[CONF_SWING_HUMIDITY];
  return true;
}

bool saveGPIO_config()
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json[CONF_RELAY_COOL] = RELAY_COOL;
  json[CONF_RELAY_HEAT] = RELAY_HEAT;
  json[CONF_RELAY_FAN] = RELAY_FAN;
  json[CONF_RELAY_HUMIDIFIER] = RELAY_HUMIDIFIER;
  json[CONF_DHT22_PIN] = DHT_DATA_PIN;
  json[CONF_LOW_TRIGGER] = LOW_TRIGGER;

    json.prettyPrintTo(Serial);
  Serial.println();

  File configFile = SPIFFS.open(GPIO_FILE, "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}

bool saveCONNECT_config()
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json[CONF_SSID] = ssid;
  json[CONF_PASSWORD] = password;
  json[CONF_MQTT_SERVER] = mqtt_server;
  json[CONF_MQTT_PORT] = mqtt_port;
  json[CONF_MQTT_USER] = mqtt_user;
  json[CONF_MQTT_PASS] = mqtt_pass;
  json[CONF_NAME] = TOPIC_NAME;

    json.prettyPrintTo(Serial);
  Serial.println();

  File configFile = SPIFFS.open(CONNECT_FILE, "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}

bool saveOPERATION_config()
{
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  json[CONF_COOL_AWAY_TEMP] = TARGET_COOL_AWAY_TEMP; 
  json[CONF_COOL_TEMP] = TARGET_COOL_TEMP;
  json[CONF_HEAT_AWAY_TEMP] = TARGET_HEAT_AWAY_TEMP;
  json[CONF_HEAT_TEMP] = TARGET_HEAT_TEMP;
  json[CONF_HOLD_TEMP] = TARGET_HOLD_TEMP;
  json[CONF_HUMIDITY] = TARGET_HUMIDITY;
  json[CONF_MODE] = CURRENT_MODE;
  json[CONF_FAN] = FAN_MODE;
  json[CONF_HOLD] = HOLD_MODE;
  json[CONF_AWAY] = AWAY_MODE;
  json[CONF_HUMIDIFIER] = HUMIDIFIER;
  json[CONF_RUNTIME] = MIN_RUNTIME;
  json[CONF_SENSOR] = SENSOR_UPDATE;
  json[CONF_MIN_UPDATE] = MIN_UPDATE;
  json[CONF_SWING_TEMP] = SWING_TEMP;
  json[CONF_SWING_HUMIDITY] = SWING_HUMIDITY;

  json.prettyPrintTo(Serial);
  Serial.println();

  File configFile = SPIFFS.open(OPERATION_FILE, "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.printTo(configFile);
  return true;
}

void setup_wifi()
{

  if (ssid.length() > 0)
  {
    int timeout = 0;
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED && timeout < 30)
    {
      timeout++;
      delay(500);
      Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.disconnect(true);
    char apssid[15];                     //Create a Unique AP from MAC address
    snprintf(apssid, 15, "HVAC-%04X", ESP.getChipId());
    WiFi.softAP(apssid);
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }
}

boolean reconnect()
{
  char mqttid[15];                     //Create a Unique AP from MAC address
  snprintf(mqttid, 15, "HVAC-%04X", ESP.getChipId());
  if (client.connect(mqttid, mqtt_user.c_str(), mqtt_pass.c_str()))
  {
    client.subscribe(GENERAL_SUBSCRIBE_TOPIC);
  }
  return client.connected();
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  payload[length] = '\0'; // Make payload a string by NULL terminating it.
  int intPayload = String((char *)payload).toInt();
  float floatPayload = String((char *)payload).toFloat();
  String stringPayload = String((char *)payload);
  String newTopic = topic;

  if (newTopic.equalsIgnoreCase(STATUS_TOPIC) && length == 0)
  {
    status_update();
    return;
  }
  else if (newTopic.equalsIgnoreCase(STATUS_TOPIC) && length > 0)
  {
    Serial.print("Do not include payload in status update request");
    Serial.println();
    return;
  }

  if (length == 0)
  {
    Serial.print("No payload detected");
    Serial.println();
    return;
  }

  if (newTopic.equalsIgnoreCase(TARGETTEMP_TOPIC))
  {

    if (HOLD_MODE)
    {
      TARGET_HOLD_TEMP = intPayload;
    }
    else
    {
      if (AWAY_MODE)
      {
        if (CURRENT_MODE == 1)
        {                                     //heating
          TARGET_HEAT_AWAY_TEMP = intPayload; // save target for heating
        }
        else if (CURRENT_MODE == 2)
        {                                     //cooling
          TARGET_COOL_AWAY_TEMP = intPayload; // save target for cooling
        }
      }
      else
      {
        if (CURRENT_MODE == 1)
        {                                //heating
          TARGET_HEAT_TEMP = intPayload; // save target for heating
        }
        else if (CURRENT_MODE == 2)
        {                                //cooling
          TARGET_COOL_TEMP = intPayload; // save target for cooling
        }
      }
    }
  }

  if (newTopic.equalsIgnoreCase(CURRENTTEMP_TOPIC)) // check if topic is for the current temp
  {
    CURRENT_TEMP = floatPayload; // save to current temperature
    LAST_UPDATE = millis();      // save now as the last time temp was received
    status_update();
    return;
  }

  if (newTopic == CURRENTHUMIDITY_TOPIC) //check if topic is for the current humidity
  {
    CURRENT_HUMIDITY = floatPayload;
    LAST_UPDATE = millis();
    status_update();
    return;
  }

  if (newTopic.equalsIgnoreCase(AWAY_TOPIC))
  {
    if (intPayload == 1 || stringPayload.equalsIgnoreCase("true") || stringPayload.equalsIgnoreCase("on"))
    {
      AWAY_MODE = true;
    }
    else if (intPayload == 0 || stringPayload.equalsIgnoreCase("false") || stringPayload.equalsIgnoreCase("off"))
    {
      AWAY_MODE = false;
    }
  }

  if (newTopic.equalsIgnoreCase(HUMIDIFIER_TOPIC))
  {
    if (intPayload == 1 || stringPayload.equalsIgnoreCase("true") || stringPayload.equalsIgnoreCase("on"))
    {
      HUMIDIFIER = true;
    }
    else if (intPayload == 0 || stringPayload.equalsIgnoreCase("false") || stringPayload.equalsIgnoreCase("off"))
    {
      HUMIDIFIER = false;
    }
  }

  if (newTopic.equalsIgnoreCase(HOLD_TOPIC))
  {
    if (intPayload == 1 || stringPayload.equalsIgnoreCase("true") || stringPayload.equalsIgnoreCase("on"))
    {
      HOLD_MODE = true;
    }
    else if (intPayload == 0 || stringPayload.equalsIgnoreCase("false") || stringPayload.equalsIgnoreCase("off"))
    {
      HOLD_MODE = false;
    }
  }

  if (newTopic.equalsIgnoreCase(CURRENTMODE_TOPIC))
  {
    Serial.print(stringPayload);
    Serial.println();
    if (stringPayload.equalsIgnoreCase("off"))
    {
      CURRENT_MODE = 0;
    }
    else if (stringPayload.equalsIgnoreCase("heat"))
    {
      CURRENT_MODE = 1;
    }
    else if (stringPayload.equalsIgnoreCase("cool"))
    {
      CURRENT_MODE = 2;
    }
  }

  if (newTopic.equalsIgnoreCase(FAN_TOPIC))
  {
    if (stringPayload.equalsIgnoreCase("auto"))
    {
      FAN_MODE = 0;
    }
    else if (stringPayload.equalsIgnoreCase("on"))
    {
      FAN_MODE = 1;
    }
  }

  if (newTopic.equalsIgnoreCase(TARGETHUMIDITY_TOPIC))
  {
    TARGET_HUMIDITY = intPayload;
  }

  if (newTopic.equalsIgnoreCase(RELAY_HEAT_TOPIC))
  {
    if (intPayload == 1 || stringPayload.equalsIgnoreCase("true") || stringPayload.equalsIgnoreCase("on"))
    {
      digitalWrite(RELAY_HEAT, LOW_TRIGGER ? LOW : HIGH);
    }
    else if (intPayload == 0 || stringPayload.equalsIgnoreCase("false") || stringPayload.equalsIgnoreCase("off"))
    {
      digitalWrite(RELAY_HEAT, LOW_TRIGGER ? HIGH : LOW);
    }
  }

  if (newTopic.equalsIgnoreCase(RELAY_COOL_TOPIC))
  {
    if (intPayload == 1 || stringPayload.equalsIgnoreCase("true") || stringPayload.equalsIgnoreCase("on"))
    {
      digitalWrite(RELAY_COOL, LOW_TRIGGER ? LOW : HIGH);
    }
    else if (intPayload == 0 || stringPayload.equalsIgnoreCase("false") || stringPayload.equalsIgnoreCase("off"))
    {
      digitalWrite(RELAY_COOL, LOW_TRIGGER ? HIGH : LOW);
    }
  }

  if (newTopic.equalsIgnoreCase(RELAY_FAN_TOPIC))
  {
    if (intPayload == 1 || stringPayload.equalsIgnoreCase("true") || stringPayload.equalsIgnoreCase("on"))
    {
      digitalWrite(RELAY_FAN, LOW_TRIGGER ? LOW : HIGH);
    }
    else if (intPayload == 0 || stringPayload.equalsIgnoreCase("false") || stringPayload.equalsIgnoreCase("off"))
    {
      digitalWrite(RELAY_FAN, LOW_TRIGGER ? HIGH : LOW);
    }
  }

  if (newTopic.equalsIgnoreCase(RELAY_HUMIDIFIER_TOPIC))
  {
    if (intPayload == 1 || stringPayload.equalsIgnoreCase("true") || stringPayload.equalsIgnoreCase("on"))
    {
      digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? LOW : HIGH);
    }
    else if (intPayload == 0 || stringPayload.equalsIgnoreCase("false") || stringPayload.equalsIgnoreCase("off"))
    {
      digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW);
    }
  }

  status_update();
    if (!saveOPERATION_config()) {
    Serial.println("Failed to save config");
  } else {
    Serial.println("Config saved");
  }

}

void status_update()
{
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  if (HOLD_MODE)
  {
    root["target_temp"] = TARGET_HOLD_TEMP;
  }
  else if (CURRENT_MODE == 1)
  {
    root["target_temp"] = AWAY_MODE ? TARGET_HEAT_AWAY_TEMP : TARGET_HEAT_TEMP;
  }
  else if (CURRENT_MODE == 2)
  {
    root["target_temp"] = AWAY_MODE ? TARGET_COOL_AWAY_TEMP : TARGET_COOL_TEMP;
  }
  root["temp"] = CURRENT_TEMP;
  root["target_humidity"] = TARGET_HUMIDITY;
  root["humidity"] = CURRENT_HUMIDITY;
  root["mode"] = MODE_ARRAY[CURRENT_MODE];
  root["fan"] = FAN_ARRAY[FAN_MODE];
  root["away"] = AWAY_MODE;
  root["hold"] = HOLD_MODE;
  root["humidifier"] = HUMIDIFIER;
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  root.printTo(Serial);
  Serial.println();
  client.publish(PUBLISH_STATUS_TOPIC, buffer);

  publish_relay();
}

void publish_relay()
{
  StaticJsonBuffer<256> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["heat"] = LOW_TRIGGER ? !digitalRead(RELAY_HEAT) : digitalRead(RELAY_HEAT);
  root["cool"] = LOW_TRIGGER ? !digitalRead(RELAY_COOL) : digitalRead(RELAY_COOL);
  root["fan"] = LOW_TRIGGER ? !digitalRead(RELAY_FAN) : digitalRead(RELAY_FAN);
  root["humidifier"] = LOW_TRIGGER ? !digitalRead(RELAY_HUMIDIFIER) : digitalRead(RELAY_HUMIDIFIER);
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  root.printTo(Serial);
  Serial.println();
  client.publish(PUBLISH_RELAY_TOPIC, buffer);
}

void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }

  if (!loadGPIO_config())
  {
    Serial.println("Failed to load GPIO config");
  }
  else
  {
    Serial.println("GPIO Config loaded");
  }
    if (!loadCONNECT_config())
  {
    Serial.println("Failed to load CONNECT config");
  }
  else
  {
    Serial.println("CONNECT Config loaded");
  }
    if (!loadOPERATION_config())
  {
    Serial.println("Failed to load OPERATION config");
  }
  else
  {
    Serial.println("OPERATION Config loaded");
  }

  static DHT static_dht(DHT_DATA_PIN, DHTTYPE);
  // save its address.
  dht = &static_dht;
  // call begin() and your ready to use the sensor if correctly wired.
  dht->begin();

  digitalWrite(RELAY_COOL, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_FAN, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_HEAT, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW);

  pinMode(RELAY_COOL, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_HEAT, OUTPUT);
  pinMode(RELAY_HUMIDIFIER, OUTPUT);

  char charNAME[TOPIC_NAME.length() + 1];
  TOPIC_NAME.toCharArray(charNAME, sizeof(charNAME));

  //SET UP MQTT TOPICS FOR SUBSCRIBE
  sprintf(GENERAL_SUBSCRIBE_TOPIC, "%s/%s/#", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(STATUS_TOPIC, "%s/%s/status", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(TARGETTEMP_TOPIC, "%s/%s/targettemp", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(CURRENTTEMP_TOPIC, "%s/%s/temp", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(CURRENTHUMIDITY_TOPIC, "%s/%s/humidity", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(TARGETHUMIDITY_TOPIC, "%s/%s/targethumidity", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(CURRENTMODE_TOPIC, "%s/%s/mode", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(AWAY_TOPIC, "%s/%s/away", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(HOLD_TOPIC, "%s/%s/hold", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(FAN_TOPIC, "%s/%s/fan", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(HUMIDIFIER_TOPIC, "%s/%s/humidifier", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(RELAY_HEAT_TOPIC, "%s/%s/relay/heat", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(RELAY_COOL_TOPIC, "%s/%s/relay/cool", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(RELAY_FAN_TOPIC, "%s/%s/relay/fan", MQTT_COMMAND_PREFIX, charNAME);
  sprintf(RELAY_HUMIDIFIER_TOPIC, "%s/%s/relay/humidifier", MQTT_COMMAND_PREFIX, charNAME);

  //SET UP MQTT TOPICS FOR PUBLISH
  sprintf(PUBLISH_STATUS_TOPIC, "%s/%s/status", MQTT_RESPONSE_PREFIX, charNAME);
  sprintf(PUBLISH_SENSOR_TOPIC, "%s/%s/sensor", MQTT_RESPONSE_PREFIX, charNAME);
  sprintf(PUBLISH_RELAY_TOPIC, "%s/%s/relay", MQTT_RESPONSE_PREFIX, charNAME);

  setup_wifi();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", mainhtml, processor);
  });
  server.on("/connectivity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", connectivityhtml, processor);
  });
  server.on("/operation", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", operationhtml, processor);
  });
  server.on("/relay", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", relayhtml, processor);
  });
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", reboothtml, processor);
    ESP.restart();
  });
  server.on("/cleared", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", clearedhtml, processor);
    ESP.restart();
  });

  server.on("/connectivity/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    
    if (request->hasArg("ssid"))
    {
      ssid = request->arg("ssid");
    }
    if (request->hasArg("password"))
    {
      password = request->arg("password");
    }
    if (request->hasArg("mqtt_server"))
    {
      mqtt_server = request->arg("mqtt_server");
    }
    if (request->hasArg("mqtt_port"))
    {
      mqtt_port = request->arg("mqtt_port").toInt();
    }
    if (request->hasArg("mqtt_user"))
    {
      mqtt_user = request->arg("mqtt_user");
    }
    if (request->hasArg("mqtt_pass"))
    {
      mqtt_pass = request->arg("mqtt_pass");
    }
    if (request->hasArg("name"))
    {
      TOPIC_NAME = request->arg("name");
    }

      status_update();
    if (!saveCONNECT_config()) {
    Serial.println("Failed to save config");
  } else {
    Serial.println("Config saved");
  }
    
    if (WiFi.status() != WL_CONNECTED)
    {
      request->redirect("/reboot");
    }
    else
    {
      request->redirect("/");
      ESP.restart();
    }
  });

  server.on("/operation/set", HTTP_POST, [](AsyncWebServerRequest *request) {

    if (request->hasArg("mode"))
    {
      CURRENT_MODE = request->arg("mode").toInt();
    }
    if (request->hasArg("fan"))
    {
      FAN_MODE = request->arg("fan").toInt();
    }
    AWAY_MODE = request->hasArg("away") ? true : false;
    HOLD_MODE = request->hasArg("hold") ? true : false;
    HUMIDIFIER = request->hasArg("humidifier") ? true : false;
    if (request->hasArg("heat_target"))
    {
      TARGET_HEAT_TEMP = request->arg("heat_target").toInt();
    }
    if (request->hasArg("cool_target"))
    {
      TARGET_COOL_TEMP = request->arg("cool_target").toInt();
    }
    if (request->hasArg("heat_away_target"))
    {
      TARGET_HEAT_AWAY_TEMP = request->arg("heat_away_target").toInt();
    }
    if (request->hasArg("cool_away_target"))
    {
      TARGET_COOL_AWAY_TEMP = request->arg("cool_away_target").toInt();
    }
    if (request->hasArg("hold_target"))
    {
      TARGET_HOLD_TEMP = request->arg("hold_target").toInt();
    }
    if (request->hasArg("swing_temp"))
    {
      SWING_TEMP = request->arg("swing_temp").toFloat();
    }
    if (request->hasArg("swing_humidity"))
    {
      SWING_HUMIDITY = request->arg("swing_humidity").toInt();
    }
    if (request->hasArg("min_runtime"))
    {
      MIN_RUNTIME = request->arg("min_runtime").toInt();
    }
    if (request->hasArg("min_update"))
    {
      MIN_UPDATE = request->arg("min_update").toInt();
    }
    if (request->hasArg("sensor_update"))
    {
      SENSOR_UPDATE = request->arg("sensor_update").toInt();
    }
      status_update();
    if (!saveOPERATION_config()) {
    Serial.println("Failed to save config");
  } else {
    Serial.println("Config saved");
  }
    request->redirect("/operation");
  });

  server.on("/relay/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    

    if (request->hasArg("heat_relay"))
    {
      RELAY_HEAT = request->arg("heat_relay").toInt();
    }
    if (request->hasArg("cool_relay"))
    {
      RELAY_COOL = request->arg("cool_relay").toInt();
    }
    if (request->hasArg("fan_relay"))
    {
      RELAY_FAN = request->arg("fan_relay").toInt();
    }
    if (request->hasArg("humidifier_relay"))
    {
      RELAY_HUMIDIFIER = request->arg("humidifier_relay").toInt();
    }
    if (request->hasArg("dht22_pin"))
    {
      DHT_DATA_PIN = request->arg("dht22_pin").toInt();
    }
    LOW_TRIGGER = request->hasArg("low_trigger") ? true : false;
      status_update();
    if (!saveGPIO_config()) {
    Serial.println("Failed to save config");
  } else {
    Serial.println("Config saved");
  }

    // digitalWrite(RELAY_HEAT, LOW_TRIGGER ? HIGH : LOW);
    // digitalWrite(RELAY_COOL, LOW_TRIGGER ? HIGH : LOW);
    // digitalWrite(RELAY_FAN, LOW_TRIGGER ? HIGH : LOW);
    // digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW);
    // pinMode(RELAY_COOL, OUTPUT);
    // pinMode(RELAY_FAN, OUTPUT);
    // pinMode(RELAY_HEAT, OUTPUT);
    // pinMode(RELAY_HUMIDIFIER, OUTPUT);
    request->redirect("/relay");
    ESP.restart();
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg("reset") && request->arg("reset") == "FACTORY RESET")
    {
      SPIFFS.format();
      request->redirect("/cleared");
    }
    else
    {
      request->redirect("/");
    }
  });
  server.begin();

  Serial.println("HTTP server started");
  client.setServer(mqtt_server.c_str(), mqtt_port);
  client.setCallback(callback);

  LAST_RECONNECT_ATTEMPT = 0;
}

void loop()
{
  //  server.handleClient();
  long now = millis();
  if (!client.connected())
  {
    if (now - LAST_RECONNECT_ATTEMPT > 5000)
    {
      LAST_RECONNECT_ATTEMPT = now;
      // Attempt to reconnect
      if (reconnect())
      {
        LAST_RECONNECT_ATTEMPT = 0;
      }
    }
  }
  else
  {
    // Client connected

    client.loop();
  }
  if (now - LAST_STATUS_UPDATE > STATUS_UPDATE_INTERVAL)
  {
    LAST_STATUS_UPDATE = now;
    status_update();
  }
  if (now - LAST_SENSOR_READ > (SENSOR_UPDATE * 1000))
  {
    // DHT dht(DHT_DATA_PIN, DHTTYPE); // SET UP THE DHT22
    // dht.begin();
    // delay(100);
    // float h = dht.readHumidity();        // READ THE HUMIDITY
    // float t = dht.readTemperature(true); // READ THE TEMPERATURE IN FAHRENHEIT
    
    float h = dht->readHumidity();        // READ THE HUMIDITY
    float t = dht->readTemperature(true); // READ THE TEMPERATURE IN FAHRENHEIT

    if (!isnan(h) || !isnan(t))
    {
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject &root = jsonBuffer.createObject();

      root["humidity"] = h;
      root["temperature"] = t;

      char buffer[200];
      root.printTo(buffer, sizeof(buffer));
      root.printTo(Serial);
      Serial.println();
      client.publish(PUBLISH_SENSOR_TOPIC, buffer);
    }

    LAST_SENSOR_READ = now; // SET LAST_SENSOR_READ TO NOW
    if (now - LAST_UPDATE > (MIN_UPDATE * 1000))
    { // IF IT'S BEEN TOO LONG SINCE MQTT TEMP WAS RECEIVED, USE SENSOR VALUE
      CURRENT_TEMP = isnan(t) ? 0 : t;
      CURRENT_HUMIDITY = isnan(h) ? 0 : h;
    }
  }

  if (CURRENT_TEMP < 1)
  {
    turn_off();
    return;
  }
  switch (CURRENT_MODE)
  {
  case 0:
    turn_off();
    return;
    break;
  case 1:

    operate_heat();
    break;
  case 2:
    operate_cool();
    break;
  }
  operate_humidifier();
  operate_fan();
}

void turn_off()
{
  digitalWrite(RELAY_HEAT, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_COOL, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_FAN, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW);
}

void operate_heat()
{

  if (digitalRead(RELAY_COOL) == LOW_TRIGGER ? LOW : HIGH)
  {                                                     // CHECK IF RELAY_COOL IS ON
    digitalWrite(RELAY_COOL, LOW_TRIGGER ? HIGH : LOW); // TURN OFF RELAY_COOL
    publish_relay();
  }
  int HEAT_TARGET;
  if (HOLD_MODE)
  {
    HEAT_TARGET = TARGET_HOLD_TEMP;
  }
  else if (AWAY_MODE)
  {
    HEAT_TARGET = TARGET_HEAT_AWAY_TEMP;
  }
  else
  {
    HEAT_TARGET = TARGET_HEAT_TEMP;
  }

  if ((CURRENT_TEMP <= (HEAT_TARGET - SWING_TEMP)) && (digitalRead(RELAY_HEAT) == LOW_TRIGGER ? HIGH : LOW))
  {
    Serial.println();
    Serial.print("Trigger was called to turn HEAT ON");
    Serial.println();
    Serial.println();

    digitalWrite(RELAY_HEAT, LOW_TRIGGER ? LOW : HIGH); // TURN THE HEAT ON
    LAST_RUNTIME = millis();
    publish_relay();
  }
  else if (CURRENT_TEMP >= (HEAT_TARGET + SWING_TEMP) && (digitalRead(RELAY_HEAT) == LOW_TRIGGER ? LOW : HIGH) && (millis() - LAST_RUNTIME > (MIN_RUNTIME * 1000)))
  {

    Serial.println();
    Serial.print("Trigger was called to turn HEAT OFF");
    Serial.println();
    Serial.println();
    digitalWrite(RELAY_HEAT, LOW_TRIGGER ? HIGH : LOW); // TURN THE HEAT OFF
    publish_relay();
    if (digitalRead(RELAY_HUMIDIFIER) == LOW_TRIGGER ? LOW : HIGH)
    {
      digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW); // TURN THE HUMIDIFIER OFF
      publish_relay();
    }
  }
}

void operate_cool()
{
  if (digitalRead(RELAY_HEAT) == LOW_TRIGGER ? LOW : HIGH)
  {                                                     // CHECK IF RELAY_HEAT IS ON
    digitalWrite(RELAY_HEAT, LOW_TRIGGER ? HIGH : LOW); // TURN OFF RELAY_HEAT
    publish_relay();
  }
  if (digitalRead(RELAY_HUMIDIFIER) == LOW_TRIGGER ? LOW : HIGH)
  {                                                           // CHECK IF RELAY_HUMIDIFIER IS ON
    digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW); // TURN OFF RELAY_HUMIDIFIER
    publish_relay();
  }

  int COOL_TARGET;
  if (HOLD_MODE)
  {
    COOL_TARGET = TARGET_HOLD_TEMP;
  }
  else if (AWAY_MODE)
  {
    COOL_TARGET = TARGET_COOL_AWAY_TEMP;
  }
  else
  {
    COOL_TARGET = TARGET_COOL_TEMP;
  }
  if ((CURRENT_TEMP >= (COOL_TARGET + SWING_TEMP)) && (digitalRead(RELAY_COOL) == LOW_TRIGGER ? HIGH : LOW))
  {
    Serial.println();
    Serial.print("Trigger was called to turn COOL ON");
    Serial.println();
    Serial.println();
    digitalWrite(RELAY_COOL, LOW_TRIGGER ? LOW : HIGH); // TURN THE COOL ON
    LAST_RUNTIME = millis();
    publish_relay();
  }
  //  else if (CURRENT_TEMP <= (COOL_TARGET - SWING_TEMP) && millis() - LAST_RUNTIME > MIN_RUNTIME)
  else if (CURRENT_TEMP <= (COOL_TARGET - SWING_TEMP) && (digitalRead(RELAY_COOL) == LOW_TRIGGER ? LOW : HIGH) && (millis() - LAST_RUNTIME > (MIN_RUNTIME * 1000)))
  {
    Serial.println();
    Serial.print("Trigger was called to turn COOL OFF");
    Serial.println();
    Serial.println();
    digitalWrite(RELAY_COOL, LOW_TRIGGER ? HIGH : LOW); // TURN THE COOL OFF
    publish_relay();
  }
}

void operate_fan()
{
  if ((FAN_MODE == 1) && (digitalRead(RELAY_FAN) == LOW_TRIGGER ? HIGH : LOW))
  {
    digitalWrite(RELAY_FAN, LOW_TRIGGER ? LOW : HIGH); // TURN THE FAN ON
    publish_relay();
  }
  else if ((FAN_MODE == 0) && (digitalRead(RELAY_FAN) == LOW_TRIGGER ? LOW : HIGH))
  {
    digitalWrite(RELAY_FAN, LOW_TRIGGER ? HIGH : LOW); // TURN THE FAN OFF
    publish_relay();
  }
}

void operate_humidifier()
{
  if (HUMIDIFIER && (digitalRead(RELAY_HEAT) == LOW_TRIGGER ? LOW : HIGH))
  {
    if ((CURRENT_HUMIDITY <= (TARGET_HUMIDITY - SWING_HUMIDITY)) && (digitalRead(RELAY_HUMIDIFIER) == LOW_TRIGGER ? HIGH : LOW))
    {
      Serial.println();
      Serial.print("Trigger to turn HUMIDIFIER ON");
      Serial.println();
      Serial.println();
      digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? LOW : HIGH); // TURN THE HUMIDIFIER ON
      publish_relay();
    }
    else if ((CURRENT_HUMIDITY >= (TARGET_HUMIDITY + SWING_HUMIDITY)) && (digitalRead(RELAY_HUMIDIFIER) == LOW_TRIGGER ? LOW : HIGH))
    {
      Serial.println();
      Serial.print("Trigger to turn HUMIDIFIER OFF");
      Serial.println();
      Serial.println();
      digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW); // TURN THE HUMIDIFIER OFF
      publish_relay();
    }
  }
  else
  {
    if (digitalRead(RELAY_HUMIDIFIER) == LOW_TRIGGER ? LOW : HIGH)
    {                                                           // CHECK IF RELAY_HUMIDIFIER IS ON
      digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW); // TURN OFF RELAY_HUMIDIFIER
      publish_relay();
    }
  }
}
