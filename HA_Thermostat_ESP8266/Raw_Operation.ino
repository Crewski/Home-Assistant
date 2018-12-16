/*
<html lang=en-EN>
        <meta content=60 http-equiv=refresh>
        <title>ESP32 HVAC</title>
        <style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;padding:10px}input{text-align:center}span{padding-left:30px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style>
        <h1>%NAME%</h1>
        <h2>THERMOSTAT</h2>
        <h3>Operation Settings</h3>
        <form action=operation/set method=POST>
            <table>
                <tr>
                    <td>Mode:<td colspan=2><span><input type=radio name=mode value=0 %off_mode%>Off</span> <span><input type=radio
                                name=mode value=1 %heat_mode%>Heat</span> <span><input type=radio name=mode value=2
                                %cool_mode%>Cool</span>
                <tr>
                    <td>Fan:<td><span><input type=radio name=fan value=0 %fan_auto%>Auto</span><span><input type=radio name=fan value=1 %fan_on%>On</span>
                <tr>
                    <td>Hold:<td><input type=checkbox name=hold %HOLD_CHECK%>
                <tr>
                    <td>Away:<td><input type=checkbox name=away %AWAY_CHECK%>
                <tr>
                    <td>Humidifier:<td><input type=checkbox name=humidifier %HUMIDIFIER_CHECK%>
                <tr>
                    <td>
                    <th>Heat<th>Cool<tr>
                    <td>Target:<td><input type=number name=heat_target value="%HEAT_TARGET%">
                    <td><input type=number name=cool_target value="%COOL_TARGET%">
                <tr>
                    <td>Away Target:<td><input type=number name=heat_away_target value="%HEAT_AWAY_TARGET%">
                    <td><input type=number name=cool_away_target value="%COOL_AWAY_TARGET%">
                <tr>
                    <td>Hold Target:<td><input type=number name=hold_target value="%HOLD_TARGET%">
                <tr>
                    <td>Swing Temp:<td><input type=number name=swing_temp value="%SWING_TEMP%" step=0.5>
                <tr>
                    <td>Swing Humidity:<td><input type=number name=swing_humidity value="%SWING_HUMIDITY%">
                <tr>
                    <td>Min Runtime (sec):<td><input type=number name=min_runtime value="%MIN_RUNTIME%"><td>Minimum time furnace/ac will run
                    
                <tr>
                    <td>Min Receive (sec):<td><input type=number name=min_update value="%MIN_UPDATE%"><td>Minimum time between recieving remote temp to switch to local
                <tr>
                    <td>Sensor Update (sec):<td><input type=number name=sensor_update value="%SENSOR_UPDATE%"><td>How often to take/publish local reading
            </table><input type=submit value="SAVE SETTINGS" class=hvac-button>
        </form>
        <form><input type=button value="MAIN MENU" class=hvac-button onclick='window.location.href="/"'></form>
 */
