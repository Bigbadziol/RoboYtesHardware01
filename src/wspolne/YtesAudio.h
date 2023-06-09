// YtesAudio.h
//TODO : przetestowac rzutowanie int->bool

#ifndef _YTESAUDIO_h
#define _YTESAUDIO_h

#include "setup.h"
#include "DFPlayerMini_Fast.h" // DFPlayerMini_Fast by PowerBroker2  Version 1.2.4
							   // https://github.com/PowerBroker2/DFPlayerMini_Fast
#include "YtesAudioPolecenie.h"
#include "YtesZyroskop.h"
#include "YtesRadar.h"
#include "YtesNaped.h"

using namespace dfplayer;

enum TOR {CENZURA, BEZ_CENZURY};
enum KANAL {LEWY,PRAWY};
enum TYP_AUDIO {MUZYKA,EFEKT};
enum TRYB_AUDIO {NORMALNY,
//				STEREO_MUZYKA, //porazka, z czasem odgrywane utwory rozje�dzaj� si� w czasie (wy��czone)
//				STEREO_EFEKTY, //porazka, nie taka sam jak z muzyk� bo efekty du�o kr�tsze  (wy��czone)
				WYCISZANIE,
				PALZOWANIE};
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


//========================================================================================
class YtesAudio {
private:
	int polecenieOpoznienie = 500; //rozny czas odpowiedzi konkretnych scalak�w(7 rodz.) dla modu��w mp3. Od 100ms do nawet 500ms.
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

	const int defGlosnoscLewy = 15; //0..30 , Domyslnie dzwieki
	const int defGlosnoscPrawy = 15;//0..30	, Domyslnie muzyka
	const int defPoziomWyciszenia = 5;//1..15
	
	const int defKatMuzyka = 1; //katalog dla glosnika lewego , dla cyfry 1, fizycznie musi miec postac "001" 
	const int defKatEfekty = 2;//katalog dla glosnika prawego, dla cyfry 2, fizycznie musi miec postac "002"
	static const int iloscUtworow = 6; // musi sie faktycznie pokrywac z iloscia plikow w folderze oraz pozycjami 
	static const int iloscEfektow = 34; // ..zdefiniowanymi w listach
									   // Uwaga!! Sprawdzac warto��, zmiana podej�cia z listy efekt�w przesy�anych
									   // do kontrolera do : efekty nale�� do grupy dzwi�kowej


	efektDzwiekowy tabEfekty[iloscEfektow] = {
		{LOSOWE_GADANIE,1},			// dlugo stac rozpierducha			,	gwizd
		{LOSOWE_GADANIE,2},			// nudze sie						,	nuda
		{LOSOWE_GADANIE,3},			// zaczniemy cos robic				,	ziewanie
		{PRZECHYL_BOK_MALY,4},		// niebezpiecznie					,	nie rowno
		{PRZECHYL_BOK_MALY,5},		// telepie jak w ruskim czolgu		,	musze uwazac
		{PRZECHYL_BOK_MALY,6},		// czy to dobry pomysl				,	ulala
		{PRZECHYL_BOK_DUZY,7},		// lezec mam az zardzewieje			,
		{PRZECHYL_BOK_DUZY,8},		// podnies mnie						,
		{PRZECHYL_BOK_DUZY,9},		// wysokie ac						,
/*
		{PRZECHYL_PRZOD_MALY,10},	// co jeszcze mam zrobic			,
		{PRZECHYL_PRZOD_MALY,11},	// moglo byc latwiej				,
		{PRZECHYL_PRZOD_MALY,12},	// zadne wyzwanie					,
		{PRZECHYL_PRZOD_DUZY,13},	// bialy montarz					,
		{PRZECHYL_PRZOD_DUZY,14},	// jak spiderman					,
		{PRZECHYL_PRZOD_DUZY,15},	// wspinac sie mam					,
		{PRZECHYL_TYL_MALY,16},		// jak na sankach					,
		{PRZECHYL_TYL_MALY,17},		// no luz oszczedzamy paliwo		,
		{PRZECHYL_TYL_MALY,18},		// z gorki latwiej					,
		{PRZECHYL_TYL_DUZY,19},		// adam malysz						,
		{PRZECHYL_TYL_DUZY,20},		// stromo tu						,
		{PRZECHYL_TYL_DUZY,21},		// zeby sibie wybije				,
*/
		//Zamieniono programowo dzwieki
		{PRZECHYL_PRZOD_MALY,16},	// jak na sankach					,
		{PRZECHYL_PRZOD_MALY,17},	// no luz oszczedzamy paliwo		,
		{PRZECHYL_PRZOD_MALY,18},	// z gorki latwiej					,
		{PRZECHYL_PRZOD_DUZY,19},	// adam malysz						,
		{PRZECHYL_PRZOD_DUZY,20},	// stromo tu						,
		{PRZECHYL_PRZOD_DUZY,21},	// zeby sibie wybije				,

		{PRZECHYL_TYL_MALY,10},		// co jeszcze mam zrobic			,
		{PRZECHYL_TYL_MALY,11},		// moglo byc latwiej				,
		{PRZECHYL_TYL_MALY,12},		// zadne wyzwanie					,
		{PRZECHYL_TYL_DUZY,13},		// bialy montarz					,
		{PRZECHYL_TYL_DUZY,14},		// jak spiderman					,
		{PRZECHYL_TYL_DUZY,15},		// wspinac sie mam					,



		{PRZYCISKI,22},				// gin								,
		{PRZYCISKI,23},				// latwiej bedzie wyfgrac			,
		{PRZYCISKI,24},				// liczylem na wiecej				,
		{PRZYCISKI,25},				// nie ucziekniesz przedemna		,
		{PRZYCISKI,26},				// syrena 104						,
		{PRZYCISKI,27},				// tylko uciekac potrafisz			,
		{PRZYCISKI,28},				// walczmy							,
		{RADAR_BLISKO,29},			// blisko bylo						,	bylo blisko
		{RADAR_BLISKO,30},			// o wlos							,	leb rozwalic
		{RADAR_BLISKO,31},			// wychamowalem						,	o maly wlos
		{WSTRZAS_POJAZDU,32},		// turbulencje						,
		{WSTRZAS_POJAZDU,33},		// telepie jak w czolgu				,
		{WSTRZAS_POJAZDU,34},		// zaraz zwymiotuje					,
	};

	const char* listaUtworowCenzura[iloscUtworow] = {		"Doom-Ethernal",
															"Kung-fu Panda",
															//"Tylko biedronka", //do wywalenia
															"Neffex-Fight back",
															"Lol-Get Jinxed",
															"Drowning pool-Bodies",
															"POD-Boom"};

	const char* listaUtworowBezCenzury[iloscUtworow] = {	"Czadoman-Ruda",
															"Eminem-Without me",
															"Gorillaz-19-2000",
															"Letni-Piekny fiat",
															//"Khali4mb-Hate niggas", // do wywalenia
															"One punch - Opening",
															"Lydka grubasa - Adelajda"
	};


	DFPlayerMini_Fast playerLewy;
	DFPlayerMini_Fast playerPrawy;
	YtesAudioPolecenie* poleceniaLewy;
	YtesAudioPolecenie* poleceniaPrawy;

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

	boolean _lewyPopStan = false; // 
	boolean _lewyAktStan = false; //
	FireTimer ftLewyGra;   // co 100ms sprawdzamy w audioHandlerze czy efekt : rozpocz�to granie, w trakcie grania,zakonczono granie,nic
						   // nie gra d�u�szy czas

	int _PO=1; //tymczasowe rozwi�zanie z powodu problem�w z indexowaniem utwor�w na karcie SD.
			 // tu przechowujemy warto�� numeru nagrania w katalogu aktualnie odtwarzanego utworu muzycznego.
			 // mo�na m�wi� raczej o numerze porz�dkowym w katalogu nie za� o unikalnym indexie w kontekscie ca�ej karty SD.

	YtesZyroskop* zyroskop = nullptr;
	YtesRadar* radar = nullptr;
	float _odlegloscRadarBlisko = 15.0f; // je�li odleg�o�� jest mniejsza od wskazanej, aktywuj efekt d�wi�kowy dla grupy RADAR_BLISKO
	YtesNaped* naped = nullptr;
	
	//od tego miejsca zdarzenia wp�ywaj�ce na odgrywanie dzwi�ku.
	boolean robotSparowany = false;
	unsigned long msOstatniRuchNaped = 0L;		// czas fizycznie wykonanego ruchu pojazdu , dane z naped lub uruchomie
	unsigned long msOstatniPrzyciskEfekt = 0L;	// to uaktualnia obsluzPolecenie dane , dla parametru 'LO' (lewy odtwarzanie) dla przyciskow 1-7 w apce
	unsigned long msOstatniRuchRadar = 0L;		// ostatni czas zdazenia z radaru przerywajacy nude.
	unsigned long msNuda = 0L;					// chwila od ktorej sprawdzany jest czas ,czy robot si� nudzi, zdarzenia : ruch , przycisk , radar od�wie�aja timer
												// inicjalizowany przez polaczenie
	unsigned long msNudaCzekaj = 10000L;		// czas oczekiwania na zdarzenie, potem zacznij gadac

	unsigned long msPrzerwaGadanieBaza = 5000L;	// minimalny czas przerwy pomi�dzy odegraniem efektow. Znacznik czasu
												// uwzgl�dniany tylko i wy��cznie dla auto-dzwi�kow na podstawie wskazan zyroskopu.
	unsigned long msPrzerwaGadanieLos = 5000L;	// z tego zakresu b�dzie losowana liczba przed�u�aj�ca czas
	unsigned long msNastepnyAutoEfekt = 0L;		// czas w przyszlosci, kiedy mozna odegrac nastepny efekt

	void _YtesAudio(); // wspolny kod dla konstruktorow (SoftwareSerial / HardwareSerial);


public:
	boolean uwzglednijNude = false;
	boolean uwzglednijZyroskop = false;
	boolean uwzglednijRadar = false;
	boolean uwzglednijNaped = false;

	int indexDlaMuzyki(int numerNagrania); 
	byte ileEfektowWGrupie(GRUPA_DZWIEKOWA grupa);
	byte losujZgrupy(GRUPA_DZWIEKOWA grupa);
	int indexMuzykaKatalog() { return _PO; }; //Obej�cie : kt�ry utw�r jest ustawiony do grania,z wzgl�du na tor

	YtesAudio(SoftwareSerial* serialportLewego, SoftwareSerial* serialportPrawego,int czasOpoznieniaPolecenia);
	~YtesAudio();

	void dodajZyroskop(YtesZyroskop* pZyroskop);
	void dodajRadar(YtesRadar* pRadar);
	void dodajNaped(YtesNaped* pNaped);
	void ustawSparowanie(boolean czySparowany) { robotSparowany = czySparowany; };
	void ustawCzasNudy(unsigned long znacznikCzasu) { msNudaCzekaj = znacznikCzasu; };
	void ustawCzasPrzerwaGadanie(unsigned long czasBazowy, unsigned long czasLosowy) { msPrzerwaGadanieBaza = czasBazowy; msPrzerwaGadanieLos = czasLosowy; };
	
	void ustawTrybAudio(TRYB_AUDIO nowyTryb);
	TRYB_AUDIO wezTrybAudio();

	void ustawTor(TOR nowyTor);
	TOR wezTor();

	void ustawPoziomWyciszenia(int nowyPoziom);
	int wezPoziomWyciszenia();

	void grajMuzyke(int nrNagrania);
	void grajEfekt(int  nrNagrania); 

	void graj(KANAL kanal, TYP_AUDIO typ,  int nrNagrania);
	void grajSdIndex(KANAL kanal , int index);

	void grajMuzykePowitalna();
	
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

	AUDIO_INFO("[Audio konstruktor] -> ustawienie glosnosci");
	delay(polecenieOpoznienie);
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

	AUDIO_INFO("[Audio konstruktor] -> ustawienie glosnosci");
	delay(polecenieOpoznienie);
	playerPrawy.volume(glosnoscPrawy); //wazny delay - zgodny z th
	delay(polecenieOpoznienie);
	//....
	msNastepnyAutoEfekt = millis() +msPrzerwaGadanieBaza + random(msPrzerwaGadanieLos);
	msNuda = millis() + msNudaCzekaj;
	poleceniaLewy = new YtesAudioPolecenie(&playerLewy, "LEWY", polecenieOpoznienie);
	poleceniaPrawy = new YtesAudioPolecenie(&playerPrawy, "PRAWY", polecenieOpoznienie);
	ftLewyGra.begin(100); // co 100ms sprawdzaj czy nast�pi�a zmiana statusu odgrywania efektu
};

#if UZYJ_HARDWARE == 1
YtesAudio::YtesAudio(HardwareSerial* serialportLewego, HardwareSerial* serialportPrawego, int czasOpoznieniaPolecenia ) {
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

/**
* @brief Zatrzymaj odgrywanie nagra� na playerach. Zatrzymaj nas�uch na przydzielonych UARTACH
*/
YtesAudio::~YtesAudio() {
	playerLewy.stop();
	playerPrawy.stop();
	serialLewy.end();
	serialPrawy.end();
	delete poleceniaLewy;
	delete poleceniaLewy;
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
* @brief Zainicjalizuj wskazany nap�d.
*/
void YtesAudio::dodajNaped(YtesNaped* pNaped) {
	naped = pNaped;
};

/**
* @brief Na podstawie toru audio : CENZURA, BEZ_CENZURY
* Uwaga mp3Player - indexuje nagrania w swoj unikalny pojebany sposob, nie zawsze s� to indexy narasataj�co.
* w rozumieniu : kat 01/5 utworow to w kat 02/ zaczynamy od 6. Aby bylo zabawniej w zale�no�ci od typu scalaka
* (mamy w ofercie bodaj 7 roznych) indexowanie moze wygladac zupelnie inaczej. Sport dla masochistow.
* Update :
* Ma by� pusty katalog 06 (wczesniej tam byl efekt powitalny) , indexowanie zaczyna dzialac poprawnie
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
		AUDIO_ERROR("Blad ![audio] -> ustawiono index 0 - graj kolejne nagranie");
		retIndex = 0;
		break;
	};
	AUDIO_INFO_V("(SD)Index dla utworu: ", retIndex);
	return retIndex;
};

/**
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
	if (nowaWartosc < 1) nowaWartosc = 1;
	if (nowaWartosc > 15) nowaWartosc = 15;
	poziomWyciszenia = nowaWartosc;
	AUDIO_INFO_V("[audio] -> Poziom wyciszenia : ", nowaWartosc);
};

/**
* @brief Pobierz aktualn� warto�� poziomu wyciszenia
*/
int YtesAudio::wezPoziomWyciszenia() {
	return poziomWyciszenia;
};

/**
* @brief Uwaga! teraz metoda doklada polecenie repeatFolder lub playFolder do listy polecen.
* Wywolanie metody nie spowoduje odegrania utworu, musi byc wywolanie audioHandler w p�tli programu.
* utwor�w po kolei
* Polecenie KOLEJKOWANE
*/
void YtesAudio::grajMuzyke(int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > iloscUtworow) wartosc = 0;
	_PO = wartosc; // tymczasowe obej�cie problemu z indexowaniem utwor�w
	if (wartosc == 0) {
		AUDIO_INFO_V("Zapetlam katalog z muzyka :", katMuzyka);
		poleceniaPrawy->dodaj(30, katMuzyka, 0); //repeatFolder(param1)
	}else {
		poleceniaPrawy->dodaj(20, katMuzyka, wartosc);//playFolder(param1, param2);
	};
};

/**
* @brief Metoda w za�o�eniu wyko�ystywana przez samego robota. Efekt odgrywany na podniesienie, uderzenie , itp.
* Metoda mo�e modyfikowa� g�o�no�c kana�u prawego(muzyka) z wzgl�du na ustawienie trybu audio.
* Podanie parametru 0 , powoduje zatrzymanie grania efektu.
* Uwaga! metoda dodaje teraz polecenie do listy polece� dla kana�u. Aby odegra� nale�y w p�tli g��wnej wywo�a� audioHandler.
* Polecenie KOLEJKOWANE
*/
void YtesAudio::grajEfekt(int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > iloscEfektow) wartosc = 0;
	if (wartosc == 0) {
		poleceniaLewy->dodaj(70, 0, 0); //stop
		return;
	}
	switch (tryb) {
	case NORMALNY:
		poleceniaLewy->dodaj(20, katEfekty, wartosc); //playFolder(param1, param2);
		break;
	case WYCISZANIE:
		muzykaWyciszona = true;
		//muzykaPrzedWyciszeniem = glosnoscPrawy; ..ustawiana teraz w poleceniu przychodz�cym
		
		int nowaGlosnosc;
		nowaGlosnosc = glosnoscPrawy - poziomWyciszenia;
		if (nowaGlosnosc < 0) nowaGlosnosc = 0;
		poleceniaPrawy->dodaj(10, nowaGlosnosc, 0);//volume(param1)
		poleceniaLewy->dodaj(20, katEfekty, wartosc);//playFolder(param1, param2);
		break;

	case PALZOWANIE:
		muzykaPauza = true;
		poleceniaPrawy->dodaj(50, 0, 0);//pause
		poleceniaLewy->dodaj(20, katEfekty, wartosc);//playFolder(param1, param2);
		break;
	default:
		break;
	};
};


/**
* @brief Graj nagranie audio na podstawie indexu pliku wzgledem ca�ej karty SD
* Uwaga ta metoda zapewnia bezposrednie odegranie utworu - polecenie NIE jest kolejkowane.
*/
void YtesAudio::grajSdIndex(KANAL kanal , int index) {
	int wartosc = index;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > 255) wartosc = 0;

	switch (kanal) {
	case LEWY:
		playerLewy.play(wartosc);
		AUDIO_INFO_V("(SdIndex) Kanal lewy , nagranie nr :", wartosc);
		break;
	case PRAWY:
		playerPrawy.play(wartosc);
		AUDIO_INFO_V("(SdIndex) Kanal prawy , nagranie nr :", wartosc);
		break;

	default:
		break;
	};
};

/**
* @brief Graj utw�r powitalny po w��czeniu robota. Jeden utwor znajduj�cy si� w katalogu numer :05
* Polecenie KOLEJKOWANE
*/
void YtesAudio::grajMuzykePowitalna() {
	poleceniaPrawy->dodaj(30, 5, 0); //repeatFolder 5
};

/**
* @brief Metoda musi byc ci�gle wywolywana w glownej p�tli programu. W przypadku trybu pracy : PALZOWANIE, WYCISZANIE
* sprawdza czy efekt specjalny zakonczyl sie. Jesli tak przywraca poprzedni� g�o�no�� lub odpa�zowuje glowny utwor.
*/
void YtesAudio::audioHandler() {
	// obs�uga polece� audio
	unsigned long msTeraz = millis();

	//Kontrola zmiany stanu (zakonczono granie efektu)
	if (ftLewyGra.fire()) {
		_lewyPopStan = _lewyAktStan;
		_lewyAktStan = playerLewy.isPlaying();		

		if (_lewyAktStan == false && _lewyPopStan == true) { //teoretycznie zakonczylo sie granie
			if (muzykaWyciszona) {
				AUDIO_INFO_V("[audio](wyciszenie) powracam do poziomu glosnosci : ", muzykaPrzedWyciszeniem);
				poleceniaPrawy->dodaj(10, muzykaPrzedWyciszeniem, 0); //volume(param1)
				muzykaWyciszona = false;
			};

			if (muzykaPauza) {
				AUDIO_INFO("[audio] handler -> odpalzowanie");
				poleceniaPrawy->dodaj(60, 0, 0); //resume
				muzykaPauza = false;
			};
		};
	};


	//TODO: w tej chwili zamieniona kolejno�� polece�, czy co� poprawi ???
	poleceniaLewy->obsluzPolecenia();
	poleceniaPrawy->obsluzPolecenia();

	if (!robotSparowany) return; //nie sparowany nic nie gramy


	// obs�uga na podstawie radaru
	if (uwzglednijRadar == true) {
		if (radar != nullptr) {
			if (msTeraz > msNastepnyAutoEfekt) {
				float odleglosc = radar->dystans();
				if (odleglosc < _odlegloscRadarBlisko) {
					byte indexEfektu = losujZgrupy(RADAR_BLISKO);
					msNastepnyAutoEfekt = msTeraz + msPrzerwaGadanieBaza + random(msPrzerwaGadanieLos);
					if (indexEfektu > 0) {
						grajEfekt(indexEfektu);
						return; //co� powiedzia�em , przerwij funkcj�
					};
				};
			};
		};
	};

	//obs�uga efekt�w dzwi�kowych na podstawie wskazan zyroskopu.
	if (uwzglednijZyroskop == true) {
		if (zyroskop != nullptr) {
			if (msTeraz > msNastepnyAutoEfekt) {
				byte indexEfektu;
				STAN_ZYROSKOP stan = zyroskop->pobierzStan(); //aktualizuj czas gadania (nuda) gdy cos innego)
				if (stan != STAN_ZYROSKOP::BRAK) {
					msNastepnyAutoEfekt = msTeraz + msPrzerwaGadanieBaza + random(msPrzerwaGadanieLos);
				};
				zyroskop->wypiszStan(false);
				switch (stan) {
				//Stan BRAK , w domysle nuda teraz obs�ugiwany osobno.
				case STAN_ZYROSKOP::LEWO_1:
					indexEfektu = losujZgrupy(PRZECHYL_BOK_MALY);
					break;
				case STAN_ZYROSKOP::LEWO_2:
					indexEfektu = losujZgrupy(PRZECHYL_BOK_DUZY);
					break;
				case STAN_ZYROSKOP::PRAWO_1:
					indexEfektu = losujZgrupy(PRZECHYL_BOK_MALY);
					break;
				case STAN_ZYROSKOP::PRAWO_2:
					indexEfektu = losujZgrupy(PRZECHYL_BOK_DUZY);
					break;
				case STAN_ZYROSKOP::PRZOD_1:
					indexEfektu = losujZgrupy(PRZECHYL_PRZOD_MALY);
					break;
				case STAN_ZYROSKOP::PRZOD_2:
					indexEfektu = losujZgrupy(PRZECHYL_PRZOD_DUZY);
					break;
				case STAN_ZYROSKOP::TYL_1:
					indexEfektu = losujZgrupy(PRZECHYL_TYL_MALY);
					break;
				case STAN_ZYROSKOP::TYL_2:
					indexEfektu = losujZgrupy(PRZECHYL_TYL_DUZY);
					break;
				case STAN_ZYROSKOP::WSTRZAS:
					indexEfektu = losujZgrupy(WSTRZAS_POJAZDU);
					break;
				default:
					break;
				};
				if (indexEfektu > 0) {
					grajEfekt(indexEfektu);
				};
			};//limit czasowy
		};//�yroskop nie null
	};//uwzgl�dnij �yroskop

	//uwzglednij gadanie dla nudy
	if (uwzglednijNude == true) {
		//Najpierw sprawdzamy czy nast�pi�o zdarzenie przerywaj�ce nud�
		// 1) zdarzenie od nap�du
		if (naped != nullptr) {
			msOstatniRuchNaped = naped->ostatniRuch();
			if (msOstatniRuchNaped > msNuda) msNuda = msOstatniRuchNaped;
		};

		// 2) zdarzenie (wewn�trzne) polecenie d�wi�ku , przyciski 1-7
		//    zmienna ustawia polecenie przychodz�ce 'LO' - lewy odtwarzanie, domy�lnie d�wi�ki
		if (msOstatniPrzyciskEfekt > msNuda) msNuda = msOstatniPrzyciskEfekt;

		// 3) zdarzenie od radaru , na ten moment przycisk nr :8
		if (radar != nullptr) {
			msOstatniRuchRadar = radar->ostatniRuch();
			if (msOstatniRuchRadar > msNuda) msNuda = msOstatniRuchRadar;
		};
		//czas up�yn�
		if (msTeraz >= msNuda + msNudaCzekaj) {
			if (msTeraz > msNastepnyAutoEfekt) {
				msNastepnyAutoEfekt = msTeraz + msPrzerwaGadanieBaza + random(msPrzerwaGadanieLos);
				byte indexEfektu = losujZgrupy(LOSOWE_GADANIE);
				if (indexEfektu > 0) {
					grajEfekt(indexEfektu);
					return; //co� powiedzia�em , przerwij funkcj�
				};
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
* @brief Ustaw g�o�no�� dla lewego lub prawgo g�o�nika. Metoda sprawdza poprawnosc zakresow 0..30
* Polecenie NIE kolejkowane, do bezpo�redniego wykonania
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
* @brief Obsluga obiektu opisuj�cego audio. Obiekt mo�e zawiera� dane :
* {
*		"TOR" : 0..1	<- 0 - cenzura  , 1 - bez cenzury
*		"TRYB" : 0..4	<- NORMALNY , STEREO_MUZYKA ,STEREO_EFEKTY , WYCISZANIE , PALZOWANIE
*		"LG" : 25,		<- Lewy glosnosc (domyslnie muzyka)
*		"PG" : 15,		<- Prawy glosnosc (domyslnie efekty dzwiekowe)
*		"PW" : 7		<- Poziom wyciszenia
*		"LO" : 3		<- Lewy tor odtwarzanie(domy�lnie efekt)
*		"PO" : 3		<- Prawy tor odtwarzanie (domy�lnie muzyka)
* }
* W przypadku LO,PO - z kt�rych katalog�w b�d�  odtwarzane nagrania, decyduje "TOR"
*/
void YtesAudio::obslozPolecenieDane(JsonObject* dane) {
	boolean torZmieniony = false;
	//tor
	JsonVariant vTor = (*dane)["TOR"];
	if (!vTor.isNull()) {
		int nTor = vTor.as<int>();
		if (nTor != tor) {
			ustawTor((TOR)vTor.as<int>());
			torZmieniony = true;
		};
	};

	//tryb
	JsonVariant vTryb = (*dane)["TRYB"];
	if (!vTryb.isNull()) {
		ustawTrybAudio((TRYB_AUDIO)vTryb.as<int>());
	}
	//lewy kanal glosnosc (efekty)
	JsonVariant vLG = (*dane)["LG"];
	if (!vLG.isNull()) {
		int lg = vLG.as<int>();
		if (glosnoscLewy != lg) {
			glosnoscLewy = lg;
			poleceniaLewy->dodaj(10, lg, 0);
			AUDIO_INFO_V("Glosnik lewy(efekty) , glosnosc ustawiona :", lg);
		}
		else {
			AUDIO_INFO("Glosnik lewy(efekty) , glosnosc bez zmian");
		};
	};
	//prawy kanal  glosnosc (muzyka)
	JsonVariant vPG = (*dane)["PG"];
	if (!vPG.isNull()) {
		int pg = vPG.as<int>();
		if (glosnoscPrawy != pg || torZmieniony == true) {
			glosnoscPrawy = pg;
			muzykaPrzedWyciszeniem = glosnoscPrawy;//teraz tu
			poleceniaPrawy->dodaj(10, pg, 0);
			AUDIO_INFO_V("Glosnik prawy(muzyka) , glosnosc ustawiona :", pg);
		}
		else {
			AUDIO_INFO("Glosnik prawy(muzyka), glosnosc bez zmian");
		};
	};
	//poziom wyciszenia
	JsonVariant vPW = (*dane)["PW"];
	if (!vPW.isNull()) {
		int pw = vPW.as<int>();
		ustawPoziomWyciszenia(pw);
	};
	//Odtwarzany kanal prawy (muzyka)
	JsonVariant vPO = (*dane)["PO"];
	if (!vPO.isNull()) {
		int po = vPO.as<int>();
		if ((po != _PO) || (torZmieniony == true)) {
			_PO = po;
			//poleceniaPrawy->dodaj(20, katMuzyka, po); //bo nie b�dzie wyciszac
			grajMuzyke(po);

		};
	};
	//Odtwarzany kanal lewy (efekty)
	JsonVariant vLO = (*dane)["LO"];
	if (!vLO.isNull()) {
		int lo = vLO.as<int>();
		msOstatniPrzyciskEfekt = millis();
		//poleceniaLewy->dodaj(20, katEfekty, lo); // bo nie b�dzie wyciszac
		grajEfekt(lo);
	};
	//Uwzglednij zyroskop
	JsonVariant vUZ = (*dane)["UZ"];
	if (!vUZ.isNull()) {
		uwzglednijZyroskop = vUZ.as<boolean>();
		AUDIO_INFO_V("[audio] -> uwzglednij zyroskop : ", uwzglednijZyroskop);
	};
	//Uwzglednij radar
	JsonVariant vUR = (*dane)["UR"];
	if (!vUR.isNull()) {
		uwzglednijRadar = vUR.as<boolean>();
		AUDIO_INFO_V("[audio] -> uwzglednij radar : ", uwzglednijZyroskop);
	};
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
	//objData["UZ"] = (byte)uwzglednijZyroskop;
	if (uwzglednijZyroskop) {
		objData["UZ"] = 1;
	}
	else {
		objData["UZ"] = 0;
	}
	//objData["UR"] = (byte)uwzglednijRadar;
	if (uwzglednijRadar) {
		objData["UR"] = 1;
	}
	else {
		objData["UR"] = 0;
	};


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