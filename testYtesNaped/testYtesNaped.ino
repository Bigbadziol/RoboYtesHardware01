#include "setup.h"
#include "YtesNaped.h"
#include "YtesRadar.h"

YtesNaped *naped;
YtesRadar* radar;

long msNastepnyRuch = millis()+ 300L;

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


void testRuchLewe() {
    naped->ruchLewePrzod();
    delay(2000);
    naped->ruchLeweStop();
    delay(2000);
    naped->ruchLeweTyl();
    delay(2000);
    naped->ruchLeweStop();
};

void testRuchPrawe() {
    naped->ruchPrawePrzod();
    delay(2000);
    naped->ruchPraweStop();
    delay(2000);
    naped->ruchPraweTyl();
    delay(2000);
    naped->ruchPraweStop();

};

void testPlynnyRuchLewe() {
    int kierunek = -5;
    int pozycja = 2000; //do przodu pelna moc
    while (1) {
        pozycja += kierunek;
        naped->ruchUs(LEWE, pozycja);
        Serial.print("Lewe us :"); Serial.println(pozycja);
        delay(200);
        if (pozycja == 995) kierunek = 5;
        if (pozycja == 2005) kierunek = -5;
    };
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    while (!Serial) continue;
    delay(200);
    Serial.println("Test klasy YtesNaped : 1.05");
    Serial.println("Symulacja polecen od apki + radar.");
    radar = new YtesRadar(HCSR_TRIG_PIN, HCSR_ECHO_PIN, 20, 2000, SERWO_RADAR_PIN); //2000->4000(wartoœæ domyœlna)
    naped = new YtesNaped();
    naped->dodajRadar(radar);
    naped->wlaczObslugeRadaru(); //zalaczenie blokady ruchu

    Serial.println(naped->odpowiedz());
    //naped->ruchTyl();
    //delay(400); //300-400 od tego trzeba zaczac - czas impulsu
    //naped->ruchStop();

    //testZlozony();

    //testRuchLewe();
    //testRuchPrawe();
    //testPlynnyRuchLewe();

}
//--------------------------------------------------------------------------------
void loop() {
    radar->mierzDystans();
    if (millis() > msNastepnyRuch) {
        msNastepnyRuch = millis() + 300;
        Serial.printf("Odczyt bezposredni : %.2f \n", radar->dystans());
        //naped->ruchPrzod();
    };

    naped->zatrzymaj();
};
