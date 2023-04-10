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
	int zasiegMin = 20;		//w mm minimalna mierzalna odleg³oœæ dla czujnika
	int zasiegMax = 4000;	//w mm maxymalna mierzalna odleg³oœæ dla czujnika
	float _ostatniPomiar = 10.0f; //realny pomiar  zakres to : 2..400  cm
	long msCzasPomiaru = 200; //interwa³ okresowego pomiaru odleg³oœci
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
* @brief Faktyczna funkcja pomiarowa. Wysy³a krótki impuls dzwiêkowy i czeka na powrót echa.
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
* @brief Na podstawie dotarcia impulsu powrotnego w oparciu o prêdkoœæ dŸwiêku oszacuj w mm odleg³oœæ do
* przeszkody. Wartoœæ 0 traktowana jest jako b³êdna.
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



/*
* @brief Oszacuj odleg³oœæ do przeszkody w cm. Wartoœæ tego pomiaru stanowi bazê dla dzia³ania pozosta³ych
* modu³ów robota. Wartoœæ 0 oznacza b³¹d. Pamiêtaæ nale¿y o faktycznych mo¿liwoœciach pomiarowych czujnika.
*/
float YtesRadar::dystansCm() {
	return (float)dystansMm() / 10;
};

/**
* @brief Pobierz odleg³oœæ do przeszkody domyslnie w centymetrach
* Uwaga!. Wartoœæ 0 oznacza b³êdny (niemo¿liwy do dokoania pomiar).
* Metoda przeznaczona do cyklicznego pomiaru odleg³oœci, ma byæ wywo³ywana w g³ównej pêtli programu.
*/
void YtesRadar::mierzDystans() {
	if (millis() > msNastepnyPomiar) {
		msNastepnyPomiar = millis() + msCzasPomiaru;
		_ostatniPomiar = dystansCm();
	};
};

/**
* @brief Wlasciwa metoda do pobierania odleg³oœci od obiektu. Odleg³oœæ wyra¿ona w cm.
*/
float YtesRadar::dystans() {
	return _ostatniPomiar;
};


#endif