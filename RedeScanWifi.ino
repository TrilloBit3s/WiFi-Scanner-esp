//http://192.168.4.1
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

const char* apSSID = "ESP32-SCAN";
const char* apPassword = "12345678";

unsigned long lastScan = 0;
const unsigned long scanInterval = 10000;

int networkCount = 0;

String getEncryptionType(wifi_auth_mode_t encryptionType)
{
    switch (encryptionType)
    {
        case WIFI_AUTH_OPEN:
            return "Aberta";

        case WIFI_AUTH_WEP:
            return "WEP";

        case WIFI_AUTH_WPA_PSK:
            return "WPA";

        case WIFI_AUTH_WPA2_PSK:
            return "WPA2";

        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA/WPA2";

        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2 Enterprise";

        default:
            return "Desconhecida";
    }
}

int getSignalQuality(int rssi)
{
    int quality = 0;

    if (rssi <= -100)
        quality = 0;
    else if (rssi >= -50)
        quality = 100;
    else
        quality = 2 * (rssi + 100);

    return quality;
}

String getSignalClass(int quality)
{
    if (quality >= 70)
        return "good";

    if (quality >= 40)
        return "medium";

    return "bad";
}

void scanWiFi()
{
    Serial.println("Escaneando redes...");

    WiFi.scanDelete();

    networkCount = WiFi.scanNetworks();

    Serial.print("Redes encontradas: ");
    Serial.println(networkCount);
}

String generateHTML()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="5">

<title>ESP32 WiFi Scanner</title>

<style>

body{
    background:#0f0f0f;
    color:white;
    font-family:Arial;
    margin:0;
    padding:20px;
}

.container{
    max-width:1200px;
    margin:auto;
}

h1{
    color:#00ff88;
}

table{
    width:100%;
    border-collapse:collapse;
}

th{
    background:#00aa66;
    padding:12px;
    text-align:left;
}

td{
    padding:12px;
    border-bottom:1px solid #333;
}

tr:nth-child(even){
    background:#1a1a1a;
}

.good{
    color:#00ff88;
}

.medium{
    color:orange;
}

.bad{
    color:red;
}

</style>
</head>

<body>

<div class="container">

<h1>ESP32 WiFi Scanner</h1>

<table>

<tr>
<th>#</th>
<th>SSID</th>
<th>RSSI</th>
<th>Canal</th>
<th>BSSID</th>
<th>Segurança</th>
</tr>
)rawliteral";

    if (networkCount == 0)
    {
        html += "<tr><td colspan='6'>Nenhuma rede encontrada</td></tr>";
    }
    else
    {
        for (int i = 0; i < networkCount; i++)
        {
            int rssi = WiFi.RSSI(i);

            int quality = getSignalQuality(rssi);

            String signalClass = getSignalClass(quality);

            html += "<tr>";

            html += "<td>" + String(i + 1) + "</td>";

            html += "<td>" + WiFi.SSID(i) + "</td>";

            html += "<td class='" + signalClass + "'>";
            html += String(rssi) + " dBm";
            html += "</td>";

            html += "<td>" + String(WiFi.channel(i)) + "</td>";

            html += "<td>" + WiFi.BSSIDstr(i) + "</td>";

            html += "<td>";
            html += getEncryptionType(WiFi.encryptionType(i));
            html += "</td>";

            html += "</tr>";
        }
    }

    html += R"rawliteral(
</table>

</div>

</body>
</html>
)rawliteral";

    return html;
}

void handleRoot()
{
    server.send(200, "text/html", generateHTML());
}

void setup()
{
    Serial.begin(115200);

    WiFi.mode(WIFI_AP_STA);

    WiFi.softAP(apSSID, apPassword);

    Serial.println();
    Serial.println("ESP32 WiFi Scanner iniciado");

    Serial.print("SSID: ");
    Serial.println(apSSID);

    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    scanWiFi();

    server.on("/", handleRoot);

    server.begin();

    Serial.println("Servidor HTTP iniciado");
}

void loop()
{
    server.handleClient();

    if (millis() - lastScan > scanInterval)
    {
        lastScan = millis();

        scanWiFi();
    }
}