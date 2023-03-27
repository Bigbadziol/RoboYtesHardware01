// YtesAudio.h

#ifndef _YTESAUDIO_h
#define _YTESAUDIO_h

#include "setup.h"
#include "DFPlayerMini_Fast.h" // DFPlayerMini_Fast by PowerBroker2  Version 1.2.4
							   // https://github.com/PowerBroker2/DFPlayerMini_Fast

using namespace dfplayer;

enum TOR {CENZURA, BEZ_CENZURY};
enum KANAL{LEWY,PRAWY};
enum TYP_AUDIO{MUZYKA,EFEKT};
enum TRYB_AUDIO{NORMALNY,STEREO_MUZYKA,STEREO_EFEKTY,WYCISZANIE,PALZOWANIE};


//TODO : WIELKA ZMIANA PRAWY-MUZA , LEWY EFEKTY
// DODATKOWO : OBOWIAZKOWO W LEWY (HW-247A)

//player prawy traktowany jest jako nadrzedny, w trybie normalnym gra muzyke
//normalny - osobne sterowanie playerem lewym , i playerem prawym
//stereo_muzyka - player prawy odgrywa z tego samego katalogu co nadrzedny(lewy) tylko muzyka, efekty sa ignorowane
//stereo_efekty - player prawy odgrywa z tego samego katalogu co nadrzedny(lewy) tylko efekty, muzyka jest ignorowana
//wyciszanie - podczas odgrywania efektu muzyka jest sciszana o okreslony poziom
//palzowanie - podczas odgrywania efektu muzyka jest pauzowana


//...Z wzglêdu na du¿e opoŸnienia w wykonywaniu poleceñ, dodatkowo ró¿ne czasy w zale¿noœci od scalaka
//pomys³em jest napisanie listy, która bêdzie przechowywa³a abstrakt polecenia
//audioHandler() bedzie sprawdza³ rozmiar listy , jeœli coœ jest to co okreœlony okres czasu wykona polecenie i
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
	//HardwareSerial* portLewy; //referencje do serialPortu
	//HardwareSerial* portPrawy;
	int polecenieOpoznienie = 350;
	SoftwareSerial* portLewy;
	SoftwareSerial* portPrawy;

	TOR defTor = CENZURA; //0..1 , 0-tor cenzura , 1-tor bez cenzury
	const int defGlosnoscLewy = 15; //0..30
	const int defGlosnoscPrawy = 15;//0..30
	const int defPoziomWyciszenia = 5;
	
	const int defKatMuzyka = 1; //katalog dla glosnika lewego , dla cyfry 1, fizycznie musi miec postac "001" 
	const int defKatEfekty = 2;//katalog dla glosnika prawego, dla cyfry 2, fizycznie musi miec postac "002"
	static const int iloscUtworow = 3; // musi sie faktycznie pokrywac z iloscia plikow w folderze oraz pozycjami 
	static const int iloscEfektow = 5; // ..zdefiniowanymi w listach

	const char* listaUtworowCenzura[iloscUtworow] = {		"Jazz 1",
															"Jazz 2",
															"Jazz 3"};


	const char* listaEfektowCenzura[iloscEfektow] = {		"witaj",
															"postaw mnie",
															"postaw mnie prosto",
															"stromo tutaj",
															"z gorki jest"};

	const char* listaUtworowBezCenzury[iloscUtworow] = { "Kung-fu Panda",
															"Deadpool - Bodies",
															"Fight back" };

	const char* listaEfektowBezCenzury[iloscEfektow] = {	"(b)witaj",
															"(b)postaw mnie",
															"(b)postaw mnie prosto",
															"(b)stromo tutaj",
															"(b)z gorki jest" };

	
	TOR tor = defTor;
	TRYB_AUDIO  tryb = NORMALNY;
	int glosnoscLewy = defGlosnoscLewy;
	int glosnoscPrawy = defGlosnoscPrawy;
	int poziomWyciszenia = defPoziomWyciszenia;
	int katMuzyka = defKatMuzyka;
	int katEfekty = defKatEfekty;

	int muzykaPrzedWyciszeniem = glosnoscLewy; // pomocnicza dla trybu : WYCISZANIE
	boolean muzykaPauza = false;
	boolean muzykaWyciszona = false; // pomocnicza dla trybu : WYCISZANIE


	void wyciszMuzyke();		//metoda pomocnicza do trybu WYCISZANIE
	void podglosnijMuzyke();	//metoda pomocnicza do trybu WYCISZANIE
	int indexDlaMuzyki(int numerNagrania);

public:
	DFPlayerMini_Fast playerLewy;
	DFPlayerMini_Fast playerPrawy;

	YtesAudio(SoftwareSerial* serialportLewego, SoftwareSerial* serialportPrawego,int czasOpoznieniaPolecenia);

	~YtesAudio();
	void ustawTrybAudio(TRYB_AUDIO nowyTryb);
	TRYB_AUDIO wezTrybAudio();

	void ustawTor(TOR nowyTor);

	TOR wezTor();
	void grajMuzyke(int nrNagrania);
	void grajEfekt(int  nrNagrania); 
	void graj(KANAL kanal, TYP_AUDIO typ,  int nrNagrania);
	void glosnosc(KANAL kanal, int wartosc);
	void obslozPolecenieDane(JsonObject* dane); //obsluga samego obiektu "dane"

	void audioHandler();
	String odpowiedz();
	void dump(KANAL odtwarzaczSymbol); //L lub P
};


YtesAudio::YtesAudio(SoftwareSerial* serialportLewego, SoftwareSerial* serialportPrawego, int czasOpoznieniaPolecenia) {
	polecenieOpoznienie = czasOpoznieniaPolecenia;
	AUDIO_INFO("[Audio konstruktor] -> LEWY przygotowanie");
	//LEWY
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
	Serial.println("[Audio konstruktor] -> PRAWY przygotowanie");
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
}

YtesAudio::~YtesAudio() {
	playerLewy.stop();
	playerPrawy.stop();
	serialLewy.end();
	serialPrawy.end();
};
/**
* @brief Wycisz muzyke
* 
*/
void YtesAudio::wyciszMuzyke() {
	muzykaPrzedWyciszeniem = glosnoscLewy;
	int nowaGlosnosc = muzykaPrzedWyciszeniem - poziomWyciszenia;
	if (nowaGlosnosc < 0) nowaGlosnosc = 0;
//	AUDIO_INFO_V("[audio] nowa glosnosc : ", nowaGlosnosc);
	playerPrawy.volume(nowaGlosnosc);													///
}

/**
* @brief Podg³oœnij muzyke
*/
void YtesAudio::podglosnijMuzyke() {
	AUDIO_INFO_V("[audio] powracam do poziomu glosnosci : ", muzykaPrzedWyciszeniem);
	playerPrawy.volume(muzykaPrzedWyciszeniem);
}

/**
* @brief Na podstawie toru audio : CENZURA, BEZ_CENZURY
* TODO : TO NIE DZIALA !! Player bardzo dziwnie liczy indexy utworow, nie pokrywa sie to z oficjalna dokumentacja !!
* Funkcja niech zostanie.
* oblicz fizyczny index nagrania.
* Docelowo wskazany w programie utwor bedzie zapetlany playerPrawy.loop();
*/
int YtesAudio::indexDlaMuzyki(int numerNagrania) {	
	int retIndex = 0;
	Serial.println("Co jest do chuja!");
	switch (tor) {
	case CENZURA:
		retIndex =  numerNagrania;
		break;
	case BEZ_CENZURY:
		Serial.println(iloscUtworow);
		Serial.println(iloscEfektow);
		Serial.println(numerNagrania);
		retIndex =  iloscUtworow + iloscEfektow + numerNagrania;		
		break;
	default:
		AUDIO_ERROR("Blad ![audio] -> ustawiono domyslny index dla zapetlanego utworu : 1");
		retIndex = 1;
		break;
	}
	AUDIO_INFO_V("Index dla utworu: ", retIndex);
	return retIndex;
}

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
	}
}
/**
* @brief Pobierz auktualne ustawienia toru
*/
TOR YtesAudio::wezTor() {
	return tor;
}

/**
* @brief Metoda z za³o¿enia ustawiona raz w configu. , Uwaga ! wartoœæ nrNagrania == 0 spowoduje odgrywanie 
* utworów po kolei
*/
void YtesAudio::grajMuzyke(int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > iloscUtworow) wartosc = 0;
	
	if (wartosc == 0) {
		AUDIO_INFO_V("Zapetlam katalog :", katMuzyka)
		playerPrawy.repeatFolder(katMuzyka);
		return;
	}

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
		 //zmianê g³oœnoœci tego kana³u powoduje metoda grajEfekt();
		 // powrot do poprzedniej wartosci zapewnia audioHandler();
 		playerPrawy.playFolder(katMuzyka, wartosc);
		//playerPrawy.loop(indexDlaMuzyki(nrNagrania));
		break;
	case PALZOWANIE:
		//zmianê g³oœnoœci tego kana³u powoduje metoda grajEfekt();
		//powrot do poprzedniej wartosci zapewnia audioHandler();
		playerPrawy.playFolder(katMuzyka, wartosc);
		//playerPrawy.loop(indexDlaMuzyki(nrNagrania));
		break;
	default:
		break;
	};

}

/*
* @brief Metoda w za³o¿eniu wyko¿ystywana przez samego robota. Efekt odgrywany na podniesienie, uderzenie , itp.
* Metoda modyfikuje swoje dzia³anie ze wzglêdu na ustawienie trybu audio.
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
}

/*
* Graj nagranie dla wskazanego kanalu(glosnika). Podanie < 0 spowoduje zatrzymanie nagrania dla 
* danego kana³u. Przekroczenie zakresow 0..255 , ustawi wartosc na 0
* 
* Nadrzêdny bedzie tryb przykladowo GrajMuzyke(jesli  normalny  to tylko lewy glosnik , jesli stereo_muzyka to oba glosniki,
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
* @brief Metoda musi byc ci¹gle wywolywana w glownej pêtli programu. W przypadku trybu pracy : PALZOWANIE, WYCISZANIE
* sprawdza czy efekt specjalny zakonczyl sie. Jesli tak przywraca poprzedni¹ g³oœnoœæ lub odpa³zowuje glowny utwor.
*/
void YtesAudio::audioHandler() {
	if (muzykaWyciszona || muzykaPauza) {
//		AUDIO_INFO("[audio] handler -> Muzyka wyciszona lub pauza");
		if (portLewy->available()) {
			bool lewyGra = playerLewy.isPlaying();
//			AUDIO_INFO_V("[audio] handler -> stan lewy :", lewyGra);
			if (muzykaWyciszona && !lewyGra) {
				playerPrawy.volume(muzykaPrzedWyciszeniem);

				//podglosnijMuzyke(); //metoda sama z siebie generuje informacje 
				muzykaWyciszona = false;
			}
			if (muzykaPauza && !lewyGra){
//				AUDIO_INFO("[audio] handler -> odpalzowanie");
				playerPrawy.resume();
				muzykaPauza = false;
			}
		}
	}
}
/**
* @brief Zmien tryb audio na : NORMALNY , STEREO_MUZYKA ,STEREO_EFEKTY , WYCISZANIE , PALZOWANIE
*/
void YtesAudio::ustawTrybAudio(TRYB_AUDIO nowyTryb) {
	tryb = nowyTryb;
};
/**
* @brief Metoda zwraca aktualny ustawiony tryb audio. Mozliwe wartoœci : NORMALNY , STEREO_MUZYKA ,STEREO_EFEKTY , WYCISZANIE , PALZOWANIE
*/
TRYB_AUDIO YtesAudio::wezTrybAudio() {
	return tryb;
};

/*
* Ustaw g³oœnoœæ dla lewego lub prawgo g³oœnika. Metoda sprawdza poprawnosc zakresow 0..30
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
* Obsluga samego bloku dane, zatem przykladowy spodziewany obiekt ma mieæ postaæ :
* {
		"LG" : 25,		<- Lewy glosnosc (domyslnie muzyka)
		"PG" : 15,		<- Prawy glosnosc (domyslnie efekty dzwiekowe)
		"LN" : 3		<- Lewy nagranie (numer nagrania z wczesniej wskazanego katalogu)
* }
*/
void YtesAudio::obslozPolecenieDane(JsonObject* dane) {
	//const char* vLG = (*dane)["KLUCZ"].as<const char*>(); //lyka
	//int noweDane = (*dane)["LG"].as<int>();//lyka
	//if (noweDane) {}
	
	//lewy glosnosc
	AUDIO_INFO("AUDIO -  blok dane");
	JsonVariant vLG = (*dane)["LG"];
	if (!vLG.isNull()) {
		glosnosc(LEWY, vLG.as<int>());
		AUDIO_INFO_V("Glosnik lewy glosnosc ustawiona :", vLG.as<int>());
	}
	
	//prawy glosnosc
	JsonVariant vPG = (*dane)["PG"];
	if (!vPG.isNull()) {
		glosnosc(PRAWY, vPG.as<int>());
		AUDIO_INFO_V("Glosnik prawy glosnosc ustawiona :", vPG.as<int>());
	};

}

/*
* Odpowiedz kontrolera jako string
*/
String YtesAudio::odpowiedz() {
	StaticJsonDocument<1024> doc;
	JsonObject objData = doc.createNestedObject("data");	
	objData["TOR"] = (byte)tor;
	objData["TRYB"] = (byte)tryb;
	objData["LG"] = glosnoscLewy;
	objData["PG"] = glosnoscPrawy;
	objData["PW"] = poziomWyciszenia;

	JsonArray arrL1 = objData.createNestedArray("L1");
	JsonArray arrL2 = objData.createNestedArray("L2");
	
	for (int i = 0; i < iloscUtworow; i++) {
		arrL1.add(listaUtworowCenzura[i]);
	};
	
	for (int i = 0; i <iloscEfektow; i++) {
		arrL2.add(listaEfektowBezCenzury[i]);
	};	

	String tmp="";
	size_t resSize = serializeJson(doc, tmp);
	return tmp;
}

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