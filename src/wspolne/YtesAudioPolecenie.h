#pragma once
#include "setup.h"
#include "DFPlayerMini_Fast.h" // DFPlayerMini_Fast by PowerBroker2  Version 1.2.4
/*
	Z wzglêdu na du¿e opoŸnienia w wykonywaniu poleceñ, dodatkowo ró¿ne czasy w zale¿noœci od scalaka
	pomys³em jest napisanie listy, która bêdzie przechowywa³a abstrakt polecenia.
	audioHandler() bedzie sprawdza³ rozmiar listy poleceñ , jeœli coœ jest to co okreœlony okres czasu wykona polecenie i
	usunie je z listy.
	Problem wystêpuje kiedy do tego samego urz¹dzenia w bardzo krótkim czasie trafi wiêcej ni¿ 1 polecenie (przyk³adowo : nowe
	ustawienia konfiguracyjne z apki)
*/
typedef struct sAudioPolecenie {
	int cmd;	//Znaczenie:
	//10-zmiana glosnosci ,param1 : 1..30 - nowa glosnosc -> player.volume(param1);
	//20-utwor z katalogu ,param1 : 1..6(katalog),  param2 : 1..n (numer utworu w kat.) -> player.playFolder(param1,param2);
	//30-zapetlij katalog, param1 : 1..6(katalog) -> player.repeatFolder(param1);
	//40-zapetlij po indexie sd nagranie : param 1..n ->player.loop(param1)
	//50-palzuj , param1 , param2 - bez znaczenia , dla pozadku 0 -> player.pause();
	//60-wznow , param1, param2 - bez znaczenia , dla pozadku 0 -> player.resume();
	//70-stop , param1 , param2 - bez znaczenia , dla pozadku 0 -> player.stop();

	int param1;
	int param2;
	struct sAudioPolecenie* next = NULL;
}sAudioPolecenie_t;


class YtesAudioPolecenie {
private:
	unsigned long msCzekaj = 500L;	//przerwa pomiêdzy wykonaniem kolejnych poleceñ
	unsigned long msOstatnioWykonane = millis();

	int _size = 0;
	struct sAudioPolecenie* error = NULL;
	struct sAudioPolecenie* head = NULL;
	struct sAudioPolecenie* current = NULL;
	DFPlayerMini_Fast* player = NULL;
	String identyfikator = "";

public:
	String polecenieId = "";
	int wartoscParametru = 0;
	YtesAudioPolecenie(DFPlayerMini_Fast* pPlayer, String identyfikatorUrzadzenia, int opoznieniePolecen);
	~YtesAudioPolecenie();

	boolean dodaj(int cmd, int param1, int param2);
	boolean usun(int index);
	sAudioPolecenie_t pobierz(int index);
	int rozmiar();
	void pokaz();
	void obsluzPierwsze();
	void obsluzPolecenia();
};

YtesAudioPolecenie::YtesAudioPolecenie(DFPlayerMini_Fast* pPlayer, String identyfikatorUrzadzenia,  int opoznieniePolecen) {
	error = (sAudioPolecenie*)malloc(sizeof(sAudioPolecenie_t));
	error->cmd = -1;
	error->param1 = 0;
	error->param2 = 0;
	error->next = NULL;
	player = pPlayer;
	identyfikator = identyfikatorUrzadzenia;
	msCzekaj = opoznieniePolecen;
};

/**
* @brief Usuñ wszystkie polecenia z listy, usun wirtualne polecenie 'error'
*/
YtesAudioPolecenie::~YtesAudioPolecenie() {
	free(error);
	//something like "free tail"
	while ((current = head) != NULL) {
		head = head->next;
		free(current);
	};
}

/**
* @brief Dodaj abstrakt polecenia do listy
*/
boolean YtesAudioPolecenie::dodaj(int cmd, int param1, int param2) {
	if (head == NULL) {
		head = (sAudioPolecenie_t*)malloc(sizeof(sAudioPolecenie_t));
		if (head) {
			head->cmd = cmd;
			head->param1 = param1;
			head->param2 = param2;
			head->next = NULL;
			_size++;
			return true;
		}
		else {
			return false;
		};
	}
	else {
		current = head;
		while (current->next != NULL) {
			current = current->next;
		};
		current->next = (sAudioPolecenie_t*)malloc(sizeof(sAudioPolecenie_t));
		if (current->next) {
			current->next->cmd = cmd;
			current->next->param1 = param1;
			current->next->param2 = param2;
			current->next->next = NULL;
			_size++;
			return true;
		}
		else {
			return false;
		};
	};
	return true;
};

/**
* @brief Usun polecenie z listy o wskazanym indeksie.
*/
boolean YtesAudioPolecenie::usun(int index) {
	if (head == NULL) return false;
	if (index > -1 && index < _size) {
		if (index == 0) { //head;
			sAudioPolecenie_t* next_cmd = NULL;
			next_cmd = head->next;
			free(head);
			head = next_cmd;
			_size--;
			return true;
		}
		else {
			current = head;
			sAudioPolecenie_t* prev_cmd = NULL;
			for (int i = 0; i < index - 1; i++)  current = current->next; //stop one before
			prev_cmd = current;
			current = current->next;
			prev_cmd->next = current->next;
			free(current);
			_size--;
			return true;
		};
	};
	return false;
};

/**
* @brief Pobierz abstrakt polecenia na podstawie indexu z listy
*/
sAudioPolecenie_t YtesAudioPolecenie::pobierz(int index) {
	current = head;
	if (index > -1 && index < _size) {
		for (int i = 0; i < index; i++)  current = current->next;
		return *current;
	};
	//error not found
	return *error;
};

/**
* @brief Zwróæ iloœæ poleceñ czekaj¹cych w kolejce.
*/
int YtesAudioPolecenie::rozmiar() {
	return _size;
};

/**
* @brief Funkcja debugowa , wyœwietl kolejkê poleceñ do wykonania.
*/
void YtesAudioPolecenie::pokaz() {
#if SERIAL_PD == 1
	current = head;
	int index = 0;
	if (current == NULL) {
		Serial.print("[ListaPolecen] -> pusta , ");
		Serial.println(identyfikator.c_str());
		return;
	}
	else {
		Serial.print("ListaPolecen] :");
		Serial.println(identyfikator.c_str());
		Serial.println("[index][cmd][param1][param2] ");
		while (current != NULL) {
			Serial.printf("%d)  %d\t%d\t%d \n", index, current->cmd, current->param1, current->param2);
			index++;
			current = current->next;
		};
		Serial.println();
	};
#endif
};

/**
* @brief Na podstwie parametru cmd (10 - volume , 20 - playFolder , 30 - repeatFolder, 40-loop , 50-pause , 60-resume)
* wykonywane jest najstarsze polecenie. Po wykonaniu polecenie zostanie usuniête z listy.
*/
void YtesAudioPolecenie::obsluzPierwsze() {
	if (_size == 0) return;
	sAudioPolecenie_t polecenie = pobierz(0);
	if (polecenie.cmd != -1) { //error
		switch (polecenie.cmd) {
		case 10:
			PD_INFO_VV("[ListaPolecen](obsluz ostatnie) -> volume : ", (uint8_t)polecenie.param1, identyfikator.c_str());
			player->volume((uint8_t)polecenie.param1);
			break;
		case 20:
			PD_INFO_S("[ListaPolecen](obsluz ostatnie) -> play folder : ",identyfikator);
			PD_INFO_VV("[ListaPolecen](obsluz ostatnie) -> parametry : ", (uint8_t)polecenie.param1, (uint8_t)polecenie.param2);
			player->playFolder((uint8_t)polecenie.param1, (uint8_t)polecenie.param2);
			break;
		case 30:
			PD_INFO_VV("[ListaPolecen](obsluz ostatnie) -> repeat folder : ", polecenie.param1,identyfikator.c_str());
			player->repeatFolder(polecenie.param1);
			break;
		case 40:			
			PD_INFO_VV("[ListaPolecen](obsluz ostatnie) -> loop : ", polecenie.param1, identyfikator.c_str());
			player->loop(polecenie.param1);
			break;
		case 50:
			PD_INFO_S("[ListaPolecen](obsluz ostatnie) -> pause , ", identyfikator);
			player->pause();
			break;
		case 60:
			PD_INFO_S("[ListaPolecen](obsluz ostatnie) -> resume , ", identyfikator);
			player->resume();
			break;
		case 70:
			PD_INFO_S("[ListaPolecen](obsluz ostatnie) -> stop , ", identyfikator);
			player->stop();
			break;
		default:
			PD_INFO_S("[ListaPolecen](obsluz ostatnie) -> nieznane polecenie , ",identyfikator);
			PD_INFO_VV("[ListaPolecen](obsluz ostatnie) -> nieznane polecenie, param1 : ", polecenie.cmd, (uint8_t)polecenie.param1);
			break;
		};
		usun(0);
	};
};

/**
* @brief Metoda to cyklicznego (msCzekaj) sprawdzania poleceñ dla wskazanego playera. Wykonywane jest najstarsze polecenie,
* nastêpnie jest ono usuwane z listy.
*/
void YtesAudioPolecenie::obsluzPolecenia() {
	if (millis() > msOstatnioWykonane) {
		msOstatnioWykonane = millis() + msCzekaj;
		obsluzPierwsze();
	};	
};
