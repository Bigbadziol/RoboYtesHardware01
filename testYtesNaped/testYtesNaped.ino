//Bluetooth joystick free app - zerknac co to :

#include "setup.h"
#include "YtesNaped.h"

YtesNaped *naped;
unsigned long msStop = millis(); // do symulacji klikniec przez uzytkownika

//zmieniona stala w YtesServo.h  MAX_ANGLE = 180 na 360
//wstepne ustawienie msImpuls = 200
void testZlozony() {
    unsigned long msTeraz = millis();
    unsigned long msKoniec = millis();
    Serial.println("Za³ozenia :");
    Serial.println("- do przodu przez 5 sek");
    Serial.println("- do tylu 2 sek");
    Serial.println("- skrecaj przodem w prawo 3 sek (prawe stoi, lewe do przodu)");

    Serial.println("Do przodu...");
    msTeraz = millis();
    msKoniec = msTeraz + (5 * 1000);
    while (msTeraz < msKoniec) {
        naped->ruchPrzod();
        msTeraz = millis();
    }
    Serial.println("Do tylu...");
    msTeraz = millis();
    msKoniec = msTeraz + (2 * 1000);
    while (msTeraz < msKoniec) {
        naped->ruchTyl();
        msTeraz = millis();
    }

    Serial.println("Przodem w prawo...");
    msTeraz = millis();
    msKoniec = msTeraz + (3 * 1000);
    while (msTeraz < msKoniec) {
        naped->ruchPrawoPrzod();
        msTeraz = millis();
    };
    Serial.println("Koniec..."); 
    naped->ruchStop();
}

String kierunekJson() {
    String ret = "";
    ret += "{";
    ret += " \"ruch\" : {";
    ret += " \"PP\" : 1";
    ret += "}}";
    return ret;
}

//Pamietac by w loop dodac jakis sensowny delay(300)
void symulacjaPetliBluetooth() {
    //String ret = bt.readString();
    String ret = "";
    int r = random(5);
    switch (r) {
        case 0: ret = "{\"ruch\":{\"PP\":1}}"; break;
        case 1: ret = "{\"ruch\":{\"P\":1}}"; break;
        case 2: ret = "{\"ruch\":{\"LP\":1}}"; break;
        case 3: ret = "{\"ruch\":{\"LT\":1}}"; break;
        case 4: ret = "{\"ruch\":{\"T\":1}}"; break;
        case 5: ret = "{\"ruch\":{\"PT\":1}}"; break;
    }

    if (ret.length() > 0) {
        DynamicJsonDocument thisJson(JSON_INCOMMING_BUFFER);
        DeserializationError error = deserializeJson(thisJson, ret);
        if (error) {
            PD_ERROR_S("[ROBOT]->Blad : Deserializacja polecenia przychodzacego", error);
        }
        else {
            JsonObject thisData = thisJson.as<JsonObject>();
            JsonVariant cmd = thisData["cmd"];
            if (!cmd.isNull()) {
                String cmdDane = cmd.as<String>();
                PD_INFO_V("Przychadzaca komenda: ", cmdDane);
                if (cmdDane.equalsIgnoreCase("DANE_PROSZE")) {
                    PD_INFO("[ROBOT] -> prosba o dane.");
                    //bt.println("{\"audio\":{}}#$!#");
                }
                else {
                    PD_INFO("[ROBOT] -> nieznana komenda");
                }
            }
            JsonVariant ruch = thisData["ruch"];
            if (!ruch.isNull()) {
                JsonObject ruchDane = ruch.as<JsonObject>();
                naped->obslozPolecenieDane(&ruchDane);
            };
        }
    };
}

void setup() {
    Serial.begin(115200);
    while (!Serial) continue;
    delay(200);
    Serial.println("Test klasy YtesNaped : 1.04");
    Serial.println("Symulacja polecen od apki.");
    naped = new YtesNaped();
    //naped->ruchTyl();
    //delay(400); //300-400 od tego trzeba zaczac - czas impulsu
    //naped->ruchStop();

    //testZlozony();

}



void loop() {
    if (millis() > msStop) {
        //symulacjaPetliBluetooth();
        msStop = millis() + 300L + random(100);
    }
    
    naped->zatrzymaj();
   
}
