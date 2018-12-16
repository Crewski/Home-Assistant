/*

<html lang=en-EN>
<meta content=60 http-equiv=refresh>
<title>ESP32 HVAC</title>
<style>body{background-color:#fff;padding-top:50px;margin:0 auto;max-width:500px;height:100%%;font-family:Arial,Helvetica,Sans-Serif;Color:#000}table{margin:0 auto;padding-bottom:40px}h1,h2,h3,h4{text-align:center}td{text-align:center;padding:10px}.hvac-button{border:none;border-radius:8px;color:#fff;background-color:#1f45fc;padding:20px;text-align:center;font-size:16px;width:100%%}</style>
<h1>%NAME%</h1>
<h2>THERMOSTAT</h2>
<h3>Relay/Sensor Settings</h3>
<form action=relay/set method=POST>
    <table>
        <tr>
            <td>Low Trigger:<td><input type="checkbox" name="low_trigger" %LOW_TRIGGER%>
        <tr>
            <td>Heat:<td><select name=heat_relay>
                    <option value=0 %heat_relay0%>GPIO 0<option value=1 %heat_relay1%>GPIO 1<option value=2
                        %heat_relay2%>GPIO 2<option value=3 %heat_relay3%>GPIO 3<option value=4 %heat_relay4%>GPIO
                        4<option value=5 %heat_relay5%>GPIO 5<option value=6 %heat_relay6%>GPIO 6<option value=7
                        %heat_relay7%>GPIO 7<option value=8 %heat_relay8%>GPIO 8<option value=9 %heat_relay9%>GPIO
                        9<option value=10 %heat_relay10%>GPIO 10<option value=11 %heat_relay11%>GPIO 11<option
                        value=12 %heat_relay12%>GPIO 12<option value=13 %heat_relay13%>GPIO 13<option value=14
                        %heat_relay14%>GPIO 14<option value=15 %heat_relay15%>GPIO 15<option value=16
                        %heat_relay16%>GPIO 16<option value=17 %heat_relay17%>GPIO 17<option value=18
                        %heat_relay18%>GPIO 18<option value=19 %heat_relay19%>GPIO 19<option value=21
                        %heat_relay21%>GPIO 21<option value=22 %heat_relay22%>GPIO 22<option value=23
                        %heat_relay23%>GPIO 23<option value=25 %heat_relay25%>GPIO 25<option value=26
                        %heat_relay26%>GPIO 26<option value=27 %heat_relay27%>GPIO 27
                </select>
        <tr>
            <td>Cool:<td><select name=cool_relay>
                    <option value=0 %cool_relay0%>GPIO 0<option value=1 %cool_relay1%>GPIO 1<option value=2 %cool_relay2%>GPIO 2<option
                        value=3 %cool_relay3%>GPIO 3<option value=4 %cool_relay4%>GPIO 4<option value=5 %cool_relay5%>GPIO 5<option
                        value=6 %cool_relay6%>GPIO 6<option value=7 %cool_relay7%>GPIO 7<option value=8 %cool_relay8%>GPIO 8<option
                        value=9 %cool_relay9%>GPIO 9<option value=10 %cool_relay10%>GPIO 10<option value=11 %cool_relay11%>GPIO 11<option
                        value=12 %cool_relay12%>GPIO 12<option value=13 %cool_relay13%>GPIO 13<option value=14 %cool_relay14%>GPIO 14<option
                        value=15 %cool_relay15%>GPIO 15<option value=16 %cool_relay16%>GPIO 16<option value=17 %cool_relay17%>GPIO 17<option
                        value=18 %cool_relay18%>GPIO 18<option value=19 %cool_relay19%>GPIO 19<option value=21 %cool_relay21%>GPIO 21<option
                        value=22 %cool_relay22%>GPIO 22<option value=23 %cool_relay23%>GPIO 23<option value=25 %cool_relay25%>GPIO 25<option
                        value=26 %cool_relay26%>GPIO 26<option value=27 %cool_relay27%>GPIO 27
                </select>
        <tr>
            <td>Fan:<td><select name=fan_relay>
                    <option value=0 %fan_relay0%>GPIO 0<option value=1 %fan_relay1%>GPIO 1<option value=2 %fan_relay2%>GPIO 2<option
                        value=3 %fan_relay3%>GPIO 3<option value=4 %fan_relay4%>GPIO 4<option value=5 %fan_relay5%>GPIO 5<option
                        value=6 %fan_relay6%>GPIO 6<option value=7 %fan_relay7%>GPIO 7<option value=8 %fan_relay8%>GPIO 8<option
                        value=9 %fan_relay9%>GPIO 9<option value=10 %fan_relay10%>GPIO 10<option value=11 %fan_relay11%>GPIO 11<option
                        value=12 %fan_relay12%>GPIO 12<option value=13 %fan_relay13%>GPIO 13<option value=14 %fan_relay14%>GPIO 14<option
                        value=15 %fan_relay15%>GPIO 15<option value=16 %fan_relay16%>GPIO 16<option value=17 %fan_relay17%>GPIO 17<option
                        value=18 %fan_relay18%>GPIO 18<option value=19 %fan_relay19%>GPIO 19<option value=21 %fan_relay21%>GPIO 21<option
                        value=22 %fan_relay22%>GPIO 22<option value=23 %fan_relay23%>GPIO 23<option value=25 %fan_relay25%>GPIO 25<option
                        value=26 %fan_relay26%>GPIO 26<option value=27 %fan_relay27%>GPIO 27
                </select>
        <tr>
            <td>Humidifier:<td><select name=humidifier_relay>
                    <option value=0 %humidifier_relay0%>GPIO 0<option value=1 %humidifier_relay1%>GPIO 1<option value=2 %humidifier_relay2%>GPIO 2<option
                        value=3 %humidifier_relay3%>GPIO 3<option value=4 %humidifier_relay4%>GPIO 4<option value=5 %humidifier_relay5%>GPIO 5<option
                        value=6 %humidifier_relay6%>GPIO 6<option value=7 %humidifier_relay7%>GPIO 7<option value=8 %humidifier_relay8%>GPIO 8<option
                        value=9 %humidifier_relay9%>GPIO 9<option value=10 %humidifier_relay10%>GPIO 10<option value=11 %humidifier_relay11%>GPIO 11<option
                        value=12 %humidifier_relay12%>GPIO 12<option value=13 %humidifier_relay13%>GPIO 13<option value=14 %humidifier_relay14%>GPIO 14<option
                        value=15 %humidifier_relay15%>GPIO 15<option value=16 %humidifier_relay16%>GPIO 16<option value=17 %humidifier_relay17%>GPIO 17<option
                        value=18 %humidifier_relay18%>GPIO 18<option value=19 %humidifier_relay19%>GPIO 19<option value=21 %humidifier_relay21%>GPIO 21<option
                        value=22 %humidifier_relay22%>GPIO 22<option value=23 %humidifier_relay23%>GPIO 23<option value=25 %humidifier_relay25%>GPIO 25<option
                        value=26 %humidifier_relay26%>GPIO 26<option value=27 %humidifier_relay27%>GPIO 27
                </select>
        <tr>
            <td>DHT22:<td><select name=dht22_pin>
                    <option value=0 %dht_pin0%>GPIO 0<option value=1 %dht_pin1%>GPIO 1<option value=2 %dht_pin2%>GPIO 2<option
                        value=3 %dht_pin3%>GPIO 3<option value=4 %dht_pin4%>GPIO 4<option value=5 %dht_pin5%>GPIO 5<option
                        value=6 %dht_pin6%>GPIO 6<option value=7 %dht_pin7%>GPIO 7<option value=8 %dht_pin8%>GPIO 8<option
                        value=9 %dht_pin9%>GPIO 9<option value=10 %dht_pin10%>GPIO 10<option value=11 %dht_pin11%>GPIO 11<option
                        value=12 %dht_pin12%>GPIO 12<option value=13 %dht_pin13%>GPIO 13<option value=14 %dht_pin14%>GPIO 14<option
                        value=15 %dht_pin15%>GPIO 15<option value=16 %dht_pin16%>GPIO 16<option value=17 %dht_pin17%>GPIO 17<option
                        value=18 %dht_pin18%>GPIO 18<option value=19 %dht_pin19%>GPIO 19<option value=21 %dht_pin21%>GPIO 21<option
                        value=22 %dht_pin22%>GPIO 22<option value=23 %dht_pin23%>GPIO 23<option value=25 %dht_pin25%>GPIO 25<option
                        value=26 %dht_pin26%>GPIO 26<option value=27 %dht_pin27%>GPIO 27<option value=34 %dht_pin34%>GPIO 34<option
                        value=35 %dht_pin35%>GPIO 35<option value=36 %dht_pin36%>GPIO 36<option value=39 %dht_pin39%>GPIO 39
                </select>
    </table><input class=hvac-button type=submit value="SAVE SETTINGS">
</form>
<form><input class=hvac-button type=button value="MAIN MENU" onclick='window.location.href="/"'></form>
*/