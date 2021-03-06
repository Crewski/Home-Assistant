


#include <ArduinoJson.h>
#include <DHT.h>
#include <Preferences.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "ESPAsyncWebServer.h"

/**********************/
/**** DEFINE PINS *****/
/**********************/

int RELAY_HEAT;
int RELAY_COOL;
int RELAY_FAN;
int RELAY_HUMIDIFIER;
int DHT_DATA_PIN;



#define DHTTYPE DHT22

#define STATUS_UPDATE_INTERVAL 60000

/***************************/
/**** DEFINE VARIABLES *****/
/***************************/

long MIN_RUNTIME;
long MIN_UPDATE;
long SENSOR_UPDATE;
float CURRENT_TEMP;
float CURRENT_HUMIDITY;
int TARGET_COOL_TEMP;
int TARGET_HEAT_TEMP;
int TARGET_HUMIDITY;
int TARGET_HEAT_AWAY_TEMP;
int TARGET_COOL_AWAY_TEMP;
int TARGET_HOLD_TEMP;
float SWING_TEMP;
int SWING_HUMIDITY;
bool AWAY_MODE;
bool HOLD_MODE;
bool HUMIDIFIER;
int CURRENT_MODE; // 0 = off, 1 = heat, 2 = cool
String MODE_OFF = "off";
String MODE_HEAT = "heat";
String MODE_COOL = "cool";
String MODE_ARRAY[] = {MODE_OFF, MODE_HEAT, MODE_COOL};
String FAN_AUTO = "auto";
String FAN_ON = "on";
String NAME;
int FAN_MODE = 0; // 0 = auto, 1 = on
String FAN_ARRAY[] = {FAN_AUTO, FAN_ON};

bool LOW_TRIGGER;

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

String ssid;
String password;
String mqtt_server;
int mqtt_port;
String mqtt_user;
String mqtt_pass;

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

Preferences DATA_STORAGE;

const char *mainhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;font-family:Arial,Helvetica,Sans-Serif;Color:#000}h1,h2,h3{text-align:center}td{text-align:center;width:50%%}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}.alert{background-color:red}.hvac-table{width:80%%;margin:0 auto;padding-bottom:40px}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Current data</h3> <table class=hvac-table> <tr> <td>Mode:<td>%CURRENT_MODE%<tr> <td>Target:<td>%TARGET_TEMP% F<tr> <td>Currently:<td>%CURRENT_TEMP% F<tr> <td>Away mode:<td>%AWAY_MODE%<tr> <td>Hold mode:<td>%HOLD_MODE%<tr> <td>Target Humidity:<td>%TARGET_HUMIDITY% %%<tr> <td>Current Humidity:<td>%CURRENT_HUMIDITY% %% </table> <form><input class=hvac-button onclick='window.location.href=\"operation\"' type=button value=\"OPERATIONS\"></form> <form><input class=hvac-button onclick='window.location.href=\"connectivity\"' type=button value=\"CONNECTIVITY\"></form> <form><input class=hvac-button onclick='window.location.href=\"relay\"' type=button value=\"RELAYS/SENSOR\"></form> <form action=/reset method=POST><input class=\"hvac-button alert\" onclick='return confirm(\"Do you want to reset back to factory settings?\")' type=submit value=\"FACTORY RESET\" name=reset></form>";
const char *connectivityhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{width:80%%;margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;width:50%%;padding:10px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Connectivity Settings</h3> <form action=connectivity/set method=POST> <table> <tr> <td>SSID:<td><input value=\"%SSID%\" name=ssid> <tr> <td>Password:<td><input value=\"%PASSWORD%\" name=password type=password> <tr> <td>MQTT Server:<td><input value=\"%MQTT_SERVER%\" name=mqtt_server> <tr> <td>MQTT Port:<td><input value=\"%MQTT_PORT%\" name=mqtt_port> <tr> <td>MQTT Username:<td><input value=\"%MQTT_USER%\" name=mqtt_user> <tr> <td>MQTT Password:<td><input value=\"%MQTT_PASS%\" name=mqtt_pass type=password> <tr> <td>Topic Name<td><input value=\"%NAME%\" name=name> </table><input value=\"SAVE SETTINGS\" type=submit class=hvac-button> </form> <form><input value=\"MAIN MENU\" type=button class=hvac-button onclick='window.location.href=\"/\"'></form> <h4>*Topic Name is used in the mqtt topics</h4>";
const char *operationhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;padding:10px}input{text-align:center}span{padding-left:30px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Operation Settings</h3> <form action=operation/set method=POST> <table> <tr> <td>Mode:<td colspan=2><span><input type=radio name=mode value=0 %off_mode%>Off</span> <span><input type=radio name=mode value=1 %heat_mode%>Heat</span> <span><input type=radio name=mode value=2 %cool_mode%>Cool</span> <tr><td>Fan:<td><span><input type=radio name=fan value=0 %fan_auto%>Auto</span><span><input type=radio name=fan value=1 %fan_on%>On</span> <tr> <td>Hold:<td><input type=checkbox name=hold %HOLD_CHECK%> <tr> <td>Away:<td><input type=checkbox name=away %AWAY_CHECK%> <tr> <td>Humidifier:<td><input type=checkbox name=humidifier %HUMIDIFIER_CHECK%> <tr> <td> <th>Heat<th>Cool<tr> <td>Target:<td><input type=number name=heat_target value=\"%HEAT_TARGET%\"> <td><input type=number name=cool_target value=\"%COOL_TARGET%\"> <tr> <td>Away Target:<td><input type=number name=heat_away_target value=\"%HEAT_AWAY_TARGET%\"> <td><input type=number name=cool_away_target value=\"%COOL_AWAY_TARGET%\"> <tr> <td>Hold Target:<td><input type=number name=hold_target value=\"%HOLD_TARGET%\"> <tr> <td>Swing Temp:<td><input type=number name=swing_temp value=\"%SWING_TEMP%\" step=0.5> <tr> <td>Swing Humidity:<td><input type=number name=swing_humidity value=\"%SWING_HUMIDITY%\"> <tr> <td>Min Runtime (sec):<td><input type=number name=min_runtime value=\"%MIN_RUNTIME%\"><td>Minimum time furnace/ac will run <tr> <td>Min Receive (sec):<td><input type=number name=min_update value=\"%MIN_UPDATE%\"><td>Minimum time between recieving remote temp to switch to local <tr> <td>Sensor Update (sec):<td><input type=number name=sensor_update value=\"%SENSOR_UPDATE%\"><td>How often to take/publish local reading </table><input type=submit value=\"SAVE SETTINGS\" class=hvac-button> </form> <form><input type=button value=\"MAIN MENU\" class=hvac-button onclick='window.location.href=\"/\"'></form>";
const char *relayhtml = "<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;padding:10px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Relay/Sensor Settings</h3> <form action=relay/set method=POST> <table> <tr> <td>Low Trigger:<td><input type=\"checkbox\" name=\"low_trigger\" %LOW_TRIGGER%> <tr> <td>Heat:<td><select name=heat_relay> <option value=0 %heat_relay0%>GPIO 0<option value=1 %heat_relay1%>GPIO 1<option value=2 %heat_relay2%>GPIO 2<option value=3 %heat_relay3%>GPIO 3<option value=4 %heat_relay4%>GPIO 4<option value=5 %heat_relay5%>GPIO 5<option value=6 %heat_relay6%>GPIO 6<option value=7 %heat_relay7%>GPIO 7<option value=8 %heat_relay8%>GPIO 8<option value=9 %heat_relay9%>GPIO 9<option value=10 %heat_relay10%>GPIO 10<option value=11 %heat_relay11%>GPIO 11<option value=12 %heat_relay12%>GPIO 12<option value=13 %heat_relay13%>GPIO 13<option value=14 %heat_relay14%>GPIO 14<option value=15 %heat_relay15%>GPIO 15<option value=16 %heat_relay16%>GPIO 16<option value=17 %heat_relay17%>GPIO 17<option value=18 %heat_relay18%>GPIO 18<option value=19 %heat_relay19%>GPIO 19<option value=21 %heat_relay21%>GPIO 21<option value=22 %heat_relay22%>GPIO 22<option value=23 %heat_relay23%>GPIO 23<option value=25 %heat_relay25%>GPIO 25<option value=26 %heat_relay26%>GPIO 26<option value=27 %heat_relay27%>GPIO 27 </select> <tr> <td>Cool:<td><select name=cool_relay> <option value=0 %cool_relay0%>GPIO 0<option value=1 %cool_relay1%>GPIO 1<option value=2 %cool_relay2%>GPIO 2<option value=3 %cool_relay3%>GPIO 3<option value=4 %cool_relay4%>GPIO 4<option value=5 %cool_relay5%>GPIO 5<option value=6 %cool_relay6%>GPIO 6<option value=7 %cool_relay7%>GPIO 7<option value=8 %cool_relay8%>GPIO 8<option value=9 %cool_relay9%>GPIO 9<option value=10 %cool_relay10%>GPIO 10<option value=11 %cool_relay11%>GPIO 11<option value=12 %cool_relay12%>GPIO 12<option value=13 %cool_relay13%>GPIO 13<option value=14 %cool_relay14%>GPIO 14<option value=15 %cool_relay15%>GPIO 15<option value=16 %cool_relay16%>GPIO 16<option value=17 %cool_relay17%>GPIO 17<option value=18 %cool_relay18%>GPIO 18<option value=19 %cool_relay19%>GPIO 19<option value=21 %cool_relay21%>GPIO 21<option value=22 %cool_relay22%>GPIO 22<option value=23 %cool_relay23%>GPIO 23<option value=25 %cool_relay25%>GPIO 25<option value=26 %cool_relay26%>GPIO 26<option value=27 %cool_relay27%>GPIO 27 </select> <tr> <td>Fan:<td><select name=fan_relay> <option value=0 %fan_relay0%>GPIO 0<option value=1 %fan_relay1%>GPIO 1<option value=2 %fan_relay2%>GPIO 2<option value=3 %fan_relay3%>GPIO 3<option value=4 %fan_relay4%>GPIO 4<option value=5 %fan_relay5%>GPIO 5<option value=6 %fan_relay6%>GPIO 6<option value=7 %fan_relay7%>GPIO 7<option value=8 %fan_relay8%>GPIO 8<option value=9 %fan_relay9%>GPIO 9<option value=10 %fan_relay10%>GPIO 10<option value=11 %fan_relay11%>GPIO 11<option value=12 %fan_relay12%>GPIO 12<option value=13 %fan_relay13%>GPIO 13<option value=14 %fan_relay14%>GPIO 14<option value=15 %fan_relay15%>GPIO 15<option value=16 %fan_relay16%>GPIO 16<option value=17 %fan_relay17%>GPIO 17<option value=18 %fan_relay18%>GPIO 18<option value=19 %fan_relay19%>GPIO 19<option value=21 %fan_relay21%>GPIO 21<option value=22 %fan_relay22%>GPIO 22<option value=23 %fan_relay23%>GPIO 23<option value=25 %fan_relay25%>GPIO 25<option value=26 %fan_relay26%>GPIO 26<option value=27 %fan_relay27%>GPIO 27 </select> <tr> <td>Humidifier:<td><select name=humidifier_relay> <option value=0 %humidifier_relay0%>GPIO 0<option value=1 %humidifier_relay1%>GPIO 1<option value=2 %humidifier_relay2%>GPIO 2<option value=3 %humidifier_relay3%>GPIO 3<option value=4 %humidifier_relay4%>GPIO 4<option value=5 %humidifier_relay5%>GPIO 5<option value=6 %humidifier_relay6%>GPIO 6<option value=7 %humidifier_relay7%>GPIO 7<option value=8 %humidifier_relay8%>GPIO 8<option value=9 %humidifier_relay9%>GPIO 9<option value=10 %humidifier_relay10%>GPIO 10<option value=11 %humidifier_relay11%>GPIO 11<option value=12 %humidifier_relay12%>GPIO 12<option value=13 %humidifier_relay13%>GPIO 13<option value=14 %humidifier_relay14%>GPIO 14<option value=15 %humidifier_relay15%>GPIO 15<option value=16 %humidifier_relay16%>GPIO 16<option value=17 %humidifier_relay17%>GPIO 17<option value=18 %humidifier_relay18%>GPIO 18<option value=19 %humidifier_relay19%>GPIO 19<option value=21 %humidifier_relay21%>GPIO 21<option value=22 %humidifier_relay22%>GPIO 22<option value=23 %humidifier_relay23%>GPIO 23<option value=25 %humidifier_relay25%>GPIO 25<option value=26 %humidifier_relay26%>GPIO 26<option value=27 %humidifier_relay27%>GPIO 27 </select> <tr> <td>DHT22:<td><select name=dht22_pin> <option value=0 %dht_pin0%>GPIO 0<option value=1 %dht_pin1%>GPIO 1<option value=2 %dht_pin2%>GPIO 2<option value=3 %dht_pin3%>GPIO 3<option value=4 %dht_pin4%>GPIO 4<option value=5 %dht_pin5%>GPIO 5<option value=6 %dht_pin6%>GPIO 6<option value=7 %dht_pin7%>GPIO 7<option value=8 %dht_pin8%>GPIO 8<option value=9 %dht_pin9%>GPIO 9<option value=10 %dht_pin10%>GPIO 10<option value=11 %dht_pin11%>GPIO 11<option value=12 %dht_pin12%>GPIO 12<option value=13 %dht_pin13%>GPIO 13<option value=14 %dht_pin14%>GPIO 14<option value=15 %dht_pin15%>GPIO 15<option value=16 %dht_pin16%>GPIO 16<option value=17 %dht_pin17%>GPIO 17<option value=18 %dht_pin18%>GPIO 18<option value=19 %dht_pin19%>GPIO 19<option value=21 %dht_pin21%>GPIO 21<option value=22 %dht_pin22%>GPIO 22<option value=23 %dht_pin23%>GPIO 23<option value=25 %dht_pin25%>GPIO 25<option value=26 %dht_pin26%>GPIO 26<option value=27 %dht_pin27%>GPIO 27<option value=34 %dht_pin34%>GPIO 34<option value=35 %dht_pin35%>GPIO 35<option value=36 %dht_pin36%>GPIO 36<option value=39 %dht_pin39%>GPIO 39 </select> </table><input class=hvac-button type=submit value=\"SAVE SETTINGS\"> </form> <form><input class=hvac-button type=button value=\"MAIN MENU\" onclick='window.location.href=\"/\"'></form>";
const char *reboothtml ="<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;font-family:Arial,Helvetica,Sans-Serif;Color:#000}h1,h2,h3{text-align:center}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Settings have been saved. The unit will reboot and try to connect to %SSID%. Look for it there.</h3>";
const char *clearedhtml ="<html lang=en-EN> <meta content=60 http-equiv=refresh> <title>ESP32 HVAC</title> <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;font-family:Arial,Helvetica,Sans-Serif;Color:#000}h1,h2,h3{text-align:center}</style> <h1>%NAME%</h1> <h2>THERMOSTAT</h2> <h3>Setting have been cleared.  The unit reboot and broadcast an access point</h3>";

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
    return NAME;
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
    uint64_t chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
    uint16_t chip = (uint16_t)(chipid >> 32);
    snprintf(apssid, 15, "HVAC-%04X", chip);
    WiFi.softAP(apssid);
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
  }
}

boolean reconnect()
{
  char mqttid[15];                     //Create a Unique AP from MAC address
  uint64_t chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(mqttid, 15, "HVAC-%04X", chip);
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

  // StaticJsonBuffer<256> jsonBuffer;
  // JsonObject &root = jsonBuffer.createObject();

  if (newTopic.equalsIgnoreCase(TARGETTEMP_TOPIC))
  {

    DATA_STORAGE.begin("thermostat", false);
    if (HOLD_MODE)
    {
      TARGET_HOLD_TEMP = intPayload;
      DATA_STORAGE.putInt(CONF_HOLD_TEMP, TARGET_HOLD_TEMP);
    }
    else
    {
      if (AWAY_MODE)
      {
        if (CURRENT_MODE == 1)
        {                                     //heating
          TARGET_HEAT_AWAY_TEMP = intPayload; // save target for heating
          DATA_STORAGE.putInt(CONF_HEAT_AWAY_TEMP, TARGET_HEAT_AWAY_TEMP);
        }
        else if (CURRENT_MODE == 2)
        {                                     //cooling
          TARGET_COOL_AWAY_TEMP = intPayload; // save target for cooling
          DATA_STORAGE.putInt(CONF_COOL_AWAY_TEMP, TARGET_COOL_AWAY_TEMP);
        }
      }
      else
      {
        if (CURRENT_MODE == 1)
        {                                //heating
          TARGET_HEAT_TEMP = intPayload; // save target for heating
          DATA_STORAGE.putInt(CONF_HEAT_TEMP, TARGET_HEAT_TEMP);
        }
        else if (CURRENT_MODE == 2)
        {                                //cooling
          TARGET_COOL_TEMP = intPayload; // save target for cooling
          DATA_STORAGE.putInt(CONF_COOL_TEMP, TARGET_COOL_TEMP);
        }
      }
    }
    DATA_STORAGE.end();
    // root["target_temp"] = intPayload;
  }

  if (newTopic.equalsIgnoreCase(CURRENTTEMP_TOPIC)) // check if topic is for the current temp
  {
    CURRENT_TEMP = floatPayload; // save to current temperature
    LAST_UPDATE = millis();      // save now as the last time temp was received
    // root["temp"] = CURRENT_TEMP;
  }

  if (newTopic == CURRENTHUMIDITY_TOPIC) //check if topic is for the current humidity
  {
    CURRENT_HUMIDITY = floatPayload;
    LAST_UPDATE = millis();
    // root["humidity"] = CURRENT_HUMIDITY;
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
    // root["away"] = AWAY_MODE;
    DATA_STORAGE.begin("thermostat", false);
    DATA_STORAGE.putBool(CONF_AWAY, AWAY_MODE);
    DATA_STORAGE.end();
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
    // root["humidifier"] = HUMIDIFIER;
    DATA_STORAGE.begin("thermostat", false);
    DATA_STORAGE.putBool(CONF_HUMIDIFIER, HUMIDIFIER);
    DATA_STORAGE.end();
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
    // root["hold"] = HOLD_MODE;
    DATA_STORAGE.begin("thermostat", false);
    DATA_STORAGE.putBool(CONF_HOLD, HOLD_MODE);
    DATA_STORAGE.end();
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

    // root["mode"] = MODE_ARRAY[CURRENT_MODE];
    DATA_STORAGE.begin("thermostat", false);
    DATA_STORAGE.putInt(CONF_MODE, CURRENT_MODE);
    DATA_STORAGE.end();
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
    // root["fan"] = FAN_ARRAY[FAN_MODE];
    DATA_STORAGE.begin("thermostat", false);
    DATA_STORAGE.putInt(CONF_FAN, FAN_MODE);
    DATA_STORAGE.end();
  }

  if (newTopic.equalsIgnoreCase(TARGETHUMIDITY_TOPIC))
  {
    TARGET_HUMIDITY = intPayload;
    // root["target_humidity"] = TARGET_HUMIDITY;
    DATA_STORAGE.begin("thermostat", false);
    DATA_STORAGE.putInt(CONF_HUMIDITY, TARGET_HUMIDITY);
    DATA_STORAGE.end();
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
    // publish_relay();
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
    // publish_relay();
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
    // publish_relay();
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
    // publish_relay();
  }

  status_update();

  // char buffer[200];
  // root.printTo(buffer, sizeof(buffer));
  // root.printTo(Serial);
  // Serial.println();
  // client.publish(PUBLISH_STATUS_TOPIC, buffer);
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

  // DECLARE PINS

  // GET DATA
  DATA_STORAGE.begin("thermostat", true);
  // DATA_STORAGE.clear();
  TARGET_COOL_AWAY_TEMP = DATA_STORAGE.getInt(CONF_COOL_AWAY_TEMP, 78);
  TARGET_COOL_TEMP = DATA_STORAGE.getInt(CONF_COOL_TEMP, 76);
  TARGET_HEAT_AWAY_TEMP = DATA_STORAGE.getInt(CONF_HEAT_AWAY_TEMP, 68);
  TARGET_HEAT_TEMP = DATA_STORAGE.getInt(CONF_HEAT_TEMP, 70);
  TARGET_HOLD_TEMP = DATA_STORAGE.getInt(CONF_HOLD_TEMP, 70);
  TARGET_HUMIDITY = DATA_STORAGE.getInt(CONF_HUMIDITY, 40);
  CURRENT_MODE = DATA_STORAGE.getInt(CONF_MODE, 0);
  FAN_MODE = DATA_STORAGE.getInt(CONF_FAN, 0);
  HOLD_MODE = DATA_STORAGE.getBool(CONF_HOLD, false);
  AWAY_MODE = DATA_STORAGE.getBool(CONF_AWAY, false);
  HUMIDIFIER = DATA_STORAGE.getBool(CONF_HUMIDIFIER, false);
  ssid = DATA_STORAGE.getString(CONF_SSID, "");
  password = DATA_STORAGE.getString(CONF_PASSWORD, "");
  mqtt_server = DATA_STORAGE.getString(CONF_MQTT_SERVER, "");
  mqtt_port = DATA_STORAGE.getInt(CONF_MQTT_PORT, 1883);
  mqtt_user = DATA_STORAGE.getString(CONF_MQTT_USER, "");
  mqtt_pass = DATA_STORAGE.getString(CONF_MQTT_PASS, "");
  MIN_RUNTIME = DATA_STORAGE.getLong(CONF_RUNTIME, 300);
  SENSOR_UPDATE = DATA_STORAGE.getLong(CONF_SENSOR, 10);
  MIN_UPDATE = DATA_STORAGE.getLong(CONF_MIN_UPDATE, 60);
  SWING_TEMP = DATA_STORAGE.getFloat(CONF_SWING_TEMP, 0.5);
  SWING_HUMIDITY = DATA_STORAGE.getInt(CONF_SWING_HUMIDITY, 2);

  RELAY_HEAT = DATA_STORAGE.getInt(CONF_RELAY_HEAT, 25);
  RELAY_COOL = DATA_STORAGE.getInt(CONF_RELAY_COOL, 26);
  RELAY_FAN = DATA_STORAGE.getInt(CONF_RELAY_FAN, 27);
  RELAY_HUMIDIFIER = DATA_STORAGE.getInt(CONF_RELAY_HUMIDIFIER, 14);
  LOW_TRIGGER = DATA_STORAGE.getBool(CONF_LOW_TRIGGER, true);
  DHT_DATA_PIN = DATA_STORAGE.getInt(CONF_DHT22_PIN, 13);

  digitalWrite(RELAY_COOL, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_FAN, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_HEAT, LOW_TRIGGER ? HIGH : LOW);
  digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW);

  pinMode(RELAY_COOL, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_HEAT, OUTPUT);
  pinMode(RELAY_HUMIDIFIER, OUTPUT);
  // dht(DHT_DATA_PIN, DHTTYPE);

  NAME = DATA_STORAGE.getString(CONF_NAME, "HVAC");
  DATA_STORAGE.end();
  char charNAME[NAME.length() + 1];
  NAME.toCharArray(charNAME, sizeof(charNAME));

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
    DATA_STORAGE.begin("thermostat", false);
    if (request->hasArg("ssid"))
    {
      DATA_STORAGE.putString(CONF_SSID, request->arg("ssid"));
    }
    if (request->hasArg("password"))
    {
      DATA_STORAGE.putString(CONF_PASSWORD, request->arg("password"));
    }
    if (request->hasArg("mqtt_server"))
    {
      DATA_STORAGE.putString(CONF_MQTT_SERVER, request->arg("mqtt_server"));
    }
    if (request->hasArg("mqtt_port"))
    {
      DATA_STORAGE.putInt(CONF_MQTT_PORT, request->arg("mqtt_port").toInt());
    }
    if (request->hasArg("mqtt_user"))
    {
      DATA_STORAGE.putString(CONF_MQTT_USER, request->arg("mqtt_user"));
    }
    if (request->hasArg("mqtt_pass"))
    {
      DATA_STORAGE.putString(CONF_MQTT_PASS, request->arg("mqtt_pass"));
    }
    if (request->hasArg("name"))
    {
      DATA_STORAGE.putString(CONF_NAME, request->arg("name"));
    }
    DATA_STORAGE.end();
    if(WiFi.status() != WL_CONNECTED){
    request->redirect("/reboot");
    } else {
    request->redirect("/");
    ESP.restart();
    }    
  });

  server.on("/operation/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    DATA_STORAGE.begin("thermostat", false);

    if (request->hasArg("mode"))
    {
      CURRENT_MODE = request->arg("mode").toInt();
      DATA_STORAGE.putInt(CONF_MODE, CURRENT_MODE);
    }
    if (request->hasArg("fan"))
    {
      FAN_MODE = request->arg("fan").toInt();
      DATA_STORAGE.putInt(CONF_FAN, FAN_MODE);
    }
    AWAY_MODE = request->hasArg("away") ? true : false;
    DATA_STORAGE.putBool(CONF_AWAY, AWAY_MODE);
    HOLD_MODE = request->hasArg("hold") ? true : false;
    DATA_STORAGE.putBool(CONF_HOLD, HOLD_MODE);
    HUMIDIFIER = request->hasArg("humidifier") ? true : false;
    DATA_STORAGE.putBool(CONF_HUMIDIFIER, HUMIDIFIER);
    if (request->hasArg("heat_target"))
    {
      TARGET_HEAT_TEMP = request->arg("heat_target").toInt();
      DATA_STORAGE.putInt(CONF_HEAT_TEMP, TARGET_HEAT_TEMP);
    }
    if (request->hasArg("cool_target"))
    {
      TARGET_COOL_TEMP = request->arg("cool_target").toInt();
      DATA_STORAGE.putInt(CONF_COOL_TEMP, TARGET_COOL_TEMP);
    }
    if (request->hasArg("heat_away_target"))
    {
      TARGET_HEAT_AWAY_TEMP = request->arg("heat_away_target").toInt();
      DATA_STORAGE.putInt(CONF_HEAT_AWAY_TEMP, TARGET_HEAT_AWAY_TEMP);
    }
    if (request->hasArg("cool_away_target"))
    {
      TARGET_COOL_AWAY_TEMP = request->arg("cool_away_target").toInt();
      DATA_STORAGE.putInt(CONF_COOL_AWAY_TEMP, TARGET_COOL_AWAY_TEMP);
    }
    if (request->hasArg("hold_target"))
    {
      TARGET_HOLD_TEMP = request->arg("hold_target").toInt();
      DATA_STORAGE.putInt(CONF_HOLD_TEMP, TARGET_HOLD_TEMP);
    }
    if (request->hasArg("swing_temp"))
    {
      SWING_TEMP = request->arg("swing_temp").toFloat();
      DATA_STORAGE.putFloat(CONF_SWING_TEMP, SWING_TEMP);
    }
    if (request->hasArg("swing_humidity"))
    {
      SWING_HUMIDITY = request->arg("swing_humidity").toInt();
      DATA_STORAGE.putInt(CONF_SWING_HUMIDITY, SWING_HUMIDITY);
    }
    if (request->hasArg("min_runtime"))
    {
      MIN_RUNTIME = request->arg("min_runtime").toInt();
      DATA_STORAGE.putInt(CONF_RUNTIME, MIN_RUNTIME);
    }
    if (request->hasArg("min_update"))
    {
      MIN_UPDATE = request->arg("min_update").toInt();
      DATA_STORAGE.putInt(CONF_MIN_UPDATE, MIN_UPDATE);
    }
    if (request->hasArg("sensor_update"))
    {
      SENSOR_UPDATE = request->arg("sensor_update").toInt();
      DATA_STORAGE.putInt(CONF_SENSOR, SENSOR_UPDATE);
    }
    DATA_STORAGE.end();
    request->redirect("/operation");
  });

  server.on("/relay/set", HTTP_POST, [](AsyncWebServerRequest *request) {
    DATA_STORAGE.begin("thermostat", false);

    if (request->hasArg("heat_relay"))
    {
      RELAY_HEAT = request->arg("heat_relay").toInt();
      DATA_STORAGE.putInt(CONF_RELAY_HEAT, RELAY_HEAT);
    }
    if (request->hasArg("cool_relay"))
    {
      RELAY_COOL = request->arg("cool_relay").toInt();
      DATA_STORAGE.putInt(CONF_RELAY_COOL, RELAY_COOL);
    }
    if (request->hasArg("fan_relay"))
    {
      RELAY_FAN = request->arg("fan_relay").toInt();
      DATA_STORAGE.putInt(CONF_RELAY_FAN, RELAY_FAN);
    }
    if (request->hasArg("humidifier_relay"))
    {
      RELAY_HUMIDIFIER = request->arg("humidifier_relay").toInt();
      DATA_STORAGE.putInt(CONF_RELAY_HUMIDIFIER, RELAY_HUMIDIFIER);
    }
    if (request->hasArg("dht22_pin"))
    {
      DHT_DATA_PIN = request->arg("dht22_pin").toInt();
      DATA_STORAGE.putInt(CONF_DHT22_PIN, DHT_DATA_PIN);
    }
    LOW_TRIGGER = request->hasArg("low_trigger") ? true : false;
    DATA_STORAGE.putBool(CONF_LOW_TRIGGER, LOW_TRIGGER);
    DATA_STORAGE.end();

    digitalWrite(RELAY_HEAT, LOW_TRIGGER ? HIGH : LOW);
    digitalWrite(RELAY_COOL, LOW_TRIGGER ? HIGH : LOW);
    digitalWrite(RELAY_FAN, LOW_TRIGGER ? HIGH : LOW);
    digitalWrite(RELAY_HUMIDIFIER, LOW_TRIGGER ? HIGH : LOW);
    pinMode(RELAY_COOL, OUTPUT);
    pinMode(RELAY_FAN, OUTPUT);
    pinMode(RELAY_HEAT, OUTPUT);
    pinMode(RELAY_HUMIDIFIER, OUTPUT);
    request->redirect("/relay");
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg("reset") && request->arg("reset") == "FACTORY RESET")
    {
      DATA_STORAGE.begin("thermostat", false);
      DATA_STORAGE.clear();
      DATA_STORAGE.end();
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
    DHT dht(DHT_DATA_PIN, DHTTYPE);      // SET UP THE DHT22
    dht.begin();
    
    float h = dht.readHumidity();        // READ THE HUMIDITY
    float t = dht.readTemperature(true); // READ THE TEMPERATURE IN FAHRENHEIT
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
