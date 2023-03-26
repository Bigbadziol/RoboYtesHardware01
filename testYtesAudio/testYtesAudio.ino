#include <splash.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
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

//Posiadane scalaki :
// 1) DFROBOTILISP3 lub DFROBOT|LISP3 wzmacniacz YX8002D -> https://www.amazon.ca/DFPlayer-A-Mini-MP3-Player/dp/B089D5NLW1
// 2) MH2024K-16SS / 8002A-XJS
// 3)
// 4)  GD3200D
//moj problem:
//https://www.youtube.com/watch?v=LJXM1KGWEKg

//BARDZO LADNIE OPISANE : 
//https://garrysblog.com/2022/06/12/mp3-dfplayer-notes-clones-noise-speakers-wrong-file-plays-and-no-library/
/*
* Pamiêtaæ nale¿y , ¿e nie mamy faktycznie wplywu na kolejnoœæ interpretacji kolejnoœci parametrów
* W zale¿noœci od biblioteki (android/esp) sk³adowa przykladowo bedzie wygenerowana w innym miejscu.
* Istotna jest wartoœæ informacji nie kolejnoœæ umiejscowienia w obiekcie.
*/

//Konwerter z ktorego kozystalismy
//https://www.konwerter.net/results/

YtesAudio* audio;

String testUstawGlosnosc() {
    String ret = "";
    ret += "{";
    ret += "\"LG\" : 10,";
    ret += "\"PG\" : 12";
    ret += "}";
    return ret;
}



bool efekt1 = false;
bool efekt2 = false;
bool efekt3 = false;
bool efekt4 = false;

static unsigned long timerLewy = millis();
static unsigned long timerPrawy = millis();
unsigned long msStart = millis();
unsigned long msTeraz = millis();


void parsowaniePolecenia() {
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
}

void setup() {
    Serial.begin(115200);
    while (!Serial) continue;
    delay(200);
    serialLewy.begin(9600);
    serialPrawy.begin(9600);

    Serial.println("Test klasy YtesAudio : 1.03");
    audio = new YtesAudio(&serialLewy, &serialPrawy, 350);
    // zainicjalizowane ustawienia
    String odpowiedz = audio->odpowiedz();
    Serial.println("Odpowiedz ustawienia bazowe :");
    Serial.println(odpowiedz.c_str());

    audio->ustawTor(BEZ_CENZURY);
    audio->ustawTrybAudio(NORMALNY);
    delay(350);

    //audio->graj(LEWY, MUZYKA, 2);
    //testy :  graj muzyke 2 , obliczony index 10 , slysze bodies
    audio->grajMuzyke(2);

    msStart = millis();
};


void loop() {
    msTeraz = millis();

    if (msTeraz > (msStart + 5000)) {
        if (efekt1 == false) {
            efekt1 = true;
            audio->grajEfekt(4);
        };
    };
/*
    if (msTeraz > (msStart + 10000)) {
        if (efekt4 == false) {
            efekt4 = true;
            audio->playerPrawy.volume(5);
        }
    }
*/
    if (msTeraz > (msStart + 15000)){
        if (efekt2 == false) {
            efekt2 = true;
            audio->grajEfekt(3);
        };
    };

    if (msTeraz > (msStart + 25000)) {
        if (efekt3 == false) {
            efekt3 = true;
            audio->grajEfekt(1);
        };
    };


/*
      if (millis() - timerPrawy > 5000) {
        timerPrawy = millis();
        Serial.println("PRAWY : Nastepne nagranie.");
        audio->playerPrawy.playNext();
      }

      if (millis() - timerLewy > 5000) {
          timerLewy = millis();
          bool gram = audio->playerLewy.isPlaying();
          Serial.print("odtwarzam    : ");
          Serial.println(gram);
          Serial.println("LEWY : Nastepne nagranie.");
          audio->playerLewy.playNext();
      }
*/
    audio->audioHandler();
};
