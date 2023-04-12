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

using namespace dfplayer;

enum TOR {CENZURA, BEZ_CENZURY};
enum KANAL {LEWY,PRAWY};
enum TYP_AUDIO {MUZYKA,EFEKT};
enum TRYB_AUDIO {NORMALNY,
//				STEREO_MUZYKA, //porazka, z czasem odgrywane utwory rozje¿dzaj¹ siê w czasie (wy³¹czone)
//				STEREO_EFEKTY, //porazka, nie taka sam jak z muzyk¹ bo efekty du¿o krótsze  (wy³¹czone)
				WYCISZANIE,
				PALZOWANIE};
enum GRUPA_DZWIEKOWA{
	LOSOWE_GADANIE,			//np. brak wskazan zyroskopu :nudze sie, ale lipa, 'ziewanie' , 'kichanie' , 'pierdzenie' ,'gwizdanie','czy sie stoi czy sie'
	PRZECHYL_BOK_MALY,		// to niebezpieczne, uwazaj troche, wiesz po ile ac ?
	PRZECHYL_BOK_DUZY,		// no i leze, dlugo bede lezal?, podnies mnie, boli mnie kolko, auaaaaa
	PRZECHYL_PRZOD_MALY,	//sam sie wspinaj, naprawde musze, nie mam sil
	PRZECHYL_PRZOD_DUZY,	//trzeba miec skrzydla nie kolka, co ja spiderman ? chyba cie pogielo
	PRZECHYL_TYL_MALY,		// zawsze lzej, chce na sanki, ³iiiii!
	PRZECHYL_TYL_DUZY,		//naprawde sie boje, zeby sobie wybije,
	WSTRZAS_POJAZDU,		//ojojo, brr , grrr, ale wieje,
	RADAR_BLISKO,			//blizej nie podjade , mam leb rozbic, a co to?
	PRZYCISKI				//dowolnoœæ 8 dzwiêków podpiêtych pod przyciski
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
	int polecenieOpoznienie = 500; //rozny czas odpowiedzi konkretnych scalaków(7 rodz.) dla modu³ów mp3. Od 100ms do nawet 500ms.
#if UZYJ_HARDWARE == 1
	HardwareSerial* portLewy;
	HardwareSerial* portPrawy;
#else
	SoftwareSerial* portLewy;
	SoftwareSerial* portPrawy;
#endif

	TOR defTor = CENZURA; //0..1 , 0-tor cenzura , 1-tor bez cenzury
						  //Tor definiuje z których fizycznie katalogów pobierane s¹ nagrania muzyczne i dŸwiêki
						  //Katalogi : (forma zapisu nazw katalogow ma kluczowa - liczba w zapisie 2-cyfrowym)
						  // 01 - cenzura muzyka
						  // 02 - cenzura efekty dŸwiêkowe
						  // 03 - bez cenzury muzyka
						  // 04 - bez cenzury dŸwiêki
						  // 05 - muzyka pocz¹tkowa - tylko 1 utwór odgrywany kiedy robot nie jest po³¹czony z appk¹
						  // 06 - jeden dzwiêk witaj¹cy po nast¹pieniu parowania

	const int defGlosnoscLewy = 15; //0..30 , Domyslnie dzwieki
	const int defGlosnoscPrawy = 15;//0..30	, Domyslnie muzyka
	const int defPoziomWyciszenia = 5;//1..15
	
	const int defKatMuzyka = 1; //katalog dla glosnika lewego , dla cyfry 1, fizycznie musi miec postac "001" 
	const int defKatEfekty = 2;//katalog dla glosnika prawego, dla cyfry 2, fizycznie musi miec postac "002"
	static const int iloscUtworow = 7; // musi sie faktycznie pokrywac z iloscia plikow w folderze oraz pozycjami 
	static const int iloscEfektow = 34; // ..zdefiniowanymi w listach
									   // Uwaga!! Sprawdzac wartoœæ, zmiana podejœcia z listy efektów przesy³anych
									   // do kontrolera do : efekty nale¿¹ do grupy dzwiêkowej


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
															"Tylko biedronka",
															"Neffex-Fight back",
															"Lol-Get Jinxed",
															"Drowning pool-Bodies",
															"POD-Boom"};

	const char* listaUtworowBezCenzury[iloscUtworow] = {	"Czadoman-Ruda",
															"Eminem-Without me",
															"Gorillaz-19-2000",
															"Letni-Piekny fiat",
															"Khali4mb-Hate niggas",
															"One punch - Opening",
															"Lydka grubasa - Adelajda"
	};


//	DFPlayerMini_Fast playerLewy;
//	DFPlayerMini_Fast playerPrawy;
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
	int _PO=1; //tymczasowe rozwi¹zanie z powodu problemów z indexowaniem utworów na karcie SD.
			 // tu przechowujemy wartoœæ numeru nagrania w katalogu aktualnie odtwarzanego utworu muzycznego.
			 // mo¿na mówiæ raczej o numerze porz¹dkowym w katalogu nie zaœ o unikalnym indexie w kontekscie ca³ej karty SD.

	//efekty na podstawie zyroskopu
	YtesZyroskop* zyroskop = nullptr;
	unsigned long msPrzerwaEfektyBaza = 7000L;//minimalny czas przerwy pomiêdzy odegraniem efektow. Znacznik czasu
										  // uwzglêdniany tylko i wy³¹cznie dla auto-dzwiêkow na podstawie wskazan zyroskopu.
	unsigned long msPrzerwaEfektyLos = 5000L; //z tego zakresu bêdzie losowana liczba przed³u¿aj¹ca czas
	unsigned long msOstatniAutoEfekt = 0L;

	//efekty na podstawie  radaru
	YtesRadar* radar = nullptr;
	float _odlegloscRadarBlisko = 20.0f; // jeœli odleg³oœæ jest mniejsza od wskazanej, graj efekt dŸwiêkowy dla grupy RADAR_BLISKO

	void _YtesAudio(); // wspolny kod dla konstruktorow (SoftwareSerial / HardwareSerial);


public:
	DFPlayerMini_Fast playerLewy; //po testach do private;
	DFPlayerMini_Fast playerPrawy;//po testach do private;

	bool uwzglednijZyroskop = false;
	bool uwzglednijRadar = false;

	int indexDlaMuzyki(int numerNagrania); 

	byte ileEfektowWGrupie(GRUPA_DZWIEKOWA grupa);
	byte losujZgrupy(GRUPA_DZWIEKOWA grupa);
	int indexMuzykaKatalog() { return _PO; }; //Obejœcie : który utwór jest ustawiony do grania,z wzglêdu na tor

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
	msOstatniAutoEfekt = millis() +msPrzerwaEfektyBaza + random(msPrzerwaEfektyLos);

	poleceniaLewy = new YtesAudioPolecenie(&playerLewy, "LEWY", polecenieOpoznienie);
	poleceniaPrawy = new YtesAudioPolecenie(&playerPrawy, "PRAWY", polecenieOpoznienie);
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
* @brief Zatrzymaj odgrywanie nagrañ na playerach. Zatrzymaj nas³uch na przydzielonych UARTACH
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
* @brief Zainicjalizuj wskazany zyroskop. SprawdŸ status zyroskopu, jeœli jest niepoprawny 
* ustaw go jako null.
*/
void YtesAudio::dodajZyroskop(YtesZyroskop* pZyroskop) {
	zyroskop = pZyroskop;
	if (zyroskop != nullptr) {
		bool zyrStatus = zyroskop->pobierzStatus();
		if (!zyrStatus) {
			zyroskop = nullptr;
			AUDIO_ERROR("[audio] - obiekt zyroskopu nie zosta³ poprawnie zainicjalizowany.");
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
* @brief Na podstawie toru audio : CENZURA, BEZ_CENZURY
* Uwaga mp3Player - indexuje nagrania w swoj unikalny pojebany sposob, nie zawsze s¹ to indexy narasataj¹co.
* w rozumieniu : kat 01/5 utworow to w kat 02/ zaczynamy od 6. Aby bylo zabawniej w zale¿noœci od typu scalaka
* (mamy w ofercie bodaj 7 roznych) indexowanie moze wygladac zupelnie inaczej. Sport dla masochistow.
* Update :
* Ma byæ pusty katalog 06 (wczesniej tam byl efekt powitalny) , indexowanie zaczyna dzialac poprawnie
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
* @brief Zliscz faktyczn¹ iloœæ dodanych efektów dŸwiêkowych dodanych do danej grupy
*/
byte YtesAudio::ileEfektowWGrupie(GRUPA_DZWIEKOWA grupa) {
	byte ret = 0;
	for (int i = 0; i < iloscEfektow; i++) {
		if (tabEfekty[i].grupa == grupa) ret++;
	}
	return ret;
};

/**
 * @brief Iteruj tablicê zliczaj¹c ilosc elementów nale¿¹cych do grupy.
 * Na tej podstawie wylosuj dzwiêk.
 * 0 oznacza brak dzwiêków dla danej grupy lub b³¹d.
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
* @brief Ustaw nowy poziom wyciszenia. Jeœli parametr przekracza ustaw go na domyœln¹ wartoœæ
*/
void YtesAudio::ustawPoziomWyciszenia(int nowyPoziom) {
	int nowaWartosc = nowyPoziom;
	if (nowaWartosc < 1) nowaWartosc = 1;
	if (nowaWartosc > 15) nowaWartosc = 15;
	poziomWyciszenia = nowaWartosc;
};

/**
* @brief Pobierz aktualn¹ wartoœæ poziomu wyciszenia
*/
int YtesAudio::wezPoziomWyciszenia() {
	return poziomWyciszenia;
};

/**
* @brief Uwaga! teraz metoda doklada polecenie repeatFolder lub playFolder do listy polecen.
* Wywolanie metody nie spowoduje odegrania utworu, musi byc wywolanie audioHandler w pêtli programu.
* utworów po kolei
* Polecenie KOLEJKOWANE
*/
void YtesAudio::grajMuzyke(int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > iloscUtworow) wartosc = 0;
	_PO = wartosc; // tymczasowe obejœcie problemu z indexowaniem utworów
	if (wartosc == 0) {
		AUDIO_INFO_V("Zapetlam katalog z muzyka :", katMuzyka);
///		playerPrawy.repeatFolder(katMuzyka);
		poleceniaPrawy->dodaj(30, katMuzyka, 0); //repeat folder
		return;
	};

	//AUDIO_INFO_V("Gram muzyke :", wartosc);
///		playerPrawy.playFolder(katMuzyka, wartosc);
		//playerPrawy.loop(indexDlaMuzyki(nrNagrania));
	poleceniaPrawy->dodaj(20, katMuzyka, wartosc);
};

/**
* @brief Metoda w za³o¿eniu wyko¿ystywana przez samego robota. Efekt odgrywany na podniesienie, uderzenie , itp.
* Metoda mo¿e modyfikowaæ g³oœnoœc kana³u prawego(muzyka) z wzglêdu na ustawienie trybu audio.
* Podanie parametru 0 , powoduje zatrzymanie grania efektu.
* Uwaga! metoda dodaje teraz polecenie do listy poleceñ dla kana³u. Aby odegraæ nale¿y w pêtli g³ównej wywo³aæ audioHandler.
* Polecenie KOLEJKOWANE
*/
void YtesAudio::grajEfekt(int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > iloscEfektow) wartosc = 0;
	if (wartosc == 0) {
///		playerLewy.stop();
		poleceniaLewy->dodaj(70, 0, 0); //stop
		return;
	}
	//AUDIO_INFO_V("Gram efekt:", wartosc);
	switch (tryb) {
	case NORMALNY:
///		playerLewy.playFolder(katEfekty, wartosc);
		poleceniaLewy->dodaj(20, katEfekty, wartosc);
		break;
	case WYCISZANIE:
		muzykaWyciszona = true;
		muzykaPrzedWyciszeniem = glosnoscPrawy;
		int nowaGlosnosc;
		nowaGlosnosc = glosnoscPrawy - poziomWyciszenia;
		if (nowaGlosnosc < 0) nowaGlosnosc = 0;
		AUDIO_INFO_V("[audio](grajEfekt) wyciszam muzyke do  : ", nowaGlosnosc);
///		playerPrawy.volume(nowaGlosnosc);	
		poleceniaPrawy->dodaj(10, nowaGlosnosc, 0);
///		playerLewy.playFolder(katEfekty, wartosc);
		poleceniaLewy->dodaj(20, katEfekty, wartosc);
		break;

	case PALZOWANIE:
		muzykaPauza = true;
///		playerPrawy.pause();
		poleceniaPrawy->dodaj(50, 0, 0);
///		playerLewy.playFolder(katEfekty, wartosc);
		poleceniaLewy->dodaj(20, katEfekty, wartosc);
		break;
	default:
		break;
	};
};


/**
* @brief Graj nagranie audio na podstawie indexu pliku wzgledem ca³ej karty SD
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
* @brief Graj utwór powitalny po w³¹czeniu robota. Jeden utwor znajduj¹cy siê w katalogu numer :05
* Polecenie KOLEJKOWANE
*/
void YtesAudio::grajMuzykePowitalna() {
	//playerPrawy.playFolder(5, 1);
///	playerPrawy.repeatFolder(5);
	poleceniaPrawy->dodaj(30, 5, 0); //repeatFolder 5
};

/**
* @brief Metoda musi byc ci¹gle wywolywana w glownej pêtli programu. W przypadku trybu pracy : PALZOWANIE, WYCISZANIE
* sprawdza czy efekt specjalny zakonczyl sie. Jesli tak przywraca poprzedni¹ g³oœnoœæ lub odpa³zowuje glowny utwor.
*/
void YtesAudio::audioHandler() {
	// obsluga polecen audio
	poleceniaPrawy->obsluzPolecenia();
	poleceniaLewy->obsluzPolecenia();

	// obs³uga na podstawie radaru
	if (radar != nullptr && millis() > msOstatniAutoEfekt && uwzglednijRadar == true) {
		float odleglosc = radar->dystans();
		if (odleglosc < _odlegloscRadarBlisko) {
			byte indexEfektu = losujZgrupy(RADAR_BLISKO);
			msOstatniAutoEfekt = millis() + msPrzerwaEfektyBaza + random(msPrzerwaEfektyLos);
			if (indexEfektu > 0) {
				grajEfekt(indexEfektu);
			};
		};
	};

	//obs³uga efektów dzwiêkowych na podstawie wskazan zyroskopu.
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
				indexEfektu = losujZgrupy(PRZECHYL_BOK_MALY);
				break;
			case STAN_ZYROSKOP::PRAWO_LEZE:
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
	};

	//Przywróæ poprzedni¹ g³oœnoœæ lub odpa³zuj utwór
	if (muzykaWyciszona || muzykaPauza) {
		if (portLewy->available()) {
			bool lewyGra = playerLewy.isPlaying();
			AUDIO_INFO_V("[audio] handler -> stan lewy :", lewyGra);
			if (muzykaWyciszona && !lewyGra) {
				AUDIO_INFO_V("[audio](wyciszenie) powracam do poziomu glosnosci : ", muzykaPrzedWyciszeniem);
///				playerPrawy.volume(muzykaPrzedWyciszeniem);
				poleceniaPrawy->dodaj(10, muzykaPrzedWyciszeniem, 0);
				muzykaWyciszona = false;
			};
			if (muzykaPauza && !lewyGra) {
				AUDIO_INFO("[audio] handler -> odpalzowanie");
///				playerPrawy.resume();
				poleceniaPrawy->dodaj(60, 0, 0); //resume
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
* Mozliwe wartoœci : NORMALNY , STEREO_MUZYKA ,STEREO_EFEKTY , WYCISZANIE , PALZOWANIE
*/
TRYB_AUDIO YtesAudio::wezTrybAudio() {
	return tryb;
};

/*
* @brief Ustaw g³oœnoœæ dla lewego lub prawgo g³oœnika. Metoda sprawdza poprawnosc zakresow 0..30
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
* @brief Obsluga obiektu opisuj¹cego audio. Obiekt mo¿e zawieraæ dane :
* {
*		"TOR" : 0..1	<- 0 - cenzura  , 1 - bez cenzury
*		"TRYB" : 0..4	<- NORMALNY , STEREO_MUZYKA ,STEREO_EFEKTY , WYCISZANIE , PALZOWANIE
*		"LG" : 25,		<- Lewy glosnosc (domyslnie muzyka)
*		"PG" : 15,		<- Prawy glosnosc (domyslnie efekty dzwiekowe)
*		"PW" : 7		<- Poziom wyciszenia
*		"LO" : 3		<- Lewy tor odtwarzanie(domyœlnie efekt)
*		"PO" : 3		<- Prawy tor odtwarzanie (domyœlnie muzyka)
* }
* W przypadku LO,PO - z których katalogów bêd¹  odtwarzane nagrania, decyduje "TOR"
*/
void YtesAudio::obslozPolecenieDane(JsonObject* dane) {
	boolean torZmieniony = false;

	JsonVariant vTor = (*dane)["TOR"];
	if (!vTor.isNull()) {
		int nTor = vTor.as<int>();
		if (nTor != tor) {
			ustawTor((TOR)vTor.as<int>());
			torZmieniony = false;
		};
	};
	//tryb : 
	JsonVariant vTryb = (*dane)["TRYB"];
	if (!vTryb.isNull()) {
		ustawTrybAudio((TRYB_AUDIO)vTryb.as<int>());
	}
	//lewy kanal glosnosc
	JsonVariant vLG = (*dane)["LG"];
	if (!vLG.isNull()) {
		int lg = vLG.as<int>();
		if (glosnoscLewy != lg) {
/// glosnosc(LEWY, lg);
			poleceniaLewy->dodaj(10, lg, 0);
			AUDIO_INFO_V("Glosnik lewy , glosnosc ustawiona :", lg);
		}
		else {
			AUDIO_INFO("Glosnik lewy , glosnosc bez zmian");
		};
	};
	//prawy kanal  glosnosc
	JsonVariant vPG = (*dane)["PG"];
	if (!vPG.isNull()) {
		int pg = vPG.as<int>();
		if (glosnoscPrawy != pg) {
/// glosnosc(PRAWY, pg);
			poleceniaPrawy->dodaj(10, pg, 0);
			AUDIO_INFO_V("Glosnik prawy , glosnosc ustawiona :", pg);
		}
		else {
			AUDIO_INFO("Glosnik prawy, glosnosc bez zmian");
		};
	};
	//poziom wyciszenia
	JsonVariant vPW = (*dane)["PW"];
	if (!vPW.isNull()) {
		ustawPoziomWyciszenia(vPW.as<int>());
	};
	//Odtwarzany kanal prawy (muzyka)
	JsonVariant vPO = (*dane)["PO"];
	if (!vPO.isNull()) {
		int po = vPO.as<int>();
		if (po != _PO) {
			_PO = po;
/// grajMuzyke(po);
			poleceniaPrawy->dodaj(20, katMuzyka, po);
		};
	};
	//Odtwarzany kanal lewy (efekty)
	JsonVariant vLO = (*dane)["LO"];
	if (!vLO.isNull()) {
		int lo = vLO.as<int>();
///grajEfekt(vLO.as<int>());
		poleceniaLewy->dodaj(20, katEfekty, lo);
	};
	//Uwzglednij zyroskop
	JsonVariant vUZ = (*dane)["UZ"];
	if (!vUZ.isNull()) {
		uwzglednijZyroskop = vUZ.as<boolean>();
	};
	//Uwzglednij radar
	JsonVariant vUR = (*dane)["UR"];
	if (!vUR.isNull()) {
		uwzglednijRadar = vUR.as<boolean>();
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