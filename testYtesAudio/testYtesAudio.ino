#include "setup.h"
#include "YtesAudio.h"
#include "YtesZyroskop.h"
#include "YtesRadar.h"

#include <ArduinoJson.h>

//TODO: porzetestowac dla pewnosci w wariancie HardwareSerial

//1)
//https://www.youtube.com/watch?v=kq2RLz65_w0// 
//REZYSTOR?!?! RX-GND pulldown 100k-1M (przy zasilaniu 3.3V) , goœæ u¿ywa HardwareSerial (1), potem b egin, mamy to samo
//gdzie indziej 1K

//2)
//Posiadane scalaki :
// 1) DFROBOTILISP3 lub DFROBOT|LISP3 wzmacniacz YX8002D -> https://www.amazon.ca/DFPlayer-A-Mini-MP3-Player/dp/B089D5NLW1
// 2) MH2024K-16SS / 8002A-XJS
// 3)
// 4)  GD3200D
// Dobre chipy -> YX5200 
//moj problem:
//https://www.youtube.com/watch?v=LJXM1KGWEKg

//3)
//BARDZO LADNIE OPISANE : 
//https://garrysblog.com/2022/06/12/mp3-dfplayer-notes-clones-noise-speakers-wrong-file-plays-and-no-library/

//4) Dla Filipa
/*
* Pamiêtaæ nale¿y , ¿e nie mamy faktycznie wplywu na kolejnoœæ interpretacji kolejnoœci parametrów
* W zale¿noœci od biblioteki (android/esp) sk³adowa przykladowo bedzie wygenerowana w innym miejscu.
* Istotna jest wartoœæ informacji nie kolejnoœæ umiejscowienia w obiekcie.
*/

//5)
//Konwerter z ktorego kozystalismy
//https://www.konwerter.net/results/

YtesAudio* audio;
YtesZyroskop *zyroskop;
YtesRadar* radar;

String testUstawGlosnosc() {
    String ret = "";
    ret += "{";
    ret += "\"LG\" : 10,";
    ret += "\"PG\" : 12";
    ret += "}";
    return ret;
}

static unsigned long timerLewy = millis();
static unsigned long timerPrawy = millis();
static unsigned long msTeraz = millis();


//-------------------------------------------------------------------------------------------
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
};
//-------------------------------------------------------------------------------------------
void losowyEfektDzwiekowy(byte iloscDzwiekow) {
    int los = random(1, iloscDzwiekow + 1);    
    audio->grajEfekt(los);
};
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void setup() {
    Wire.begin();               //Potrzebne zyroskopowi
    randomSeed(analogRead(A0)); //Potrzebne zyroskopowi

    Serial.begin(115200);
    while (!Serial) delay(100);

#if UZYJ_HARDWARE == 1
    PD_INFO("Uwaga, komunikacja UART - HardwareSerial");
    serialLewy.begin(9600, SERIAL_8N1, SERIAL2_LEWY_RX, SERIAL2_LEWY_TX);
    serialPrawy.begin(9600, SERIAL_8N1, SERIAL1_PRAWY_RX, SERIAL1_PRAWY_TX);
#else
    PD_INFO("Uwaga, komunikacja UART - SoftwareSerial");
    serialLewy.begin(9600);
    serialPrawy.begin(9600);
#endif 
    AUDIO_INFO("Test klasy YtesAudio : 1.08");
    AUDIO_INFO("Problem prawy glosnik");
    zyroskop = new YtesZyroskop(&mpu, 100); 
    radar = new YtesRadar(HCSR_TRIG_PIN, HCSR_ECHO_PIN, 20, 4000, SERWO_RADAR_PIN);
    
    audio = new YtesAudio(&serialLewy, &serialPrawy, 350);
    audio->dodajZyroskop(zyroskop);
    audio->dodajRadar(radar);
    audio->uwzglednijZyroskop = false;
    audio->uwzglednijRadar = false;
    
    
    // zainicjalizowane ustawienia
    String odpowiedz = audio->odpowiedz();
    Serial.println("Odpowiedz ustawienia bazowe :");
    Serial.println(odpowiedz.c_str());

    audio->ustawTor(BEZ_CENZURY);
    audio->ustawTrybAudio(WYCISZANIE);
    delay(350);

    //audio->graj(LEWY, MUZYKA, 2);
    //testy :  graj muzyke 2 , obliczony index 10 , slysze bodies
    
    //audio->graj(LEWY, MUZYKA, 2); //gra muze , gra dzwieki.
    audio->graj(PRAWY, MUZYKA, 2);
    
    //audio->grajMuzyke(2);
    //audio->grajEfekt(8);


#if UZYJ_HARDWARE == 0      //Tylko dla SoftwareSerial
    serialLewy.listen();
#endif
    
};
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void loop() {
    msTeraz = millis();

    //Losowe efekty dzwiekowe , dla roznych trybow ustawienia obiektu audio
    if (millis() - timerPrawy > 8000) {
        timerPrawy = millis();
       losowyEfektDzwiekowy(5);
    };

/*
      //------------------------------------------------------------------------------------------------
      //Kolejne utwory z karty SD
      //Uwaga mp3Player - indexuje nagrania w swoj unikalny pojebany sposob, nie zawsze s¹ to indexy narasataj¹co
      //w rozumieniu : kat 01/5 utworow to w kat 02/ zaczynamy od 6. Aby bylo zabawniej w zale¿noœci od typu scalaka
      //(mamy w ofercie bodaj 7 roznych) indexowanie moze wygladac zupelnie inaczej. Sport dla masochistow.
      //------------------------------------------------------------------------------------------------
      if (millis() - timerPrawy > 5000) {
          timerPrawy = millis();
          bool gram = audio->playerPrawy.isPlaying();
          Serial.printf(" PRAWY : Aktualnie odtwarzam :%d \n",gram);
          Serial.println("PRAWY : Nastepne nagranie.");
          audio->playerPrawy.playNext();
      }

      if (millis() - timerLewy > 5000) {
          timerLewy = millis();
          bool gram = audio->playerLewy.isPlaying();
          Serial.printf(" LEWY : Aktualnie odtwarzam :%d \n",gram);          
          Serial.println("LEWY : Nastepne nagranie.");
          audio->playerLewy.playNext();
      }
*/
    zyroskop->uaktualnijOdczyty();
    audio->audioHandler();
};
