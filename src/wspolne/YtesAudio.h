// YtesAudio.h
//TODO : przetestowac rzutowanie int->bool

#ifndef _YTESAUDIO_h
#define _YTESAUDIO_h

#include "setup.h"
#include "DFPlayerMini_Fast.h" // DFPlayerMini_Fast by PowerBroker2  Version 1.2.4
							   // https://github.com/PowerBroker2/DFPlayerMini_Fast
#include "YtesZyroskop.h"
#include "YtesRadar.h"

using namespace dfplayer;

enum TOR {CENZURA, BEZ_CENZURY};
enum KANAL {LEWY,PRAWY};
enum TYP_AUDIO {MUZYKA,EFEKT};
enum TRYB_AUDIO {NORMALNY,STEREO_MUZYKA,STEREO_EFEKTY,WYCISZANIE,PALZOWANIE};
enum GRUPA_DZWIEKOWA{
	LOSOWE_GADANIE,			//np. brak wskazan zyroskopu :nudze sie, ale lipa, 'ziewanie' , 'kichanie' , 'pierdzenie' ,'gwizdanie','czy sie stoi czy sie'
	PRZECHYL_BOK_MALY,		// to niebezpieczne, uwazaj troche, wiesz po ile ac ?
	PRZECHYL_BOK_DUZY,		// no i leze, dlugo bede lezal?, podnies mnie, boli mnie kolko, auaaaaa
	PRZECHYL_PRZOD_MALY,	//sam sie wspinaj, naprawde musze, nie mam sil
	PRZECHYL_PRZOD_DUZY,	//trzeba miec skrzydla nie kolka, co ja spiderman ? chyba cie pogielo
	PRZECHYL_TYL_MALY,		// zawsze lzej, chce na sanki, �iiiii!
	PRZECHYL_TYL_DUZY,		//naprawde sie boje, zeby sobie wybije,
	WSTRZAS_POJAZDU,		//ojojo, brr , grrr, ale wieje,
	RADAR_BLISKO,			//blizej nie podjade , mam leb rozbic, a co to?
	PRZYCISKI				//dowolno�� 8 dzwi�k�w podpi�tych pod przyciski
};

typedef struct efektDzwiekowy {
	GRUPA_DZWIEKOWA grupa;
	byte index;
}efektDzwiekowy_t;


//player prawy traktowany jest jako nadrzedny, w trybie normalnym gra muzyke
// lewy - efekty efekty
//normalny - osobne sterowanie playerem lewym , i playerem prawym
//stereo_muzyka - player prawy odgrywa z tego samego katalogu co nadrzedny(lewy) tylko muzyka, efekty sa ignorowane
//stereo_efekty - player prawy odgrywa z tego samego katalogu co nadrzedny(lewy) tylko efekty, muzyka jest ignorowana
//wyciszanie - podczas odgrywania efektu muzyka jest sciszana o okreslony poziom
//palzowanie - podczas odgrywania efektu muzyka jest pauzowana


//...Z wzgl�du na du�e opo�nienia w wykonywaniu polece�, dodatkowo r�ne czasy w zale�no�ci od scalaka
//pomys�em jest napisanie listy, kt�ra b�dzie przechowywa�a abstrakt polecenia
//audioHandler() bedzie sprawdza� rozmiar listy , je�li co� jest to co okre�lony okres czasu wykona polecenie i
//usunie je z listy.
class YtesAudioPolecenie {
public :
	String polecenieId = "";
	int wartoscParametru = 0;
	YtesAudioPolecenie(String id, int wartosc) {
		polecenieId = id;
		wartoscParametru = wartosc;
	};
};

class YtesAudio {
private:
	int polecenieOpoznienie = 350; //rozny czas odpowiedzi konkretnych scalak�w(7 rodz.) dla modu��w mp3. Od 100ms do nawet 500ms.
#if UZYJ_HARDWARE == 1
	HardwareSerial* portLewy;
	HardwareSerial* portPrawy;
#else
	SoftwareSerial* portLewy;
	SoftwareSerial* portPrawy;
#endif

	TOR defTor = CENZURA; //0..1 , 0-tor cenzura , 1-tor bez cenzury
						  //Tor definiuje z kt�rych fizycznie katalog�w pobierane s� nagrania muzyczne i d�wi�ki
						  //Katalogi : (forma zapisu nazw katalogow ma kluczowa - liczba w zapisie 2-cyfrowym)
						  // 01 - cenzura muzyka
						  // 02 - cenzura efekty d�wi�kowe
						  // 03 - bez cenzury muzyka
						  // 04 - bez cenzury d�wi�ki
						  // 05 - muzyka pocz�tkowa - tylko 1 utw�r odgrywany kiedy robot nie jest po��czony z appk�
						  // 06 - jeden dzwi�k witaj�cy po nast�pieniu parowania

	const int defGlosnoscLewy = 30; //0..30 , Domyslnie dzwieki
	const int defGlosnoscPrawy = 15;//0..30	, Domyslnie muzyka
	const int defPoziomWyciszenia = 5;//1..15
	
	const int defKatMuzyka = 1; //katalog dla glosnika lewego , dla cyfry 1, fizycznie musi miec postac "001" 
	const int defKatEfekty = 2;//katalog dla glosnika prawego, dla cyfry 2, fizycznie musi miec postac "002"
	static const int iloscUtworow = 3; // musi sie faktycznie pokrywac z iloscia plikow w folderze oraz pozycjami 
	static const int iloscEfektow = 11; // ..zdefiniowanymi w listach
									   // Uwaga!! Sprawdzac warto��, zmiana podej�cia z listy efekt�w przesy�anych
									   // do kontrolera do : efekty nale�� do grupy dzwi�kowej
	efektDzwiekowy tabEfekty[iloscEfektow] = {
		{LOSOWE_GADANIE,1},//nudze sie , 
		{LOSOWE_GADANIE,2},//nuda,
		{LOSOWE_GADANIE,3},//pierd
		{LOSOWE_GADANIE,4},//ziew
		{PRZECHYL_BOK_MALY,5},//niebezpiecznie
		{PRZECHYL_BOK_MALY,6},//musze uwazac
		{PRZECHYL_BOK_MALY,7},//czy to dobry pomysl 
		{PRZECHYL_BOK_DUZY,8},//kurwa leze
		{PRZECHYL_BOK_DUZY,9},//podniescie mnie
		{PRZECHYL_BOK_DUZY,10},//aua
		{PRZECHYL_BOK_DUZY,11} //kolko mnie boli
	};

	const char* listaUtworowCenzura[iloscUtworow] = {		"Jazz 1",
															"Jazz 2",
															"Jazz 3"};

	const char* listaUtworowBezCenzury[iloscUtworow] = {	"Kung-fu Panda",
															"Deadpool - Bodies",
															"Fight back" };


	DFPlayerMini_Fast playerLewy;
	DFPlayerMini_Fast playerPrawy;

	TOR tor = defTor;
	TRYB_AUDIO  tryb = NORMALNY;
	int glosnoscLewy = defGlosnoscLewy;
	int glosnoscPrawy = defGlosnoscPrawy;
	int poziomWyciszenia = defPoziomWyciszenia;
	int katMuzyka = defKatMuzyka;
	int katEfekty = defKatEfekty;

	int muzykaPrzedWyciszeniem = glosnoscPrawy; // pomocnicza dla trybu : WYCISZANIE
	boolean muzykaPauza = false;
	boolean muzykaWyciszona = false; // pomocnicza dla trybu : WYCISZANIE
	int _PO=1; //tymczasowe rozwi�zanie z powodu problem�w z indexowaniem utwor�w na karcie SD.
			 // tu przechowujemy warto�� numeru nagrania w katalogu aktualnie odtwarzanego utworu muzycznego.
			 // mo�na m�wi� raczej o numerze porz�dkowym w katalogu nie za� o unikalnym indexie w kontekscie ca�ej karty SD.

	//efekty na podstawie zyroskopu
	YtesZyroskop* zyroskop = nullptr;
	unsigned long msPrzerwaEfektyBaza = 7000L;//minimalny czas przerwy pomi�dzy odegraniem efektow. Znacznik czasu
										  // uwzgl�dniany tylko i wy��cznie dla auto-dzwi�kow na podstawie wskazan zyroskopu.
	unsigned long msPrzerwaEfektyLos = 5000L; //z tego zakresu b�dzie losowana liczba przed�u�aj�ca czas
	unsigned long msOstatniAutoEfekt = 0L;

	//efekty na podstawie  radaru
	YtesRadar* radar = nullptr;
	int indexDlaMuzyki(int numerNagrania);
	void _YtesAudio(); // wspolny kod dla konstruktorow (SoftwareSerial / HardwareSerial);
	void wyciszMuzyke();		//metoda pomocnicza do trybu WYCISZANIE
	void podglosnijMuzyke();	//metoda pomocnicza do trybu WYCISZANIE

public:
	bool uwzglednijZyroskop = false;
	bool uwzglednijRadar = false;

	byte ileEfektowWGrupie(GRUPA_DZWIEKOWA grupa);
	byte losujZgrupy(GRUPA_DZWIEKOWA grupa);
	int indexMuzykaKatalog() { return _PO; }; //Obej�cie : kt�ry utw�r jest ustawiony do grania,z wzgl�du na tor

	YtesAudio(SoftwareSerial* serialportLewego, SoftwareSerial* serialportPrawego,int czasOpoznieniaPolecenia);
	~YtesAudio();
	void dodajZyroskop(YtesZyroskop* pZyroskop);
	void dodajRadar(YtesRadar* pRadar);
	
	void ustawTrybAudio(TRYB_AUDIO nowyTryb);
	TRYB_AUDIO wezTrybAudio();

	void ustawTor(TOR nowyTor);
	TOR wezTor();

	void ustawPoziomWyciszenia(int nowyPoziom);
	int wezPoziomWyciszenia();

	void grajMuzyke(int nrNagrania);
	void grajEfekt(int  nrNagrania); 
	void graj(KANAL kanal, TYP_AUDIO typ,  int nrNagrania);

	void grajMuzykePowitalna();
	void grajEfektPowitalny();
	
	void glosnosc(KANAL kanal, int wartosc);
	void obslozPolecenieDane(JsonObject* dane); //obsluga samego obiektu "dane"
	String odpowiedz();

	void audioHandler();
	void dump(KANAL odtwarzaczSymbol); //L lub P
};

void YtesAudio::_YtesAudio() {
	//LEWY
	AUDIO_INFO("[Audio konstruktor] -> LEWY przygotowanie");
	if (!playerLewy.begin(serialLewy, false, polecenieOpoznienie)) {
		Serial.println("Unable to begin: LEWY");
	};
	AUDIO_INFO("[Audio konstruktor] -> stop granie");
	delay(polecenieOpoznienie);
	playerLewy.stop();
	delay(polecenieOpoznienie);
	//playerLewy.playbackSource(TF);
	//delay(polecenieOpoznienie); //wazny delay - zgodny z th
	AUDIO_INFO("[Audio konstruktor] -> ustawienie glosnosci");
	playerLewy.volume(glosnoscLewy);
	delay(polecenieOpoznienie);

	//PRAWY
	AUDIO_INFO("[Audio konstruktor] -> PRAWY przygotowanie");
	if (!playerPrawy.begin(serialPrawy, false, polecenieOpoznienie)) {
		Serial.println("Unable to begin: Prawy");
	};
	AUDIO_INFO("[Audio konstruktor] -> stop granie");
	delay(polecenieOpoznienie);
	playerPrawy.stop();
	delay(polecenieOpoznienie);
	//playerLewy.playbackSource(TF);
	//delay(polecenieOpoznienie);
	AUDIO_INFO("[Audio konstruktor] -> ustawienie glosnosci");
	playerPrawy.volume(glosnoscPrawy); //wazny delay - zgodny z th
	delay(polecenieOpoznienie);
	//....
	msOstatniAutoEfekt = millis() +msPrzerwaEfektyBaza + random(msPrzerwaEfektyLos);
};

#if UZYJ_HARDWARE == 1
YtesAudio::YtesAudio(HardwareSerial* serialportLewego, HardwareSerial* serialportPrawego, int czasOpoznieniaPolecenia , YtesZyroskop* zyro) {
	portLewy = serialportLewego;
	portPrawy = serialportPrawego;
	polecenieOpoznienie = czasOpoznieniaPolecenia;
	_YtesAudio();
};
#else 
YtesAudio::YtesAudio(SoftwareSerial* serialportLewego, SoftwareSerial* serialportPrawego, int czasOpoznieniaPolecenia) {
	portLewy = serialportLewego;
	portPrawy = serialportPrawego;
	polecenieOpoznienie = czasOpoznieniaPolecenia;
	_YtesAudio();
};
#endif

/*
* @brief Zatrzymaj odgrywanie nagra� na playerach. Zatrzymaj nas�uch na przydzielonych UARTACH
*/
YtesAudio::~YtesAudio() {
	playerLewy.stop();
	playerPrawy.stop();
	serialLewy.end();
	serialPrawy.end();
};

/**
* @brief Zainicjalizuj wskazany zyroskop. Sprawd� status zyroskopu, je�li jest niepoprawny 
* ustaw go jako null.
*/
void YtesAudio::dodajZyroskop(YtesZyroskop* pZyroskop) {
	zyroskop = pZyroskop;
	if (zyroskop != nullptr) {
		bool zyrStatus = zyroskop->pobierzStatus();
		if (!zyrStatus) {
			zyroskop = nullptr;
			AUDIO_ERROR("[audio] - obiekt zyroskopu nie zosta� poprawnie zainicjalizowany.");
		}
		else {
			AUDIO_INFO("[audio] dodano zyroskop.");
		};
	}
	else {
		AUDIO_ERROR("[audio] Blad! nie dodano zyroskopu.");
	}
};
/**
* @brief Zainicjalizuj wskazany radar.
*/
void YtesAudio::dodajRadar(YtesRadar* pRadar) {
	radar = pRadar;
};

/**
* @brief Wycisz muzyke
* TODO : po testach do private
*/
void YtesAudio::wyciszMuzyke() {
	muzykaPrzedWyciszeniem = glosnoscLewy;
	int nowaGlosnosc = muzykaPrzedWyciszeniem - poziomWyciszenia;
	if (nowaGlosnosc < 0) nowaGlosnosc = 0;
	AUDIO_INFO_V("[audio](wyciszenie) nowa glosnosc : ", nowaGlosnosc);
	playerPrawy.volume(nowaGlosnosc);													///
};

/**
* @brief Podg�o�nij muzyke
* TODO: po  testach do private
*/
void YtesAudio::podglosnijMuzyke() {
	AUDIO_INFO_V("[audio](wyciszenie) powracam do poziomu glosnosci : ", muzykaPrzedWyciszeniem);
	playerPrawy.volume(muzykaPrzedWyciszeniem);
};

/**
* @brief Na podstawie toru audio : CENZURA, BEZ_CENZURY
* Uwaga mp3Player - indexuje nagrania w swoj unikalny pojebany sposob, nie zawsze s� to indexy narasataj�co.
* w rozumieniu : kat 01/5 utworow to w kat 02/ zaczynamy od 6. Aby bylo zabawniej w zale�no�ci od typu scalaka
* (mamy w ofercie bodaj 7 roznych) indexowanie moze wygladac zupelnie inaczej. Sport dla masochistow.
* TODO : Dobra� identyczne scalaki , kt�re TAK SAMO indeksuj� utwory na SD
* Funkcja niech zostanie.
* oblicz fizyczny index nagrania.
* Docelowo wskazany w programie utwor bedzie zapetlany playerPrawy.loop();
*/
int YtesAudio::indexDlaMuzyki(int numerNagrania) {
	int retIndex = 0;
	switch (tor) {
	case CENZURA:
		retIndex = numerNagrania;
		break;
	case BEZ_CENZURY:
		retIndex = iloscUtworow + iloscEfektow + numerNagrania;
		break;
	default:
		AUDIO_ERROR("Blad ![audio] -> ustawiono domyslny index dla zapetlanego utworu : 1");
		retIndex = 1;
		break;
	};
	AUDIO_INFO_V("Index dla utworu: ", retIndex);
	return retIndex;
};

/*
* @brief Zliscz faktyczn� ilo�� dodanych efekt�w d�wi�kowych dodanych do danej grupy
*/
byte YtesAudio::ileEfektowWGrupie(GRUPA_DZWIEKOWA grupa) {
	byte ret = 0;
	for (int i = 0; i < iloscEfektow; i++) {
		if (tabEfekty[i].grupa == grupa) ret++;
	}
	return ret;
};

/**
 * @brief Iteruj tablic� zliczaj�c ilosc element�w nale��cych do grupy.
 * Na tej podstawie wylosuj dzwi�k.
 * 0 oznacza brak dzwi�k�w dla danej grupy lub b��d.
 * 
*/
byte YtesAudio::losujZgrupy(GRUPA_DZWIEKOWA grupa) {
	byte ilosc = ileEfektowWGrupie(grupa);
	byte krok = 0;
	if (ilosc == 0) return 0;
	byte los = random(1,ilosc+1);
	for (int i = 0; i < iloscEfektow; i++) {
		if (tabEfekty[i].grupa == grupa) {
			krok++;
			if (krok == los) {
				return tabEfekty[i].index;
			};
		};
	};
	return 0;
};
//--------------------------PUBLICZNE ------------------------------------------------
/**
* @brief Zmien tor nagrania , przypisz odpowiednie katalogi z nagraniami w wersji cenzuralnej i niecenzuralnej
*/
void YtesAudio::ustawTor(TOR nowyTor) {
	switch (nowyTor) {
	case CENZURA:
		katMuzyka = 1;
		katEfekty = 2;
		tor = CENZURA;
		AUDIO_INFO("[audio] -> ustawiono tor z cenzura.");
		break;
	case BEZ_CENZURY:
		katMuzyka = 3;
		katEfekty = 4;
		tor = BEZ_CENZURY;
		AUDIO_INFO("[audio] -> ustawiono tor bez cenzury.");
		break;
	default:
		katMuzyka = 1;
		katEfekty = 2;
		tor = CENZURA;
		AUDIO_ERROR("[audio] -> uwaga ! Nieznany tor , ustawiono domyslny cenzuralny");
		break;
	};
};
/**
* @brief Pobierz auktualne ustawienia toru
*/
TOR YtesAudio::wezTor() {
	return tor;
};

/**
* @brief Ustaw nowy poziom wyciszenia. Je�li parametr przekracza ustaw go na domy�ln� warto��
*/
void YtesAudio::ustawPoziomWyciszenia(int nowyPoziom) {
	int nowaWartosc = nowyPoziom;
	if (nowaWartosc < 0) nowaWartosc = 0;
	if (nowaWartosc > 15) nowaWartosc = 15;
	poziomWyciszenia = nowaWartosc;
};

/**
* @brief Pobierz aktualn� warto�� poziomu wyciszenia
*/
int YtesAudio::wezPoziomWyciszenia() {
	return poziomWyciszenia;
};

/**
* @brief Metoda z za�o�enia ustawiona raz w configu. , Uwaga ! warto�� nrNagrania == 0 spowoduje odgrywanie 
* utwor�w po kolei
*/
void YtesAudio::grajMuzyke(int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > iloscUtworow) wartosc = 0;
	_PO = wartosc; // tymczasowe obej�cie problemu z indexowaniem utwor�w
	if (wartosc == 0) {
		AUDIO_INFO_V("Zapetlam katalog z muzyka :", katMuzyka);
		playerPrawy.repeatFolder(katMuzyka);
		if (tryb == STEREO_MUZYKA) {
			playerLewy.repeatFolder(katMuzyka);
			}
		return;
	};

	AUDIO_INFO_V("Gram muzyke :", wartosc);
	switch (tryb) {
	case NORMALNY:
		playerPrawy.playFolder(katMuzyka, wartosc);
		//playerPrawy.loop(indexDlaMuzyki(nrNagrania));
		break;
	case STEREO_MUZYKA:
		playerPrawy.playFolder(katMuzyka, wartosc);
		playerLewy.playFolder(katMuzyka, wartosc);
		//playerPrawy.loop(indexDlaMuzyki(nrNagrania));
		//playerLewy.loop(indexDlaMuzyki(nrNagrania));
		break;
	case STEREO_EFEKTY:
		//nic nie robimy
		break;
	case WYCISZANIE:
		//zmian� g�o�no�ci tego kana�u powoduje metoda grajEfekt();
		// powrot do poprzedniej wartosci zapewnia audioHandler();
		playerPrawy.playFolder(katMuzyka, wartosc);
		//playerPrawy.loop(indexDlaMuzyki(nrNagrania));
		break;
	case PALZOWANIE:
		//zmian� g�o�no�ci tego kana�u powoduje metoda grajEfekt();
		//powrot do poprzedniej wartosci zapewnia audioHandler();
		playerPrawy.playFolder(katMuzyka, wartosc);
		//playerPrawy.loop(indexDlaMuzyki(nrNagrania));
		break;
	default:
		break;
	};
};

/*
* @brief Metoda w za�o�eniu wyko�ystywana przez samego robota. Efekt odgrywany na podniesienie, uderzenie , itp.
* Metoda mo�e modyfikowa� g�o�no�c kana�u prawego(muzyka) z wzgl�du na ustawienie trybu audio.
*/
void YtesAudio::grajEfekt(int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > iloscEfektow) wartosc = 0;

	AUDIO_INFO_V("Gram efekt:", wartosc);
	switch (tryb) {
	case NORMALNY:
		playerLewy.playFolder(katEfekty, wartosc);
		break;
	case STEREO_MUZYKA:
		//ignoruj muzyka jest priorytetem
		break;
	case STEREO_EFEKTY:
		playerPrawy.playFolder(katEfekty, wartosc);
		playerLewy.playFolder(katEfekty, wartosc);
		break;
	case WYCISZANIE:
		muzykaWyciszona = true;
		wyciszMuzyke();
		playerLewy.playFolder(katEfekty, wartosc);
		break;
	case PALZOWANIE:
		muzykaPauza = true;
		playerPrawy.pause();
		playerLewy.playFolder(katEfekty, wartosc);
		break;
	default:
		break;
	};
};

/*
* Graj nagranie dla wskazanego kanalu(glosnika). Podanie < 0 spowoduje zatrzymanie nagrania dla 
* danego kana�u. Przekroczenie zakresow 0..255 , ustawi wartosc na 0
* 
* Nadrz�dny bedzie tryb przykladowo GrajMuzyke(jesli  normalny  to tylko lewy glosnik , jesli stereo_muzyka to oba glosniki,
* jesli stereo_efekt to ignoruj kompletnie
* //GrajEfekt  
* //GrajMuzyke
*/
void YtesAudio::graj(KANAL kanal, TYP_AUDIO typ, int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > 255) wartosc = 0;
	
	switch (kanal) {
	case LEWY:
		if (wartosc == 0) playerLewy.stop();
		else {
			if (typ == MUZYKA) playerLewy.playFolder(katMuzyka, wartosc);
			else playerLewy.playFolder(katEfekty, wartosc);
		}
		AUDIO_INFO_V("Kanal lewy , nagranie nr :",  wartosc);
		break;
	case PRAWY:
		if (wartosc == 0) playerPrawy.stop();
		else {
			if (typ == MUZYKA) playerPrawy.playFolder(katMuzyka, wartosc);
			else playerPrawy.playFolder(katEfekty, wartosc);
		};
		AUDIO_INFO_V("Kanal prawy , nagranie nr :", wartosc);
		break;

	default:
		break;
	};
};

/**
* @brief Graj utw�r powitalny po w��czeniu robota. Jeden utwor znajduj�cy si� w katalogu numer :05
*/
void YtesAudio::grajMuzykePowitalna() {
	playerPrawy.playFolder(5, 1);
};

/**
* @brief Grej efekt powitalny po w��czeniu robota. Jeden efekt znajduj�cy si� w katalogu numer : 06
*/
void YtesAudio::grajEfektPowitalny() {
	playerLewy.playFolder(6, 1);
};

/**
* @brief Metoda musi byc ci�gle wywolywana w glownej p�tli programu. W przypadku trybu pracy : PALZOWANIE, WYCISZANIE
* sprawdza czy efekt specjalny zakonczyl sie. Jesli tak przywraca poprzedni� g�o�no�� lub odpa�zowuje glowny utwor.
*/
void YtesAudio::audioHandler() {
	// obs�uga na podstawie radaru
	if (radar != nullptr && millis() > msOstatniAutoEfekt && uwzglednijRadar == true) {
		//jesli costam costam
		// msOstatniAutoEfekt = millis() + msPrzerwaEfektyBaza + random(msPrzerwaEfektyLos);
	};
	//obs�uga efekt�w dzwi�kowych na podstawie wskazan zyroskopu.
	if (zyroskop != nullptr && millis() > msOstatniAutoEfekt && uwzglednijZyroskop == true) {
		STAN_ZYROSKOP stan = zyroskop->pobierzStan();
		byte indexEfektu;
		msOstatniAutoEfekt = millis() + msPrzerwaEfektyBaza + random(msPrzerwaEfektyLos);
		zyroskop->wypiszStan(false);
		switch (stan) {
			case STAN_ZYROSKOP::BRAK:
				indexEfektu = losujZgrupy(LOSOWE_GADANIE);				
				break;
			case STAN_ZYROSKOP::LEWO_PRZECHYL:
				indexEfektu = losujZgrupy(PRZECHYL_BOK_MALY);
				break;
			case STAN_ZYROSKOP::LEWO_LEZE:
				indexEfektu = losujZgrupy(PRZECHYL_BOK_DUZY);
				break;
			case STAN_ZYROSKOP::PRAWO_PRZECHYL:
				break;
			case STAN_ZYROSKOP::PRAWO_LEZE:
				break;
			case STAN_ZYROSKOP::PRZOD_1:
				break;
			case STAN_ZYROSKOP::PRZOD_2:
				break;
			case STAN_ZYROSKOP::TYL_1:
				break;
			case STAN_ZYROSKOP::TYL_2:
				break;
			case STAN_ZYROSKOP::WSTRZAS:
				break;
			default:
				break;
		};
		if (indexEfektu > 0) {
			grajEfekt(indexEfektu);
		};
	};

	//Przywr�� poprzedni� g�o�no�� lub odpa�zuj utw�r
	if (muzykaWyciszona || muzykaPauza) {
		if (portLewy->available()) {
			bool lewyGra = playerLewy.isPlaying();
			AUDIO_INFO_V("[audio] handler -> stan lewy :", lewyGra);
			if (muzykaWyciszona && !lewyGra) {
				podglosnijMuzyke(); //metoda sama z siebie generuje informacje 
				muzykaWyciszona = false;
			};
			if (muzykaPauza && !lewyGra) {
				AUDIO_INFO("[audio] handler -> odpalzowanie");
				playerPrawy.resume();
				muzykaPauza = false;
			};
		};
	};
};

/**
* @brief Zmien tryb audio na : NORMALNY , STEREO_MUZYKA ,STEREO_EFEKTY , WYCISZANIE , PALZOWANIE
*/
void YtesAudio::ustawTrybAudio(TRYB_AUDIO nowyTryb) {
	tryb = nowyTryb;
};

/**
* @brief Metoda zwraca aktualny ustawiony tryb audio.
* Mozliwe warto�ci : NORMALNY , STEREO_MUZYKA ,STEREO_EFEKTY , WYCISZANIE , PALZOWANIE
*/
TRYB_AUDIO YtesAudio::wezTrybAudio() {
	return tryb;
};

/*
* Ustaw g�o�no�� dla lewego lub prawgo g�o�nika. Metoda sprawdza poprawnosc zakresow 0..30
*/
void YtesAudio::glosnosc(KANAL kanal, int wartosc) {
	int nowaWartosc = wartosc;
	if (nowaWartosc < 0) nowaWartosc = 0;
	if (nowaWartosc > 30) nowaWartosc = 30;

	switch (kanal) {
	case LEWY:
		glosnoscLewy = nowaWartosc;
		playerLewy.volume((byte)nowaWartosc);
		break;
	case PRAWY:
		glosnoscPrawy = nowaWartosc;
		playerPrawy.volume((byte)nowaWartosc);
		break;
	default:
		break;
	};
};

/*
* Obsluga obiektu opisuj�cego audio. Obiekt mo�e zawiera� dane :
* {
*		"TOR" : 0..1	<- 0 - cenzura  , 1 - bez cenzury
*		"TRYB" : 0..4	<- NORMALNY , STEREO_MUZYKA ,STEREO_EFEKTY , WYCISZANIE , PALZOWANIE
		"LG" : 25,		<- Lewy glosnosc (domyslnie muzyka)
		"PG" : 15,		<- Prawy glosnosc (domyslnie efekty dzwiekowe)
		"PW" : 7		<- Poziom wyciszenia
		"LO" : 3		<- Lewy tor odtwarzanie(domy�lnie efekt)
		"PO" : 3		<- Prawy tor odtwarzanie (domy�lnie muzyka)
* }
* W przypadku LO,PO - z kt�rych katalog�w b�d�  odtwarzane nagrania, decyduje "TOR"
*/
void YtesAudio::obslozPolecenieDane(JsonObject* dane) {
	//const char* vLG = (*dane)["KLUCZ"].as<const char*>(); //lyka
	//int noweDane = (*dane)["LG"].as<int>();//lyka
	//if (noweDane) {}
	//tor audio : cenzura/ bez_cenzury
	JsonVariant vTor = (*dane)["TOR"];
	if (!vTor.isNull()) {
		ustawTor((TOR)vTor.as<int>());
	};
	//tryb : 
	JsonVariant vTryb = (*dane)["TRYB"];
	if (!vTryb.isNull()) {
		ustawTrybAudio((TRYB_AUDIO)vTryb.as<int>());
	}
	//lewy kanal glosnosc
	AUDIO_INFO("AUDIO -  blok dane");
	JsonVariant vLG = (*dane)["LG"];
	if (!vLG.isNull()) {
		glosnosc(LEWY, vLG.as<int>());
		AUDIO_INFO_V("Glosnik lewy glosnosc ustawiona :", vLG.as<int>());
	}
	//prawy kanal  glosnosc
	JsonVariant vPG = (*dane)["PG"];
	if (!vPG.isNull()) {
		glosnosc(PRAWY, vPG.as<int>());
		AUDIO_INFO_V("Glosnik prawy glosnosc ustawiona :", vPG.as<int>());
	};
	//poziom wyciszenia
	JsonVariant vPW = (*dane)["PW"];
	if (!vPW.isNull()) {
		ustawPoziomWyciszenia(vPW.as<int>());
	};
	//Odtwarzany kanal prawy (muzyka)
	JsonVariant vPO = (*dane)["PO"];
	if (!vPO.isNull()) {
		grajMuzyke(vPO.as<int>());
	};
	//Odtwarzany kanal lewy (efekty)
	JsonVariant vLO = (*dane)["LO"];
	if (!vLO.isNull()) {
		grajEfekt(vLO.as<int>());
	};
	//Uwzglednij zyroskop
	JsonVariant vUZ = (*dane)["UZ"];
	if (!vUZ.isNull()) {
		uwzglednijZyroskop = vUZ.as<boolean>();
	}
	//Uwzglednij radar
	JsonVariant vUR = (*dane)["UR"];
	if (!vUR.isNull()) {
		uwzglednijRadar = vUR.as<boolean>();
	}
};

/*
* Odpowiedz kontrolera jako string bo JsonObiect to referencja do istniejacego dokumentu son. 
* Potrzebny byl by osobny doc i wskazywanie na niego. Duze prawdopodobienstwo memory leak.
*/
String YtesAudio::odpowiedz() {
	StaticJsonDocument<1024> doc;
	//DynamicJsonDocument doc(1024);
	JsonObject objData = doc.createNestedObject("audio");	
	objData["TOR"] = (byte)tor;
	objData["TRYB"] = (byte)tryb;
	objData["LG"] = glosnoscLewy;
	objData["PG"] = glosnoscPrawy;
	objData["PW"] = poziomWyciszenia;
	objData["PO"] = _PO;
	objData["UZ"] = (int)uwzglednijZyroskop;
	objData["UR"] = (int)uwzglednijRadar;

	JsonArray arrL1 = objData.createNestedArray("L1"); // cenzuralne nagrania
	JsonArray arrL2 = objData.createNestedArray("L2"); // niecenzuralne nagrania
	
	for (int i = 0; i < iloscUtworow; i++) {
		arrL1.add(listaUtworowCenzura[i]);
	};
	
	for (int i = 0; i <iloscUtworow; i++) {
		arrL2.add(listaUtworowBezCenzury[i]);
	};	

	String tmp="";
	size_t resSize = serializeJson(doc, tmp);
		
	int firstIndex = tmp.indexOf('{');	
	int lastIndex = tmp.lastIndexOf('}');
	String odp = tmp.substring(firstIndex + 1, lastIndex);
	return odp;
}
//------------------------DEBUG----------------------------------------------------
void YtesAudio::dump(KANAL odtwarzaczSymbol) {
#if SERIAL_AUDIO_INFO
	DFPlayerMini_Fast* odtwarzacz;
	if (odtwarzaczSymbol == LEWY) odtwarzacz = &playerLewy;
	else if (odtwarzaczSymbol == PRAWY) odtwarzacz = &playerPrawy;
	int sciezkaNr = odtwarzacz->currentSdTrack();
	int glosnosc = odtwarzacz->currentVolume();
	int gram = odtwarzacz->isPlaying();
	Serial.print("Mp3 :"); Serial.println(odtwarzaczSymbol);
	Serial.print("odtwarzam		 : "); Serial.println(gram);
	Serial.print("nagranie index : "); Serial.println(sciezkaNr);
	Serial.print("glosnosc       : "); Serial.println(glosnosc);
#endif
};


#endif