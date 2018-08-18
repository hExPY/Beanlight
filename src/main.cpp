#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/*
* Coniguration
** CONNECTION_INDICATOR - blinking led during startup when defined
** ERROR_INDICATOR - blinking led sequence when parsing or http error occured
*/
#define CONNECTION_INDICATOR
#define ERROR_INDICATOR
#define LED_RED GPIO_NUM_14
#define LED_GREEN GPIO_NUM_27
#define LED_BLUE GPIO_NUM_26

#define WIFI_SSID "Your Wifi"
#define WIFI_PASSWORD "Your Wifi Password"
#define ROCKETBEANS_API "https://api.rocketbeans.tv/v1/frontend/init"

/*
* Information
*/
#define __product__ "Beanlight"
#define __version__ "1001DEV"
#define __author__ "Marvyn Zalewski <mszalewski@ownpixel.com>"
#define __copyright__ "(c) 2018 KeyboardInterrupt.org"

/*
* Global Variables
*/
int status = WL_IDLE_STATUS;

/*
* Functions
*/
void setLampToBlack()
{
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
}

void setLampToBlue()
{
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, HIGH);
}

void setLampToGreen()
{
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, LOW);
}

void setLampToCyan()
{
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);
}

void setLampToRed()
{
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
}

void setLampToPurple()
{
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, HIGH);
}

void setLampToYellow()
{
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, LOW);
}

void setLampToWhite()
{
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_BLUE, HIGH);
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
            const size_t bufferSize = JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(8) + 2 * JSON_OBJECT_SIZE(2) + 10 * JSON_OBJECT_SIZE(3) + 5 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(8) + 1190;
            DynamicJsonBuffer jsonBuffer(bufferSize);
            String payload = http.getString();
            JsonObject &root = jsonBuffer.parseObject(payload);
            JsonObject &data = root["data"];
            JsonObject &data_streamInfo = data["streamInfo"];
            JsonObject &data_streamInfo_info = data_streamInfo["info"];
            String data_streamInfo_info_type = data_streamInfo_info["type"].as<String>();

            if (!root.success())
            {
                Serial.println((String)__product__ + " parsing failed.");
                Serial.println(payload);
                #ifdef ERROR_INDICATOR
                for (int i = 0; i <= 6; i++)
                {
                    for (int i = 0; i <= 3; i++)
                    {
                        setLampToCyan();
                        delay(100);
                        setLampToBlack();
                        delay(100);
                    };
                    delay(500);
                }
                #endif
                setLampToCyan();
            } else
            {
                /* When current show is live */
                if (data_streamInfo_info_type == "live")
                    setLampToRed();
                /* When current show is new */
                else if (data_streamInfo_info_type == "premiere")
                    setLampToBlue();
                /* When current show is playback */
                else if (data_streamInfo_info_type == "rerun")
                    setLampToWhite();
                /* Graceful handling if no type matched */
                else {
                    Serial.println((String)__product__ + " unsupported show type -> "+ data_streamInfo_info_type +".");
                    #ifdef ERROR_INDICATOR
                    for (int i = 0; i <= 3; i++)
                    {
                        for (int i = 0; i <= 3; i++)
                        {
                            setLampToPurple();
                            delay(100);
                            setLampToBlack();
                            delay(100);
                        };
                        delay(500);
                    }
                    #endif
                    setLampToPurple();
                }
            }
        }
        else
        {
            Serial.println((String)__product__ + " error on HTTP request");
            #ifdef ERROR_INDICATOR
            for (int i = 0; i <= 3; i++)
            {
                for (int i = 0; i <= 3; i++)
                {
                    setLampToRed();
                    delay(100);
                    setLampToBlack();
                    delay(100);
                    setLampToYellow();
                    delay(100);
                    setLampToBlack();
                    delay(100);
                };
                delay(500);
            }
            #endif
            setLampToYellow();
        }
        http.end();
    }
    delay(60000);
}
