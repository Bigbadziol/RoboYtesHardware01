#ifndef _YTESRADAR_h
#define _YTESRADAR_h

#include "setup.h"
#include "arduino.h"
#include "YtesServo.h"

class YtesRadar {
private:
	unsigned long _msKrok = 50;
	unsigned long _msNastepnyKrok;
	int _ruch180zapamietanyKat;		//
	int _ruch180aktualnyKat;		//	
	int _ruch180kierunek;			// -1 lub 1  ,-1 - zacznij d��y� do 0
	int _ruch180wykonuj = false;	//

	int aktualnyKat;		 // aktualnie ustawiony kat radaru;
	Servo serwoRadar;
	int pinTrigger;
	int pinEcho;
	int zasiegMin = 20;				// w mm minimalna mierzalna odleg�o�� dla czujnika
	int zasiegMax = 2000;			// w mm maxymalna mierzalna odleg�o�� dla czujnika
	float _ostatniPomiar = 10.0f;   // realny pomiar  zakres to : 2..180 cm
	long msCzasPomiaru = 200;		// interwa� okresowego pomiaru odleg�o�ci
	unsigned long msNastepnyPomiar = millis() + msCzasPomiaru;
	unsigned long msRadarAkcja = 0L;// zmienna pomocnicza przekazywana klasie audio, ruch radaru przerywa nud�
	unsigned int echoMicrosekundy();
	int dystansMm();
	float dystansCm();
public:
	YtesRadar(int triggerPin, int echoPin, int min, int max,int RadarPin);
	~YtesRadar();

	void ustawRadar(int kat); // przyjmowane 
	int pobierzKat();
	void mierzDystans();
	float dystans();
	void obslozPolecenieDane(JsonObject* dane);
	String odpowiedz();
	void ruch180Inicjuj(int katKoncowy);
	void ruch180Krok();
	unsigned long ostatniRuch() { return msRadarAkcja; };

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
	aktualnyKat = 90;
	_ruch180zapamietanyKat = aktualnyKat;
	ustawRadar(aktualnyKat);
	RADAR_INFO("[radar] ustawiony na 90 st.");
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
	aktualnyKat = _kat;
	serwoRadar.write(_kat);
};

/**
* @brief Pobierz aktualny k�t ustawienia radaru
*/
int YtesRadar::pobierzKat() {
	return aktualnyKat;
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

/**
* "STARTKAT" - 0..180 , bazowy k�t od kt�rego przeprowadzi� demonstracj�
* "WYKONAJ" - 0..1 , 1 - rozpocznij sekwencj�
* "KAT" - 0..180 - ustaw fizycznie radar
*/
void YtesRadar::obslozPolecenieDane(JsonObject* dane) {
	//demonstracna niezaleznego ruchu.
	JsonVariant vRuch180 = (*dane)["RUCH180"];
	if (!vRuch180.isNull()) {
		JsonVariant vStart = (*dane)["RUCH180"]["STARTKAT"];
		JsonVariant vWykonaj = (*dane)["RUCH180"]["WYKONAJ"];
		if (!vStart.isNull() && !vWykonaj.isNull()) {
			int start = vStart.as<int>();
			int wykonaj = vWykonaj.as<int>();
			if (wykonaj == 1) ruch180Inicjuj(start);
		};
	};
	//fizyczne ustawienie radaru
	JsonVariant vKat = (*dane)["KAT"];
	if (!vKat.isNull()) {
		int nowyKat = vKat.as<int>();
		ustawRadar(nowyKat);
	};
};

/*
* Prze�lij aktualne ustawienia radaru
*/
String YtesRadar::odpowiedz() {
	StaticJsonDocument<256> doc;
	JsonObject objData = doc.createNestedObject("radar");
	objData["KAT"] = (byte)aktualnyKat;
	JsonObject objRuch180 = objData.createNestedObject("RUCH180");
	objRuch180["STARTKAT"] = _ruch180zapamietanyKat;
	objRuch180["WYKONAJ"] = (byte)_ruch180wykonuj;
	String tmp = "";
	size_t resSize = serializeJson(doc, tmp);

	int firstIndex = tmp.indexOf('{');
	int lastIndex = tmp.lastIndexOf('}');
	String odp = tmp.substring(firstIndex + 1, lastIndex);
	return odp;
};

/**
* @brief Inicjalizacja dla demonstracji , pelny ruch od 0 do 180 i powrot do kata bazowego
*/
void YtesRadar::ruch180Inicjuj(int katKoncowy) {
	_ruch180zapamietanyKat = katKoncowy;
	_ruch180aktualnyKat = katKoncowy;
	_ruch180wykonuj = true;
	_ruch180kierunek = -1;
	_msNastepnyKrok = millis();
	msRadarAkcja = _msNastepnyKrok; //przerwij nud�
	ustawRadar(katKoncowy);
};

/**
* @brief wykonaj kolejny krok dla demonstracji
*/
void YtesRadar::ruch180Krok() {
	if (_ruch180wykonuj == true) {
		if (millis() > _msNastepnyKrok) {
			_msNastepnyKrok = millis() + _msKrok;
			msRadarAkcja = _msNastepnyKrok; //od�wie�aj, w domy�le odwlekaj nud� do puki trwa proces.
			if (_ruch180aktualnyKat == 0) _ruch180kierunek = 1;
			if (_ruch180aktualnyKat == 180) {
				ustawRadar(_ruch180zapamietanyKat);
				_ruch180wykonuj = false;
				return;
			}
			_ruch180aktualnyKat = _ruch180aktualnyKat + _ruch180kierunek;
			ustawRadar(_ruch180aktualnyKat);
		};
	};
};

#endif