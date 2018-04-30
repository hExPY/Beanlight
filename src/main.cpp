#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/*
* Coniguration
** CONNECTION_INDICATOR - blinking led during startup when defined
*/
#define CONNECTION_INDICATOR
#define LED_RED GPIO_NUM_14
#define LED_GREEN GPIO_NUM_27
#define LED_BLUE GPIO_NUM_26

#define WIFI_SSID "WLAN"
#define WIFI_PASSWORD "WLAN_PASSWORD"
#define ROCKETBEANS_API "https://www.rocketbeans.tv/?next5Shows=true"

/*
* Information
*/
#define __product__ "Beanlight"
#define __version__ "1000DEV"
#define __author__ "Marvyn Zalewski <mszalewski@ownpixel.com>"
#define __copyright__ "(c) 2018 KeyboardInterrupt.org"

/*
* Global Variables
*/
int status = WL_IDLE_STATUS;

/*
* Functions
*/
void setLampToRed()
{
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
}

void setLampToBlue()
{
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
}

void setLampToGreen()
{
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_RED, LOW);
}

void setLampToWhite()
{
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_RED, HIGH);
}

void setLampToBlack()
{
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_RED, LOW);
}

/*
* Init
*/
void setup()
{
    Serial.begin(115200);
    Serial.println(__product__);
    Serial.println(__version__);
    Serial.println(__author__);
    Serial.println((String)__copyright__ + "\n");
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    while (status != WL_CONNECTED)
    {
        Serial.println((String)__product__ + " attempting to connect to Wifi network, WIFI_SSID: " + (String)WIFI_SSID);
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        #ifdef CONNECTION_INDICATOR
        setLampToRed();
        delay(2000);
        setLampToBlue();
        delay(2000);
        setLampToGreen();
        delay(2000);
        #else
        delay(6000);
        #endif
    };
    #ifdef CONNECTION_INDICATOR
    for (int i = 0; i <= 3; i++)
    {
        setLampToGreen();
        delay(250);
        setLampToBlack();
        delay(250);
    };
    #endif
    Serial.println((String)__product__ + " connected to network");
}

void loop()
{
    if ((WiFi.status() == WL_CONNECTED))
    {
        HTTPClient http;
        http.begin(ROCKETBEANS_API);
        int httpCode = http.GET();
        if (httpCode > 0)
        {
            const size_t bufferSize = JSON_ARRAY_SIZE(5) + 5 * JSON_OBJECT_SIZE(14) + 2970;
            DynamicJsonBuffer jsonBuffer(bufferSize);
            String payload = http.getString();
            JsonArray &root = jsonBuffer.parseArray(payload);
            if (!root.success())
            {
                Serial.println((String)__product__ + " parsing failed.");
                Serial.println(payload);
            }
            /* When current show is live */
            if (root[0]["isLive"] == 1)
                setLampToRed();
            /* When current show is new */
            else if (root[0]["isNew"] == 1)
                setLampToBlue();
            /* When current show is playback */
            else
                setLampToWhite();
        }
        else
        {
            Serial.println((String)__product__ + " error on HTTP request");
        }
        http.end();
    }
    delay(60000);
}
