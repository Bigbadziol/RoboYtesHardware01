// YtesAudio.h

#ifndef _YTESAUDIO_h
#define _YTESAUDIO_h

#include "setup.h"
#include "DFPlayerMini_Fast.h" // DFPlayerMini_Fast by PowerBroker2  Version 1.2.4
							   // https://github.com/PowerBroker2/DFPlayerMini_Fast

using namespace dfplayer;

enum KANAL{LEWY,PRAWY};
enum TYP_AUDIO{MUZYKA,EFEKT};
enum TRYB_AUDIO{NORMALNY,STEREO_MUZYKA,STEREO_EFEKTY,WYCISZANIE,PALZOWANIE};

typedef struct sOstatniStan {
	bool gram;
	int indexGlobalny; //nie numer w katalogu , tylko na calej karcie
	int glosnosc;
}sOstatniStan_t;

//player lewy traktowany jest jako nadrzedny, w trybie normalnym gra muzyke
//normalny - osobne sterowanie playerem lewym , i playerem prawym
//stereo_muzyka - player prawy odgrywa z tego samego katalogu co nadrzedny(lewy) tylko muzyka, efekty sa ignorowane
//stereo_efekty - player prawy odgrywa z tego samego katalogu co nadrzedny(lewy) tylko efekty, muzyka jest ignorowana
//wyciszanie - podczas odgrywania efektu muzyka jest sciszana o okreslony poziom
//palzowanie - podczas odgrywania efektu muzyka jest pauzowana


class YtesAudio {
private:
	HardwareSerial* portLewy; //referencje do serialPortu
	HardwareSerial* portPrawy;
	const int defGlosnoscLewy = 20; //0..30
	const int defGlosnoscPrawy = 20;//0..30
	const int poziomWyciszenia = 10; //O tyle zostanie pomniejszona wartosc kanalu lewego na czas grania efektu

	const int defKatMuzyka = 1; //katalog dla glosnika lewego , dla cyfry 1, fizycznie musi miec postac "001" 
	const int defKatEfekty = 2;//katalog dla glosnika prawego, dla cyfry 2, fizycznie musi miec postac "002"
	static const byte iloscUtworow = 6; // musi sie faktycznie pokrywac z iloscia plikow w folderze oraz pozycjami 
	static const byte iloscEfektow = 8; // ..zdefiniowanymi w listach
	const char* listaUtworow[iloscUtworow] = {"Bodies",
											  "Here comes the booms",
											  "Numb",
											  "Fight back",
											  "One generation",
											  "Du hast"};
	const char* listaEfektow[iloscEfektow] = { "krzyk 1",
											   "krzyk 2",
											   "tortura",
											   "bol",
											   "straszny krzyk",
											   "pierd",
											   "ohhh",
											   "pila"};


	int glosnoscLewy = defGlosnoscLewy;
	int glosnoscPrawy = defGlosnoscPrawy;

	int _lewy_PrzedWyciszeniem = glosnoscLewy;

	sOstatniStan_t stanLewy;  //odpowiedz z mp3playera w callbacku
	sOstatniStan_t stanPrawy; //odpowiedz z mp3playera w callbacku

	DFPlayerMini_Fast playerLewy;
	DFPlayerMini_Fast playerPrawy;

	TRYB_AUDIO tryb;
public:
	YtesAudio(HardwareSerial *serialportLewego, HardwareSerial * serialportPrawego);
	~YtesAudio();	
	void ustawTrybAudio(TRYB_AUDIO nowyTryb);
	TRYB_AUDIO wezTrybAudio();

	void uaktualnijStanLewy();
	void uaktualnijStanPrawy();
	void uaktualnij();


	void grajEfekt(int  nrNagrania); 
	void graj(KANAL kanal, TYP_AUDIO typ,  int nrNagrania);
	void glosnosc(KANAL kanal, int wartosc);
	void obslozPolecenieDane(JsonObject* dane); //obsluga samego obiektu "dane"
	String odpowiedz();
	void dump(KANAL odtwarzaczSymbol); //L lub P
};

YtesAudio::YtesAudio(HardwareSerial* serialportLewego, HardwareSerial* serialportPrawego){
	//serialLewy, seriaPrawy - musza byc zainicjalizowane xxx.begin(...) globalnie
	portLewy = serialportLewego;
	portPrawy = serialportPrawego;
	if (!playerLewy.begin(serialLewy)) {  //Use softwareSerial to communicate with mp3.    
		Serial.println("player lewy - blad. (Brak karty lub brak komunikacji) ");
	}
	else {		
		playerLewy.setTimeout(500);
		playerLewy.volume(glosnoscLewy);  //0..30
		playerLewy.EQSelect(EQ_NORMAL);//EQ_NORMAL , EQ_POP , EQ_ROCK , EQ_JAZZ ,EQ_CLASSIC ,EQ_BASS
		playerLewy.playbackSource(TF); // U(u disc) = 1, TF = 2(sd card), AUX = 3, SLEEP = 4, FLASH = 5;
		stanLewy.glosnosc = glosnoscLewy;
		stanLewy.gram = false;
		stanLewy.indexGlobalny = 0;
		playerLewy.stop();
	};
	if (!playerPrawy.begin(serialPrawy)) {  //Use softwareSerial to communicate with mp3.    
		Serial.println("player prawy - blad. (Brak karty lub brak komunikacji) ");
	}
	else {
		playerPrawy.setTimeout(500);
		playerPrawy.volume(glosnoscPrawy);		
		playerPrawy.EQSelect(EQ_NORMAL);
		playerPrawy.playbackSource(TF);
		stanPrawy.glosnosc = glosnoscPrawy;
		stanPrawy.gram = false;
		stanPrawy.indexGlobalny = 0;
		playerPrawy.stop();
	};

}

YtesAudio::~YtesAudio() {
	playerLewy.stop();
	playerPrawy.stop();
	serialLewy.end();
	serialPrawy.end();
};

void YtesAudio::uaktualnijStanLewy() {
	stanLewy.gram = playerLewy.isPlaying();
	stanLewy.glosnosc = playerLewy.currentVolume();
	stanLewy.indexGlobalny = playerLewy.currentSdTrack();
	Serial.printf("Lewy gram : %d , glosnosc : %d  , index : %d , hardware : %d \n",
		stanLewy.gram, stanLewy.glosnosc, stanLewy.indexGlobalny,playerLewy.currentVersion());
}

void YtesAudio::uaktualnijStanPrawy() {
	stanPrawy.gram = playerPrawy.isPlaying();
	stanPrawy.glosnosc = playerPrawy.currentVolume();
	stanPrawy.indexGlobalny = playerPrawy.currentSdTrack();
	Serial.printf("Prawy gram : %d , glosnosc : %d , index : %d , hardware : %d \n",
		stanPrawy.gram, stanPrawy.glosnosc,stanPrawy.indexGlobalny,playerPrawy.currentVersion());
}

void YtesAudio::uaktualnij() {
	if (portLewy->available()) {
		uaktualnijStanLewy();
		playerLewy.printError();
		//portLewy->flush();
	}
	if (portPrawy->available()) {
		uaktualnijStanPrawy();
		playerPrawy.printError();
		//portLewy->flush();
	}
}
/*
* @brief Metoda w za³o¿eniu wyko¿ystywana przez samego robota. Efekt odgrywany na podniesienie, uderzenie , itp.
* Metoda modyfikuje swoje dzia³anie ze wzglêdu na ustawienie trybu audio.
* Podanie <0 - powoduje zatrzymanie grania efektu
*/

void YtesAudio::grajEfekt(int nrNagrania) {
	int wartosc = nrNagrania;
	if (wartosc < 0) wartosc = 0;
	if (wartosc > iloscEfektow) wartosc = 0;

	switch (tryb) {
	case NORMALNY:
		if (wartosc == 0) playerPrawy.stop();
		else playerPrawy.playFolder(defKatEfekty, wartosc);
		break;
	case STEREO_MUZYKA:
		//ignoruj muzyka jest priorytetem
		break;
	case STEREO_EFEKTY:
		if (wartosc == 0) {
			playerLewy.stop();
			playerPrawy.stop();
		}
		else {
			playerLewy.playFolder(defKatEfekty, wartosc);
			playerPrawy.playFolder(defKatEfekty, wartosc);
		}
		break;
	case WYCISZANIE:
		//_lewy_PrzedWyciszeniem =
		 //..kod odpowiadajacy za wyciszanie kanalu lewego- muzyka
		 // 
		 //..powrot do poprzedniej wartosci musi nastapic w callbacku
		break;
	case PALZOWANIE:
		//..kod resume w callbacku
		playerLewy.pause();
		playerPrawy.playFolder(defKatEfekty, wartosc);
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
			if (typ == MUZYKA) playerLewy.playFolder(defKatMuzyka, wartosc);
			else playerLewy.playFolder(defKatEfekty, wartosc);
		}
		AUDIO_INFO_V("Kanal lewy , nagranie nr :",  wartosc);
		break;
	case PRAWY:
		if (wartosc == 0) playerPrawy.stop();
		else {
			if (typ == MUZYKA) playerPrawy.playFolder(defKatMuzyka, wartosc);
			else playerPrawy.playFolder(defKatEfekty, wartosc);
		};
		AUDIO_INFO_V("Kanal prawy , nagranie nr :", wartosc);
		break;

	default:
		break;
	};
};

/**

*/
void YtesAudio::ustawTrybAudio(TRYB_AUDIO nowyTryb) {
	tryb = nowyTryb;
};
/**

*/
TRYB_AUDIO YtesAudio::wezTrybAudio() {
	return tryb;
}

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


	//restart playerow
	JsonVariant vRestart = (*dane)["RESET"];
	if (!vRestart.isNull()) {
		int v = vRestart.as<int>();
		if (v == 1) {
			playerLewy.reset();
			playerPrawy.reset();
			AUDIO_INFO("Glosniki prawy i lewy - reset");
		}
	}
}

/*
* Odpowiedz kontrolera jako string
*/
String YtesAudio::odpowiedz() {
	StaticJsonDocument<1024> doc;
	doc["cmd"] = "audioOdp";
	JsonObject objData = doc.createNestedObject("data");	
	//objData["LK"] = defKatMuzyka; //Rozwiazaniem bedzie podmiana kart :D
	//objData["PK"] = defKatEfekty; // Jak wyzej
	objData["LG"] = glosnoscLewy;
	objData["PG"] = glosnoscPrawy;

	JsonArray arrL = objData.createNestedArray("L");
	JsonArray arrP = objData.createNestedArray("P");
	
	for (int i = 0; i < iloscUtworow; i++) {
		arrL.add(listaUtworow[i]);
	};
	
	for (int i = 0; i <iloscEfektow; i++) {
		arrP.add(listaEfektow[i]);
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