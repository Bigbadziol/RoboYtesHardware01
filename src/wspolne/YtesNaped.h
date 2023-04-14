// YtesNaped.h
// Zgoednie z dokumentacja , dla serwa AR3606HB , zakres impulsow to 800-2200 , pozycja neutralna to 15000
#ifndef _YTESNAPED_h
#define _YTESNAPED_h
#include "arduino.h"
#include "YtesServo.h"
#include "YtesRadar.h"

enum KOLO{ LEWE,PRAWE };

class YtesNaped {
	private:
		const int LEWE_PRZOD_US = 2000; //org. 2200
		const int LEWE_TYL_US = 1000;	//org. 800
		const int LEWE_STOP_US = 1490;	

		const int PRAWE_PRZOD_US = 1000;  //tu dobrze : 800
		const int PRAWE_TYL_US = 2000;   //tu dobrze : 2200
		const int PRAWE_STOP_US = 1490;  //tu dobrze : 1500

		const unsigned long msImpuls = 300; //czas wykonywania polecenia ruchu
		unsigned long msTeraz = millis(); //aktualny czas w ms.
		unsigned long msStop = msTeraz + msImpuls;

		Servo serwoLewe;
		Servo serwoPrawe;
		//void zaloaczLewe();
		//void zalaczPrawe();
		boolean poruszamSie = false;
		void uaktualnijStoper(); // polecenie modyfikuje stan poruszamSie
		unsigned long msOstatniRuch = 0L; //czas ostatniego wykonania ruchu przez pojazd, domyslnie wartosc przekazywana dalej
										//do audio
		//...
		boolean _autostop = false; //czy mam uniemozliwic blokade przed udezeniem w przeszkode(w oparciu o radar).
		float _odlegloscStop = 15.0f; // odleg³oœæ wyrazona w cm. Zbli¿enie siê do przeszkody na odleg³oœæ mniejsz¹
									  // zablokuje mo¿liwoœæ wykonania ruchów :prawo-przod,przod,lewo-przod
		YtesRadar* radar = nullptr;
		boolean blokujRuchDoPrzodu();
public:
	YtesNaped();
	~YtesNaped();
	void dodajRadar(YtesRadar* pRadar);

	// Polecenia dla kol na zasadzie : raz nadany kierunek kontynuowany jest a¿ do nadejœcia kolejnego polecenia.
	// polecenia nie modyfikuj¹ zmiennej poruszamSie oraz zmiennej msOstatniRuch.
	void ruchLewePrzod();
	void ruchLeweTyl();
	void ruchLeweStop();

	void ruchPrawePrzod();
	void ruchPraweTyl();
	void ruchPraweStop();

	//Polecenia dla pojazdu na zasadzie : wykonuj ruch w danym kierunku przez okreslony(msImpuls) okres czasu.
	//Kolejne polecenie przychodz¹ce w czasie teraz-stop , odœwie¿a czas stop o msImpuls
	//Polecenia poœrednio (bo u¿ywaj¹ uaktualnijStoper)  zmienn¹¹ poruszamSie
	void ruchPrzod();
	void ruchTyl();
	void ruchStop();

	void ruchPrawoPrzod();
	void ruchLewoPrzod();

	void ruchPrawoTyl();
	void ruchLewoTyl();

	void ruchUs(KOLO kolo, int czasUs);

	void ruchParametry(int lewe, int prawe); //parametry w posaci -1,0,1 do sterowania kolem otrzymane z metody obslozPolecenieDane

	void zatrzymaj(); //bazowy stan naszego robota, Brak polecen kierunku przez okreslony czas automatycznie zatrzymuje robota
	// kazde polecenie() ruchu odswieza czas zatrzymania o wartosc impuls. Metoda musi byc wywolywana , niezaleznie , cyklicznie
	// w glowej petli programu, modyfikuje zmienna poruszamSie

	void wlaczObslugeRadaru();
	void wylaczObslugeRadaru();

	void obslozPolecenieDane(JsonObject* dane); //obsluga samego obiektu.
	String odpowiedz();
	unsigned long ostatniRuch() { return msOstatniRuch; }; //czas faktycznie wykonanego ruchu przez pojazd
};

/**
* @brief ustaw poczatkowe parametry serw kol, pamietac ¿e na pa³ê ko¿ystamy z biblioteki ktora obs³uguje serwa 180
* nie 360.
*/
YtesNaped::YtesNaped() {
	//serwoLewe.attach(LEWE_KOLO_PIN,Servo::CHANNEL_NOT_ATTACHED,0,360);
	//serwoPrawe.attach(PRAWE_KOLO_PIN, Servo::CHANNEL_NOT_ATTACHED, 0, 360);
	serwoLewe.attach(LEWE_KOLO_PIN, Servo::CHANNEL_NOT_ATTACHED, 0, 360, 800, 2200);
	serwoPrawe.attach(PRAWE_KOLO_PIN, Servo::CHANNEL_NOT_ATTACHED, 0, 360, 800, 2200);
	NAPED_INFO("[Naped] -> serwa ustawione.");
};

/**
* @brief Odlacz impuls sterujacy serwami obu kol.
*/
YtesNaped::~YtesNaped() {
	serwoLewe.detach();
	serwoPrawe.detach();
	NAPED_INFO("[Naped] -> serwa odlaczone.");
};

/**
* @brief Dodaj radar
*/
void YtesNaped::dodajRadar(YtesRadar* pRadar) {
	radar = pRadar;
};

/**
* @brief Ustaw czas zatrzymania pojazdu.
*/
void YtesNaped::uaktualnijStoper() {
	poruszamSie = true;
	msStop = millis() + msImpuls;
}

/**
* @brief Funkcja pomocnicza która sprawdza czy : radar jest podpiêty, czy autostop w³¹czony , czy przekroczona bezpieczna odleg³oœæ
* True - jeœli ruch ma byæ zablokowany
*/
boolean YtesNaped::blokujRuchDoPrzodu() {
	if (_autostop == false) {
		return false;
	}else {
		if (radar != nullptr) {
			float odleglosc = radar->dystans();
			//NAPED_INFO_V("(blokada ruchu) Odleglosc:", odleglosc);
			if (_autostop == true && odleglosc > 2 && odleglosc < _odlegloscStop) {
				NAPED_INFO("(naped) : blokada ruchu");
				return true;
			};
		};
	};
	return false;
};

//------------------------------- POLECENIA DLA KOL ---------------------------------
/**
* @brief Ruch lewego kola do przodu. Po wydaniu  polecenia ruch bêdzie kontynuowany az do czasu otrzymania
* innego polecenia dla lewego ko³a.
*/
void YtesNaped::ruchLewePrzod() {
	serwoLewe.writeMicroseconds(LEWE_PRZOD_US);
};

/**
* @brief Ruch lewego kola do tylu. Po wydaniu  polecenia ruch bêdzie kontynuowany az do czasu otrzymania
* innego polecenia dla lewego ko³a.
*/
void YtesNaped::ruchLeweTyl() {
	serwoLewe.writeMicroseconds(LEWE_TYL_US);
};

/**
* @brief Zatrzymanie lewego kola. Po wydaniu  polecenia ruch bêdzie kontynuowany az do czasu otrzymania
* innego polecenia dla lewego ko³a.
*/
void YtesNaped::ruchLeweStop() {
	serwoLewe.writeMicroseconds(LEWE_STOP_US);
};

/**
* @brief Ruch prawego kola do przodu. Po wydaniu  polecenia ruch bêdzie kontynuowany az do czasu otrzymania
* innego polecenia dla prawego ko³a.
*/
void YtesNaped::ruchPrawePrzod() {
	serwoPrawe.writeMicroseconds(PRAWE_PRZOD_US);
};

/**
* @brief Ruch prawego kola do tylu. Po wydaniu  polecenia ruch bêdzie kontynuowany az do czasu otrzymania
* innego polecenia dla prawego ko³a.
*/
void YtesNaped::ruchPraweTyl() {
	serwoPrawe.writeMicroseconds(PRAWE_TYL_US);
};

/**
* @brief Zatrzymanie prawego kola. Po wydaniu  polecenia ruch bêdzie kontynuowany az do czasu otrzymania
* innego polecenia dla prawego ko³a.
*/
void YtesNaped::ruchPraweStop() {
	serwoPrawe.writeMicroseconds(PRAWE_STOP_US);
};

//------------------------------- POLECENIA DLA POJAZDU ---------------------------------
/**
* @brief  Logiczny ruch pojazdu do przodu. Serwo prawe krêci siê w przeciwnym ruchu do serwa lewego.
* Testuj warunek blokowania ruchu, kiedy przeszkoda znajduje siê zbyt blisko.
*/
void YtesNaped::ruchPrzod() {
	if (!blokujRuchDoPrzodu()) {
		msOstatniRuch = millis();
		uaktualnijStoper();
		ruchLewePrzod();
		ruchPrawePrzod();
	};
};

/**
*  @brief Logiczny ruch pojazdu do ty³u. Serwo prawe krêci siê w przeciwnym ruchu do serwa lewego.
*/
void YtesNaped::ruchTyl() {
	msOstatniRuch = millis();
	uaktualnijStoper();
	ruchLeweTyl();
	ruchPraweTyl();
};

/**
*  @brief Zatrzymaj kola.
*/
void YtesNaped::ruchStop() {
	poruszamSie = false;
	ruchLeweStop();
	ruchPraweStop();
};

/**
* @biref Logiczny skrêt przodem w prawo. 
* Prawe ko³o stoi , lewe krêci siê do przodu.
* Testuj warunek blokowania ruchu, kiedy przeszkoda znajduje siê zbyt blisko.
*/
void YtesNaped::ruchPrawoPrzod() {
	if (!blokujRuchDoPrzodu()) {
		msOstatniRuch = millis();
		uaktualnijStoper();
		ruchPraweStop();
		ruchLewePrzod();
	};
};

/**
* @biref Logiczny skrêt przodem w lewo.
* Lewe ko³o stoi , prawe krêci siê do przodu.
* Testuj warunek blokowania ruchu, kiedy przeszkoda znajduje siê zbyt blisko.
*/
void YtesNaped::ruchLewoPrzod() {
	if (!blokujRuchDoPrzodu()) {
		msOstatniRuch = millis();
		uaktualnijStoper();
		ruchLeweStop();
		ruchPrawePrzod();
	};
};

/**
* @biref Logiczny skrêt ty³em w prawo.
* Prawe ko³o stoi , lewe krêci siê do ty³u
*/
void YtesNaped::ruchPrawoTyl() {
	msOstatniRuch = millis();
	uaktualnijStoper();
	ruchPraweStop();
	ruchLeweTyl();
};

/**
* @biref Logiczny skrêt ty³em w lewo.
* Prawe lewe stoi , lewe krêci siê do ty³u
*/
void YtesNaped::ruchLewoTyl() {
	msOstatniRuch = millis();
	uaktualnijStoper();
	ruchLeweStop();
	ruchPraweTyl();
};

/**
* @brief.Bezposredni zapis sygnalu do wskazanego kola. kolo = LEWE, PRAWE. czasUs 800..2200
* Wartosc bliska 1500 powinna zatrzymac kolo
*/
void YtesNaped::ruchUs(KOLO kolo, int czasUs) {
	msTeraz = millis();
	msOstatniRuch = millis();
	switch (kolo) {
	case LEWE:
		serwoLewe.writeMicroseconds(czasUs);
		break;
	case PRAWE:
		serwoPrawe.writeMicroseconds(czasUs);
		break;
	default:
		break;
	};
};

/**
* @brief Nadaj ruch pojazdowi steruj¹c jego lewym i prawym ko³em. 
* Akceptowalne wartoœci parametrów :
* 1 - ruch ko³a do przodu
* 0 - zatrzymaj kolo
* -1 - ruch ko³a do ty³u
* Ruch wykonywany kest przez okres (msImpuls) czasu
*/
void  YtesNaped::ruchParametry(int lewe, int prawe) {
	if (lewe < -1 || lewe > 1) {
		NAPED_ERROR("[Ruch parametry] , parametr lewego kola poza zakresem. ");
		return;
	}
	if (prawe < -1 || prawe> 1) {
		NAPED_ERROR("[Ruch parametry] , parametr prawego kola poza zakresem.");
		return;
	}

	if (lewe == 1 && prawe == 1) ruchPrzod();
	else if (lewe == 0 && prawe == 0) ruchStop(); //realnie stan nie na pulpicie filipa
	else if (lewe == -1 && prawe == -1) ruchTyl();

	else if (lewe == 1 && prawe == 0) ruchPrawoPrzod();
	else if (lewe == 0 && prawe == 1) ruchLewoPrzod();
	else if (lewe == -1 && prawe == 0) ruchLewoTyl();
	else if (lewe == 0 && prawe == -1) ruchPrawoTyl();
	else {
		NAPED_ERROR("Nie poprawny zestaw parametrow.");
	};
};

/**
* @brief Metoda do  zatrzymywania ruchu  pojazdu
*/
void YtesNaped::zatrzymaj() {
	if ((poruszamSie == true) && (msStop < millis())) {
		ruchStop();
		poruszamSie = false;
		NAPED_INFO("Zatrzymano pojazd.");
	};
};
/**
* @brief Uwzglednij wskazania radaru przed wykonaniem ruchu w kierunkach : prawo-przod,przod, lewo-przod
* W³¹czenie opcji blokuje mo¿liwoœæ ude¿enia w przeszkodê.
*/
void YtesNaped::wlaczObslugeRadaru() {
	_autostop = true;
};

/**
* @brief Wy³¹cz radar, mo¿na waln¹æ w przeszkodê.
*/
void YtesNaped::wylaczObslugeRadaru() {
	_autostop = false;
};


/**
* @brief Na podstawie danych z pakietu ustal kierunek ruchu
* {"ruch":{"PT":1}} , tu trafi -> {"PT":1}
*/
void YtesNaped::obslozPolecenieDane(JsonObject* dane) {

	JsonVariant vLP = (*dane)["LP"];
	if (!vLP.isNull()) {
		NAPED_INFO("Polecenie ruch -> lewo-przod");
		ruchLewoPrzod();
	}
	JsonVariant vP = (*dane)["P"];
	if (!vP.isNull()) {
		NAPED_INFO("Polecenie ruch -> przod");
		ruchPrzod();
	}
	JsonVariant vPP = (*dane)["PP"];
	if (!vPP.isNull()) {
		NAPED_INFO("Polecenie ruch -> prawo-przod");
		ruchPrawoPrzod();
	}

	JsonVariant vLT = (*dane)["LT"];
	if (!vLT.isNull()) {
		NAPED_INFO("Polecenie ruch -> lewo-tyl");
		ruchLewoTyl();
	}
	JsonVariant vT = (*dane)["T"];
	if (!vT.isNull()) {
		NAPED_INFO("Polecenie ruch -> tyl");
		ruchTyl();
	}
	JsonVariant vPT = (*dane)["PT"];
	if (!vPT.isNull()) {
		NAPED_INFO("Polecenie ruch -> prawo-tyl");
		ruchPrawoTyl();
	};
	JsonVariant vAutostop = (*dane)["AUTOSTOP"];
	if (!vAutostop.isNull()) {
		int as = vAutostop.as<int>();
		NAPED_INFO_V("Ustawienie autostop :", as);
		if (as == 1) _autostop = true;
		else _autostop = false;
	};
};

/**
* @brief Przygoduj odpowiedz dotyczac¹ ustawieñ modu³u napêd.
* Obecnie zwracane tylko ustawienie 'autostop'
*/
String YtesNaped::odpowiedz() {
	StaticJsonDocument<256> doc;	
	JsonObject objData = doc.createNestedObject("naped");
	objData["AUTOSTOP"] = (byte)_autostop;
	String tmp = "";
	size_t resSize = serializeJson(doc, tmp);

	int firstIndex = tmp.indexOf('{');
	int lastIndex = tmp.lastIndexOf('}');
	String odp = tmp.substring(firstIndex + 1, lastIndex);
	return odp;
};


#endif

