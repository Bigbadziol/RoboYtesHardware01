#include "setup.h"
#include "YtesWSled.h"

YtesWSled* ledy;


String testDanePrzychodzace() {
    String ret = "";
    ret += "{";
    ret += "\"WLACZONE\" : 1,";
    ret += "\"KOLOR\" : {\"R\":76,\"G\":114,\"B\":8}";
    ret += "}";
    return ret;
};

//-------------------------------------------------------------------------------------------
void parsowaniePolecenia() {
    // obsluga danych przychodzacych
    Serial.println("Dane testowe wchodzace : ");
    Serial.println(testDanePrzychodzace());
    StaticJsonDocument<1024> doc;
    auto error = deserializeJson(doc, testDanePrzychodzace());
    if (!error) {
        JsonObject dane = doc.as<JsonObject>();
        ledy->obslozPolecenieDane(&dane);
    }
    else {
        Serial.println("Blad serializacji polecenia.");
    };
};
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    while (!Serial) continue;
    delay(1000);
    Serial.println("-----------------------------------------------------");
    Serial.println("Test klasy YtesWsLed 1.01");
    ledy = new YtesWSled();
    Serial.println(ledy->odpowiedz());
    ledy->wzorNiePolaczony();
    delay(2000);
    ledy->wylacz();
    delay(4000);
    ledy->wlacz();
    parsowaniePolecenia();
};
//-------------------------------------------------------------------------------------------
void loop() {

};
