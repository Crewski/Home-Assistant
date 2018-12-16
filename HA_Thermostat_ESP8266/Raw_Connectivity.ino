/*
<html lang=en-EN>
<meta content=60 http-equiv=refresh>
<title>ESP32 HVAC</title>
<style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{width:80%%;margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;width:50%%;padding:10px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style>
<h1>%NAME%</h1>
<h2>THERMOSTAT</h2>
<h3>Connectivity Settings</h3>
<form action=connectivity/set method=POST>
    <table>
        <tr>
            <td>SSID:<td><input value="%SSID%" name=ssid>
        <tr>
            <td>Password:<td><input value="%PASSWORD%" name=password type=password>
        <tr>
            <td>MQTT Server:<td><input value="%MQTT_SERVER%" name=mqtt_server>
        <tr>
            <td>MQTT Port:<td><input value="%MQTT_PORT%" name=mqtt_port>
        <tr>
            <td>MQTT Username:<td><input value="%MQTT_USER%" name=mqtt_user>
        <tr>
            <td>MQTT Password:<td><input value="%MQTT_PASS%" name=mqtt_pass type=password>
        <tr>
            <td>Topic Name<td><input value="%NAME%" name=name>
    </table><input value=Submit type=submit class=hvac-button>
</form>
<form><input value="MAIN MENU" type=button class=hvac-button onclick='window.location.href="/"'></form>
<h4>*Topic Name is used in the mqtt topics</h4>
 */
