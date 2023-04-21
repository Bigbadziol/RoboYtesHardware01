//15.24.2023-dom
/*
* // Uwaga, przyjrzec sie zachowaniu GPIO0 - przyszly radar  , ze wgledu na klod w YtesServo.h moze generowac blad
* //This is bold text to get your attention, I can make it red as well: It is "\r\n" not "\n\r". <--NA LEDZIKU SPRAWDZIC
*/


// Testowe uk³ady , upewnic sie !
// ESP32 - mini 01) AC:67:B2:2D:16:92
//  C8:F0:9E:F4:C6:3E , a poelutownym MPU6050


#include "setup.h"
#include "YtesNaped.h"
#include "YtesRadar.h"
#include "YtesZyroskop.h"
#include "YtesWSled.h"
#include "YtesAudio.h"

#include "esp_bt_main.h"
#include "esp_bt_device.h"
//#include "BluetoothSerial.h" 
#include "BdlBluetoothSerial.h" // uwaga dla zgodnosci z ledzikiem
#include <FastLED.h>


const char* nazwaUrzadzenia = "YtesRobot02";
const char* pinUrzadzenia = "0987";

//HardwareSerial serialLewy(2);// - urzyj uart2 (technicznie piny 16,17); - do komunikacji z playerLewy
//HardwareSerial serialPrawy(1);// - mapuj uart1 (22,14); - do komunikacji z playerPrawy


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth wylaczone na poziomie ROTSA , menuconfig sie klania.
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth wylaczone , menuconfig sie klania.
#endif


YtesRadar* radar;
YtesNaped* naped;
YtesZyroskop* zyroskop;
YtesWSled* ledy;
YtesAudio* audio;
BdlBluetoothSerial bt; //zamiennie do orginalnej biblioteki

//---------------Bufor bluetooth---------------------------
const uint16_t numChars = 512;
char receivedChars[numChars];
boolean newData = false;
static uint16_t ndx = 0;
char endMarker = '\n';
char rc;
//--------------------------------------------------------------------------------------------
unsigned long msRadarOdczytTest = millis();
unsigned long msZyroskopTest = millis();
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
String mojMac() {
    String tmp = "";
    const uint8_t* point = esp_bt_dev_get_address();
    for (int i = 0; i < 6; i++) {
        char str[3];
        sprintf(str, "%02X", (int)point[i]);
        tmp += str;
        if (i < 5) tmp += ":";
    }
    return tmp;
};
//--------------------------------------------------------------------------------------------
void AuthCompleteCallback(boolean success) {
#if SERIAL_PD == 1
    if (success) {
        Serial.println("Parowanie zakonczone sukcesem.");
    }
    else {
        Serial.println("Parowanie nie udane.");
    };
#endif
};
//--------------------------------------------------------------------------------------------
String pelnaOdpowiedz() {
    String ret = "";
    ret.reserve(512); //zerknac ile faktycznie teraz jest danych i dodaæ 50%
    ret.concat("{");
    ret.concat(naped->odpowiedz());
    ret.concat(",");
    ret.concat(audio->odpowiedz());
    ret.concat(",");
    ret.concat(ledy->odpowiedz());
    ret.concat(",");
    ret.concat(radar->odpowiedz());
    ret.concat("}");
    ret.concat("#$!#"); // ten znacznik rozpoznaje appka , jest to informacja o koncu bloku danych
    return ret;
};
//--------------------------------------------------------------------------------------------
void stanNiePolaczony() {
    ledy->wzorNiePolaczony();
    audio->grajEfekt(0); //zatrzymaj jesli cos gra
    audio->grajMuzykePowitalna();
    audio->uwzglednijRadar = false;
    audio->uwzglednijZyroskop = false;
    audio->ustawSparowanie(false);
};
//--------------------------------------------------------------------------------------------
void stanPolaczony() {
    ledy->wzorPolaczony();
    audio->ustawSparowanie(true);
    audio->grajMuzyke(audio->indexMuzykaKatalog()); //utwór ustawiony przez u¿ytkownika
};
// --------------- Funkcje testowe -----------------------------------------------------------
void radarRuch180() {
    for (int i = 0; i <= 180l; i++) {
        radar->ustawRadar(i);
        delay(25);
    };
};
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void setup() {
    randomSeed(analogRead(A0)); //pamietac do generatora!!
    Wire.begin(); //Pamietac!! do glownego

    //Serial do wyœwietlania informacji debugowych
#if SERIAL_PD == 1
    Serial.begin(115200);
    while (!Serial) continue;
    delay(1000);
#endif

#if UZYJ_HARDWARE == 1
    //PD_INFO("Uwaga, komunikacja UART - HardwareSerial");
    serialLewy.begin(9600, SERIAL_8N1, SERIAL2_LEWY_RX, SERIAL2_LEWY_TX);
    serialPrawy.begin(9600, SERIAL_8N1, SERIAL1_PRAWY_RX, SERIAL1_PRAWY_TX);
#else
    //PD_INFO("Uwaga, komunikacja UART - SoftwareSerial");
    serialLewy.begin(9600);
    serialPrawy.begin(9600);
#endif 

    PD_INFO("-----------------------------------------------------");
    PD_INFO("RoboYtesHardware 1.22");
    PD_INFO("Bardzo duze zmiany w YtesAudio, YtesAudioPolecenie");
    PD_INFO("--");

    bt.onAuthComplete(AuthCompleteCallback);
    bt.setPin(pinUrzadzenia);
    bt.begin(nazwaUrzadzenia, false); //Bluetooth device name
    PD_INFO_V("Urzadzenie:", nazwaUrzadzenia);
    PD_INFO_V("Pin:", pinUrzadzenia);
    PD_INFO_S("Mac:", mojMac());
    PD_INFO("Robot gotowy do parowania.");
    
    ledy = new YtesWSled();
    radar = new YtesRadar(HCSR_TRIG_PIN, HCSR_ECHO_PIN, 20, 2000, SERWO_RADAR_PIN);

    zyroskop = new YtesZyroskop(&mpu, 100); // 0 - kazdy przebieg petli , wiêksza wartoœæ - pomiar co okreœlony czas w ms.
    
    naped = new YtesNaped();
    naped->dodajRadar(radar);
    naped->wlaczObslugeRadaru(); //uwzglêdnij w poleceniu ruchu , mo¿liwoœæ zablokowania go przez przeszkodê.

    audio = new YtesAudio(&serialLewy, &serialPrawy, 500); // UWAGA ! zmiana z 350
    audio->dodajZyroskop(zyroskop);
    audio->dodajRadar(radar);
    audio->dodajNaped(naped);

    audio->uwzglednijNude = true;
    audio->uwzglednijZyroskop = true;
    audio->uwzglednijRadar = true;
    audio->uwzglednijNaped = true;
    audio->ustawTor(CENZURA);
    audio->ustawTrybAudio(WYCISZANIE);
    audio->ustawCzasNudy(12000L);
    audio->ustawCzasPrzerwaGadanie(5000L, 5000L);

    stanNiePolaczony(); //Oswietlenie utwor - przed po³¹czeniem z aplikacj¹

#if UZYJ_HARDWARE == 0      //Tylko dla SoftwareSerial
    serialLewy.listen();
#endif

   
    //Serial.println("TEST PELNEJ ODPOWIEDZI :");
    //Serial.println(pelnaOdpowiedz());
};

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void loop() {
    //---------------------Odczyt radar -  zasieg---------------------------------------
    radar->mierzDystans();
    radar->ruch180Krok();
/*
    if (millis() > msRadarOdczytTest) {
        msRadarOdczytTest = millis() + 1000;
        Serial.printf("Testowy odczyt odleglosci : %.2f \n", radar->dystans());
    };
*/
    //--------------------Zyroskop odczyt pozycji---------------------------------------
    zyroskop->uaktualnijOdczyty();
/*
    if (millis() > msZyroskopTest) {
        msZyroskopTest = millis() + 500;
        zyroskop->wypiszKluczowePomiary();
    };
*/
    //--------------------Audio---------------------------------------------------------
    audio->audioHandler();

    //---------------------BLUETOOTH----------------------------------------------------
    //zaladuj dane do bufora
    while (bt.available() > 0 && newData == false) {
        rc = bt.read();
        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }else {
            receivedChars[ndx] = '\0'; // koniec stringa
            ndx = 0;
            newData = true;
        };
    };
    // znak \n odnaleziony - mozemy dzialac dalej
    if (newData == true) {
        Serial.print("Dane surowe :  "); Serial.println(receivedChars);

        DynamicJsonDocument thisJson(JSON_INCOMMING_BUFFER);
        DeserializationError error = deserializeJson(thisJson, receivedChars);
        if (error) {
            PD_ERROR_S("[ROBOT]->Blad : Deserializacja polecenia przychodzacego", error);
        }
        else {
            JsonObject thisData = thisJson.as<JsonObject>();
            //prosba o poadanie danych konfiguracyjnych
            JsonVariant cmd = thisData["cmd"];                
            if (!cmd.isNull()) {
                String cmdDane = cmd.as<String>();
                PD_INFO_V("Przychadzaca komenda: ", cmdDane);
                if (cmdDane.equalsIgnoreCase("DANE_PROSZE")) {
                    PD_INFO("[ROBOT] -> prosba o dane.");
                    bt.println(pelnaOdpowiedz());
                    //bt.println("{\"audio\":{}}#$!#");
                    stanPolaczony(); //zaswiec lampkami , utwor wybrany , itp
                }
                else if (cmdDane.equalsIgnoreCase("PAPA")) {
                    PD_INFO("[Robot] -> rozlaczenie aplikacji");
                    stanNiePolaczony();
                }
                else {
                    PD_INFO("[ROBOT] -> nieznana komenda");
                }
            }
            //polecenia dla modulu 'naped'
            JsonVariant vNaped = thisData["naped"];
            if (!vNaped.isNull()) {
                PD_INFO("[ROBOT] -> obiekt naped");
                JsonObject daneNaped = vNaped.as<JsonObject>();
                naped->obslozPolecenieDane(&daneNaped);
            };
            //polecenie dla modulu 'audio'
            JsonVariant vAudio = thisData["audio"];
            if (!vAudio.isNull()) {
                PD_INFO("[ROBOT]-> obiekt audio");
                JsonObject daneAudio = vAudio.as<JsonObject>();                
                audio->obslozPolecenieDane(&daneAudio);
            };
            //polecenie dla modulu 'ledy'
            JsonVariant vLedy = thisData["ledy"];
            if (!vLedy.isNull()) {
                PD_INFO("[ROBOT]-> obiekt ledy.");
                JsonObject daneLedy = vLedy.as<JsonObject>();
                ledy->obslozPolecenieDane(&daneLedy);
            };
            //polecenie dla modulu 'radar'
            JsonVariant vRadar = thisData["radar"];
            if (!vRadar.isNull()) {
                PD_INFO("[ROBOT]-> obiekt radar.");
                JsonObject daneRadar = vRadar.as<JsonObject>();
                radar->obslozPolecenieDane(&daneRadar);
            };

        };
        newData = false;
    }

    naped->zatrzymaj();  
};
