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
	int zasiegMin = 20;		//w mm minimalna mierzalna odleg�o�� dla czujnika
	int zasiegMax = 4000;	//w mm maxymalna mierzalna odleg�o�� dla czujnika
	float _ostatniPomiar = 10.0f; //realny pomiar  zakres to : 2..400  cm
	long msCzasPomiaru = 200; //interwa� okresowego pomiaru odleg�o�ci
	unsigned long msNastepnyPomiar = millis() + msCzasPomiaru;
	unsigned int echoMicrosekundy();
	int dystansMm();
	float dystansCm();
public:
	YtesRadar(int triggerPin, int echoPin, int min, int max,int RadarPin);
	~YtesRadar();

	void ustawRadar(int kat); // przyjmowane 
	void mierzDystans();
	float dystans();
};
//-----------------KONSTRUKTOR , DESTRUKTOR, PRYWATNE ----------------------------
/**
* @brief Zainicjuj serwo
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

/**
* @brief Faktyczna funkcja pomiarowa. Wysy�a kr�tki impuls dzwi�kowy i czeka na powr�t echa.
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
};

/**
* @brief Na podstawie dotarcia impulsu powrotnego w oparciu o pr�dko�� d�wi�ku oszacuj w mm odleg�o�� do
* przeszkody. Warto�� 0 traktowana jest jako b��dna.
*/
int YtesRadar::dystansMm() {
	int czas = echoMicrosekundy();
	// Z internetu :  332m/s = 3320cm/s = 0.0332cm/us).
	int dystans = (czas / 2) * 0.332;
	if (dystans > zasiegMin && dystans < zasiegMax) return dystans;
	return 0;
};


//---------------------------------PUBLICZNE -------------------------------------
/**
* @brief Utaw radar do okre�lonej pozycji . Domuszczalny zakres to 0-180 st.
* Przekroczenie zakresu ustawi radar w pozycji 90 st. (na wprost);
*/
void YtesRadar::ustawRadar(int kat) {
	int _kat = kat;
	if (_kat < 0) kat = 90;
	if (_kat > 180) kat = 90;
	_kat = 180 - _kat;
	serwoRadar.write(_kat);
};



/*
* @brief Oszacuj odleg�o�� do przeszkody w cm. Warto�� tego pomiaru stanowi baz� dla dzia�ania pozosta�ych
* modu��w robota. Warto�� 0 oznacza b��d. Pami�ta� nale�y o faktycznych mo�liwo�ciach pomiarowych czujnika.
*/
float YtesRadar::dystansCm() {
	return (float)dystansMm() / 10;
};

/**
* @brief Pobierz odleg�o�� do przeszkody domyslnie w centymetrach
* Uwaga!. Warto�� 0 oznacza b��dny (niemo�liwy do dokoania pomiar).
* Metoda przeznaczona do cyklicznego pomiaru odleg�o�ci, ma by� wywo�ywana w g��wnej p�tli programu.
*/
void YtesRadar::mierzDystans() {
	if (millis() > msNastepnyPomiar) {
		msNastepnyPomiar = millis() + msCzasPomiaru;
		_ostatniPomiar = dystansCm();
	};
};

/**
* @brief Wlasciwa metoda do pobierania odleg�o�ci od obiektu. Odleg�o�� wyra�ona w cm.
*/
float YtesRadar::dystans() {
	return _ostatniPomiar;
};


#endif