#include "setup.h"
#include "YtesZyroskop.h"
/**
* Wazna jest ustawienie fizyczne ukladu na plytce.
* Uk³ad ma byæ ustawiony pinami w kierunku przodu pojazdu.
*	oooooooo
*   |      |
*    ------
* Wtedy osie wygl¹daj¹ nastêpuj¹co :
* Y - to przod /tyl
* 
*/


#include <cmath>


YtesZyroskop* zyroskop;
unsigned long msWyswietlOdczyty;

void setup() {
	randomSeed(analogRead(A0)); //pamietac do generatora!!
	Serial.begin(115200);
	while (!Serial) delay(50);
	Wire.begin(); //Pamietac!! do glownego
	Serial.println("Test klasy YtesZyroskop : 1.01");
	Serial.println("Dodanio stan :ruch");
	zyroskop = new YtesZyroskop(&mpu, 100); // 0 - kazdy przebieg petli , wiêksza wartoœæ - co okreœlony czas w ms.
	delay(1000);
	msWyswietlOdczyty = millis();
	
};

void loop() {
	zyroskop->uaktualnijOdczyty();

	zyroskop->wypiszStan();
	//zyroskop->wypiszRuchWstrzas();
};
