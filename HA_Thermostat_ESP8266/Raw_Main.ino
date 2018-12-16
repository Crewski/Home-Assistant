/*

<html lang=en-EN>
<meta content=60 http-equiv=refresh>
<title>ESP32 HVAC</title>
<style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;font-family:Arial,Helvetica,Sans-Serif;Color:#000}h1,h2,h3{text-align:center}td{text-align:center;width:50%%}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}.alert{background-color:red}.hvac-table{width:80%%;margin:0 auto;padding-bottom:40px}</style>
<h1>%NAME%</h1>
<h2>THERMOSTAT</h2>
<h3>Current data</h3>
<table class=hvac-table>
    <tr>
        <td>Mode:<td>%CURRENT_MODE%<tr>
        <td>Target:<td>%TARGET_TEMP% F<tr>
        <td>Currently:<td>%CURRENT_TEMP% F<tr>
        <td>Away mode:<td>%AWAY_MODE%<tr>
        <td>Hold mode:<td>%HOLD_MODE%<tr>
        <td>Target Humidity:<td>%TARGET_HUMIDITY% %%<tr>
        <td>Current Humidity:<td>%CURRENT_HUMIDITY% %%
</table>
<form><input class=hvac-button onclick='window.location.href="operation"' type=button value="OPERATIONS"></form>
<form><input class=hvac-button onclick='window.location.href="connectivity"' type=button value="CONNECTIVITY"></form>
<form><input class=hvac-button onclick='window.location.href="relay"' type=button value="RELAYS/SENSOR"></form>
<form action=/reset method=POST><input class="hvac-button alert" onclick='return confirm("Do you want to reset back to factory settings?")'
        type=submit value="FACTORY RESET" name=reset></form>
 
 */
