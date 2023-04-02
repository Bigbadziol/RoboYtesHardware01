/*
* // Uwaga, przyjrzec sie zachowaniu GPIO0 - przyszly radar  , ze wgledu na klod w YtesServo.h moze generowac blad
* //This is bold text to get your attention, I can make it red as well: It is "\r\n" not "\n\r". <--NA LEDZIKU SPRAWDZIC


/*
* Powazny problem z szybkoscia komunikacji pomiedzy app-hardware
* - blok obslugi przychodzacych danych bt, przeniesiony do funkcji- nie spowalnia   testYtesNaped->symulacjaPetliBluetooth()
* - spamowanie osobnym guzikiem bez klasy communicator nie spowalnia (po stronie apki) wszystko ok
* - dodanie delay, do petli glownej nie zmienia
*/

/*
* Jak z dzwiêkiem, czy oba kana³y maj¹ graæ równoczeœnie czy w przypadku np. uderzenia , lewy(domyslnie muzyka) ma 
* zostaæ zapa³zowany ?
* 
*/
// ESP32 - mini 01) AC:67:B2:2D:16:92
//  C8:F0:9E:F4:C6:3E , a poelutownym MPU6050


#include "setup.h"
#include "YtesNaped.h"
#include "YtesRadar.h"
#include "YtesZyroskop.h"


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
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif


YtesRadar* radar;
YtesNaped* naped;
YtesZyroskop* zyroskop;

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
CRGB leds[NUM_LEDS];   // leds array

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


void recvWithEndMarker() {


    while (bt.available() > 0 && newData == false) {
        rc = bt.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}


void showNewData() {
    if (newData == true) {
        Serial.print("Dane :  ");
        Serial.println(receivedChars);
        newData = false;
    }
}


void radarRuch180() {
    for (int i = 0; i <= 180l; i++) {
        radar->ustawRadar(i);
        delay(25);
    };
};

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void setup() {
    randomSeed(analogRead(A0)); //pamietac do generatora!!
    Wire.begin(); //Pamietac!! do glownego

    Serial.begin(115200);
    while (!Serial) continue;
    delay(1000);

    Serial.println("-----------------------------------------------------");
    Serial.println("RoboYtesHardware 1.06");
    Serial.println("Zmiana sposobu pobierania danych , przez buforowanie.");
    Serial.println("Ledy");


    bt.onAuthComplete(AuthCompleteCallback);
    bt.setPin(pinUrzadzenia);
    bt.begin(nazwaUrzadzenia, false); //Bluetooth device name
    Serial.print("Urzadzenie : "); Serial.println(nazwaUrzadzenia);
    Serial.print("Pin :"); Serial.println(pinUrzadzenia);
    Serial.print("Mac : "); Serial.println(mojMac().c_str());
    Serial.println("Robot gotowy do parowania");

    LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    leds[0] = CRGB::Red;
    leds[1] = CRGB::Green;
    leds[2] = CRGB::Blue;
    #define COLOR_START CRGB::Red  // kolor pocz¹tkowy
    #define COLOR_END CRGB::Black // kolor koñcowy
    #define START_LED 4 
    fill_gradient_RGB(leds + START_LED, NUM_LEDS - START_LED, COLOR_START, COLOR_END);
    FastLED.show();

    radar = new YtesRadar(HCSR_TRIG_PIN, HCSR_ECHO_PIN, 20, 4000, SERWO_RADAR_PIN);
    naped = new YtesNaped();
    zyroskop = new YtesZyroskop(&mpu, 100); // 0 - kazdy przebieg petli , wiêksza wartoœæ - co okreœlony czas w ms.

    
    //radarRuch180();
    //radar->ustawRadar(90);
    //delay(2000);
    // 
    // 
    //naped->ruchPrawoPrzod();


};

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void loop() {
    //---------------------Odczyt radar -  zasieg---------------------------------------
    if (millis() > msRadarOdczytTest) {
        msRadarOdczytTest = millis() + 1000;
//        Serial.printf("Testowy odczyt odleglosci : %.2f \n", radar->dystansCm());
    };
    //--------------------Zyroskop odczyt pozycji---------------------------------------
    zyroskop->uaktualnijOdczyty();
    if (millis() > msZyroskopTest) {
        msZyroskopTest = millis() + 500;
        zyroskop->wypiszKluczowePomiary();
    };

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
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
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
            JsonVariant cmd = thisData["cmd"];
            if (!cmd.isNull()) {
                String cmdDane = cmd.as<String>();
                PD_INFO_V("Przychadzaca komenda: ", cmdDane);
                if (cmdDane.equalsIgnoreCase("DANE_PROSZE")) {
                    PD_INFO("[ROBOT] -> prosba o dane.");
                    bt.println("{\"audio\":{}}#$!#");
                }
                else {
                    PD_INFO("[ROBOT] -> nieznana komenda");
                }
            }
            JsonVariant ruch = thisData["ruch"];
            if (!ruch.isNull()) {
                //PD_INFO("[ROBOT] -> wykonaj ruch");
                JsonObject ruchDane = ruch.as<JsonObject>();
                naped->obslozPolecenieDane(&ruchDane);
            };
            JsonVariant audio = thisData["audio"];
            if (!audio.isNull()) {
                JsonObject audioDane = audio.as<JsonObject>();
                PD_INFO("[ROBOT]-> obiekt audio");
            };
        };



        newData = false;
    }

    naped->zatrzymaj();
  
};
