#include "setup.h"
#include "YtesRadar.h"

YtesRadar* radar;
void setup() {
  
    Serial.begin(115200);
    while (!Serial) continue;  
    delay(100);
    Serial.println("test YtesRadar 1.00");
    RADAR_INFO("Zabawa z serwem i HC-SR04");
    radar = new YtesRadar(HCSR_TRIG_PIN, HCSR_ECHO_PIN, 20, 4000);
    //radar->ustaw90(); //nie dziala
    //radar->testObrot(); //nie dziala
}

void loop() {
    Serial.println(radar->mierzDystans());
    delay(500);
}
