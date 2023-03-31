#ifndef _YTESRADAR_h
#define _YTESRADAR_h

#include "setup.h"
#include "arduino.h"
#include "YtesServo.h"

class YtesRadar {
private:
	//Servo serwo;

	int pinTrigger;
	int pinEcho;
	int zasiegMin = -1;
	int zasiegMax = -1;
	float _ostatniPomiar = 10.0f; //minimalny  zakres to : 2..400  cm
	long msCzasPomiaru = 200;
	unsigned long msNastepnyPomiar = millis()+msCzasPomiaru;
	unsigned int echoMicrosekundy();
public:
	YtesRadar(int trigger, int echo, int min, int max);
	~YtesRadar();
	//serwo
	void ustaw90();
	void testObrot();
	//czujnik 
	int dystansMm();
	float dystansCm();
	float mierzDystans();
};
//-----------------KONSTRUKTOR , DESTRUKTOR, PRYWATNE ----------------------------
/**
* @brief Zainicjuj serwo
* 
*/
YtesRadar::YtesRadar(int trigger, int echo, int min, int max) {
	pinMode(trigger, OUTPUT);
	pinMode(echo, INPUT);
	pinTrigger = trigger;
	pinEcho = echo;
	zasiegMin = min;
	zasiegMax = max;
	//serwo.attach(SERWO_RADAR_PIN, Servo::CHANNEL_NOT_ATTACHED, 0, 180, 500, 2400);
	RADAR_INFO("[radar] serwo zalaczone");	
};
/**
* @brief Zwolnij serwo
*/
YtesRadar::~YtesRadar() {
	//serwo.detach();
	RADAR_INFO("[radar] serwo odlaczone.");
};

//---------------------------------PUBLICZNE -------------------------------------
/**
* @brief Ustaw radar w pocz¹tkowej pozycji.
*/
void YtesRadar::ustaw90() {	
	//serwo.write(90);
	RADAR_INFO("[radar] pozycja pocz¹tkowa ustawiona.");
};

unsigned int YtesRadar::echoMicrosekundy() {
	digitalWrite(pinTrigger, LOW);
	delayMicroseconds(5);
	digitalWrite(pinTrigger, HIGH);
	delayMicroseconds(10);
	digitalWrite(pinTrigger, LOW);
	while (digitalRead(pinEcho) == LOW);

	int pulseStart = micros();
	while (digitalRead(pinEcho) == HIGH);
	return micros() - pulseStart;
}

int YtesRadar::dystansMm() {
	int czas = echoMicrosekundy();
	// Z internetu :  332m/s = 3320cm/s = 0.0332cm/us).
	int dystans = (czas / 2) * 0.332;
	if (zasiegMin == -1 && zasiegMax == -1) return dystans;
	if (dystans > zasiegMin && dystans < zasiegMax) return dystans;
	return 0;
};

float YtesRadar::dystansCm() {
	return (float)dystansMm() / 10;
};

//---------------------------------DEBUG------------------------------------------
/**
* Wykonaj pe³ny ruch w zakresie 0-180 st. , nastepnie 180-0 st
*/
void YtesRadar::testObrot() {
	RADAR_INFO("[radar] test obrotu 0-180 i powrot do 0.");
	for (int i = 0; i <= 180; i++) {
		//		serwo.write(i);
		delay(20);
	};
	for (int i = 180; i >= 0; i++) {
		//		serwo.write(i);
	};
};

/**
* @brief Pobierz odleg³oœæ do przeszkody domyslnie w centymetrach
*/
float YtesRadar::mierzDystans() {
	if (millis() > msNastepnyPomiar) {
		msNastepnyPomiar = millis() + msCzasPomiaru;
		return dystansCm();
	};
	return _ostatniPomiar;
};
#endif