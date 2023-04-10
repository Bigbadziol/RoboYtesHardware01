#include "setup.h"
#include "YtesRadar.h"

YtesRadar* radar;
long msWypiszPomiar = millis();

void setup() {
    Serial.begin(115200);
    while (!Serial) continue;
    delay(100);
    Serial.println("--------------------------------");
    Serial.println("test YtesRadar 1.02");
    Serial.println("Zabawa z serwem i HC-SR04");
    
    radar = new YtesRadar(HCSR_TRIG_PIN, HCSR_ECHO_PIN, 20, 2000, SERWO_RADAR_PIN); //org : 4000-> 2000
    radar->ustawRadar(90);
};

void loop() {
    radar->mierzDystans();
    if (millis() > msWypiszPomiar) {
        msWypiszPomiar = millis() + 500;
        Serial.printf("Odleglosc : %.2f \n",radar->dystans());

    }

}
