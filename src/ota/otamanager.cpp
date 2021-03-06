#include "otamanager.h"
#define NO_GLOBAL_INSTANCES

#ifdef ESP32
#include <ESP32httpUpdate.h>
#else
#include <ESP8266httpUpdate.h>
#endif

#ifdef ESP32

class OTAManagerPrivate
{
public:
    String pendingOta;
    ESP32HTTPUpdate *ota;
};

#else

class OTAManagerPrivate
{
public:
    String pendingOta;
    ESP8266HTTPUpdate *ota;
    WiFiClient *wifiClient;
};

#endif

OTAManager::OTAManager()
{
    d = new OTAManagerPrivate();
}

OTAManager::~OTAManager()
{
    delete d;
}

void OTAManager::update(const String &url)
{
#ifdef ESP32
    d->ota = new ESP32HTTPUpdate();
    d->ota->rebootOnUpdate(true);
#else
    d->ota = new ESP8266HTTPUpdate();
    d->ota->setLedPin(LED_BUILTIN, LOW);
    // Add optional callback notifiers
    d->ota->onStart([](){
      Serial.println("Update started");
    });
    d->ota->onEnd([](){
      Serial.println("Update finished");
    });
    d->ota->onProgress([](int cur, int total){
      Serial.print("Update progress: ");
      Serial.print(cur);
      Serial.print(" of ");
      Serial.println(total);
    });
    d->ota->onError([](int error){
      Serial.println("Update error: " + error);
    });
#endif

    d->pendingOta = url;

}

void OTAManager::loop()
{
    if (d->pendingOta != "") {
#ifdef ESP32
        t_httpUpdate_return ret = d->ota->update(d->pendingOta);

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", d->ota->getLastError(), d->ota->getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK");
                break;
        }

#else
        d->wifiClient = new WiFiClient();
        int ret = d->ota->update(*d->wifiClient, d->pendingOta);
        switch (ret) {
          case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", d->ota->getLastError(), d->ota->getLastErrorString().c_str());
            delete d->ota;
            delete d->wifiClient;
            break;

          case HTTP_UPDATE_NO_UPDATES:
            delete d->ota;
            delete d->wifiClient;
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

          case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
        }
#endif
    }



}
