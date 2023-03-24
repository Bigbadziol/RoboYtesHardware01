#include "setup.h"
#include "YtesAudio.h"
#include <ArduinoJson.h>

//PRAWY -DFPLAYER MINI
// LEWY - MP3-TF-16P

//dobry chip : YX5200 ??
//PRZETESTOWAC ZMIANE Hardware na serialPort
// 
//https://www.youtube.com/watch?v=kq2RLz65_w0// 
//REZYSTOR?!?! RX-GND pulldown 100k-1M (przy zasilaniu 3.3V) , goœæ u¿ywa HardwareSerial (1), potem b egin, mamy to samo
//gdzie indziej 1K

//moj problem:
//https://www.youtube.com/watch?v=LJXM1KGWEKg
YtesAudio* audio;

//BARDZO LADNIE OPISANE : 
//https://garrysblog.com/2022/06/12/mp3-dfplayer-notes-clones-noise-speakers-wrong-file-plays-and-no-library/
/*
* Pamiêtaæ nale¿y , ¿e nie mamy faktycznie wplywu na kolejnoœæ interpretacji kolejnoœci parametrów
* W zale¿noœci od biblioteki (android/esp) sk³adowa przykladowo bedzie wygenerowana w innym miejscu.
* Istotna jest wartoœæ informacji nie kolejnoœæ umiejscowienia w obiekcie.
*/

/*
String testDuzoDanych() {
    //nie mamy wplywu czy glosnosc zostanie ustawiona przed czy po poleceniu odegrania konkretnego nagrania
    String ret = "";
    ret += "{";
    ret += "\"LG\" : 10,";
    ret += "\"PG\" : 10,";
    ret += "\"LN\" : 1,";
    ret += "\"PN\" : 4";
    ret += "}";
    return ret;
}
*/
String testUstawGlosnosc() {
    String ret = "";
    ret += "{";
    ret += "\"LG\" : 10,";
    ret += "\"PG\" : 12";
    ret += "}";
    return ret;
}

String testReset() {
    String ret = "";
    ret += "{";
    ret += "\"RESET\": 1";
    ret += "}";
}

unsigned long msTestStart;

void setup() {
    Serial.begin(115200);
    while (!Serial) continue;
    delay(200);
    serialLewy.begin(9600, SERIAL_8N1, SERIAL2_LEWY_RX, SERIAL2_LEWY_TX);
    serialPrawy.begin(9600, SERIAL_8N1, SERIAL1_PRAWY_RX, SERIAL1_PRAWY_TX);
    Serial.println("Test klasy YtesAudio : 1.03");
    audio = new YtesAudio(&serialLewy,&serialPrawy);
    // zainicjalizowane ustawienia
    String odpowiedz = audio->odpowiedz();
    Serial.println("Odpowiedz ustawienia bazowe :");
    Serial.println(odpowiedz.c_str());


    // obsluga danych przychodzacych
    Serial.println("Dane testowe wchodzace : ");
    Serial.println(testUstawGlosnosc());
    StaticJsonDocument<1024> doc;
    auto error = deserializeJson(doc, testUstawGlosnosc());
    if (!error) {
        JsonObject dane = doc.as<JsonObject>();
        audio->obslozPolecenieDane(&dane);
    }
    else {
        Serial.println("Blad serializacji polecenia.");
    };
    //ponowna odpowiedz po nowych ustawieniach
    //odpowiedz = audio->odpowiedz();
    //Serial.println("Odpowiedz zmiany :");
    //Serial.println(odpowiedz.c_str());

    //LEWY SPAMUJE SERIAL PO ZAKONCZENIU grania , prawy nie reaguje na glosnosc(zmiane)
    //audio->glosnosc(PRAWY, 10);
    audio->ustawTrybAudio(NORMALNY);
    audio->graj(LEWY, MUZYKA, 4); // 4- fight back
    //audio->graj(PRAWY, MUZYKA, 6); // 6 - du hast
    //audio->graj(LEWY, EFEKT, 8); // 8- pila
    audio->graj(PRAWY, EFEKT, 8); //pila

    //audio->grajEfekt(7);
}


void loop() {
    audio->uaktualnij();
    delay(1000);
};
