//Test klasy przesylania danych z aplikacji : Bluetooth RC control
/*
* You will notice that the examples here do not use any of these Arduino functions
Serial.parseInt()
Serial.parseFloat()
Serial.readBytes()
Serial.readBytesUntil()
Serial.readString() - wirtualna dla bluetooth
-Ogarn¹æ temat na zasadzie : znak specjalny koniec strumienia danych , dwa znaki specjalne , pocz¹tek i koniec
 strumienia danych.

Wiedza na przyszlosc , jesli potrzeba niezw³ocznej obs³ugi danych z Bt nie uzywac powyzszych.
*/
#include "setup.h"
#include "BdlBluetoothSerial.h" // uwaga dla zgodnosci z ledzikiem
#include "YtesNaped.h"

const char* nazwaUrzadzenia = "YtesRobot01";
const char* pinUrzadzenia = "0987";

String fakePdpowiedz = "{\"audio\":{}}#$!#";

BdlBluetoothSerial bt; //zamiennie do orginalnej biblioteki
YtesNaped* naped;

const uint16_t numChars = 512;
char receivedChars[numChars];
boolean newData = false;



void setup() {
    Serial.begin(115200);
    while (!Serial) continue;
    delay(1000);

    Serial.println("Test aplikacji Bluetooth RC Controller 1.10");
    Serial.println("Tylko pojedyncze znaki interpretuje:");

    bt.setPin(pinUrzadzenia);
    bt.begin(nazwaUrzadzenia, false); //Bluetooth device name
    Serial.print("Urzadzenie : "); Serial.println(nazwaUrzadzenia);
    Serial.print("Pin :"); Serial.println(pinUrzadzenia);
    //naped = new YtesNaped();
}


void recvWithEndMarker() {
    static uint16_t ndx = 0;
    char endMarker = '\n';
    char rc;

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


//testowana prubka na sucho : {"audio":{"PN":3}}    (18 bajtow)
void loop() {
    recvWithEndMarker();
    //recvWithStartEndMarkers();
    showNewData();
}

/*
* // Wersja z 2 znakami (rozpoczynaj¹cym i koncz¹cym strumien danych)
* Przed setup :
* const uint16_t numChars = 512;
char receivedChars[numChars];
boolean newData = false;
* 
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static uint16_t ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (bt.available() > 0 && newData == false) {
        rc = bt.read();
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}
*/

