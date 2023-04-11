#include "setup.h"
#include "YtesRadar.h"

YtesRadar* radar;
long msWypiszPomiar = millis();
/*
{"KAT" : 90,
"RUCH180":{
    "STARTKAT":90,
    "WYKONAJ": 1
}}

*/

String testAktywujRuch180() {
    String ret;
    ret.concat("{\"KAT\":90,");
    ret.concat("\"RUCH180\":{");
    ret.concat("\"STARTKAT\":90,");
    ret.concat("\"WYKONAJ\":1");
    ret.concat("}}");
    return ret;
};
//-------------------------------------------------------------------------------------------
void parsowaniePolecenia() {
    // obsluga danych przychodzacych
    Serial.println("Dane testowe wchodzace : ");
    Serial.println(testAktywujRuch180());
    StaticJsonDocument<1024> doc;
    auto error = deserializeJson(doc, testAktywujRuch180());
    if (!error) {
        JsonObject dane = doc.as<JsonObject>();
        radar->obslozPolecenieDane(&dane);
    }
    else {
        Serial.println("Blad serializacji polecenia.");
    };
};
void setup() {
    Serial.begin(115200);
    while (!Serial) continue;
    delay(100);
    Serial.println("--------------------------------");
    Serial.println("test YtesRadar 1.03");
    Serial.println("Zabawa z serwem i HC-SR04");
    Serial.println("Przykladowa demonstracja ruchu niezaleznego.");
    
    radar = new YtesRadar(HCSR_TRIG_PIN, HCSR_ECHO_PIN, 20, 2000, SERWO_RADAR_PIN); //org : 4000-> 2000
    radar->ustawRadar(90);

    Serial.println(radar->odpowiedz().c_str());
    //radar->ruch180Inicjuj(90);
    parsowaniePolecenia();

};

void loop() {

    //Pomiar dystansu
    radar->mierzDystans();
    if (millis() > msWypiszPomiar) {
        msWypiszPomiar = millis() + 500;
        Serial.printf("Odleglosc : %.2f \n",radar->dystans());
    }
    radar->ruch180Krok();
}
