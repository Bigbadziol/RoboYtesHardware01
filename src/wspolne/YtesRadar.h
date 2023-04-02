#ifndef _YTESRADAR_h
#define _YTESRADAR_h

#include "setup.h"
#include "arduino.h"
#include "YtesServo.h"

class YtesRadar {
private:
	Servo serwoRadar;

	int pinTrigger;
	int pinEcho;
	int zasiegMin = -1;
	int zasiegMax = -1;
	float _ostatniPomiar = 10.0f; //minimalny  zakres to : 2..400  cm
	long msCzasPomiaru = 200;
	unsigned long msNastepnyPomiar = millis() + msCzasPomiaru;
	unsigned int echoMicrosekundy();
public:
	YtesRadar(int triggerPin, int echoPin, int min, int max,int RadarPin);
	~YtesRadar();
	//serwo	
	void ustawRadar(int kat); // przyjmowane 
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
YtesRadar::YtesRadar(int triggerPin, int echoPin, int min, int max, int radarPin) {
	pinMode(triggerPin, OUTPUT);
	pinMode(echoPin, INPUT);
	pinTrigger = triggerPin;
	pinEcho = echoPin;
	zasiegMin = min;
	zasiegMax = max;
	serwoRadar.attach(radarPin, Servo::CHANNEL_NOT_ATTACHED, 0, 180, 500, 2400);	
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
* @brief Utaw radar do okreœlonej pozycji . Domuszczalny zakres to 0-180 st.
* Przekroczenie zakresu ustawi radar w pozycji 90 st. (na wprost);
*/
void YtesRadar::ustawRadar(int kat) {
	int _kat = kat;
	if (_kat < 0) kat = 90;
	if (_kat > 180) kat = 90;
	_kat = 180 - _kat;
	serwoRadar.write(_kat);
};

/**
* 
*/
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