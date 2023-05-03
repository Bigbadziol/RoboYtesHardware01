#ifndef _YTESZYROSKOP_h
#define _YTESZYROSKOP_h

#include "setup.h"
#include <MPU6050_light.h>

enum STAN_ZYROSKOP {
	BRAK,
	LEWO_1,
	LEWO_2,
	PRAWO_1,
	PRAWO_2,
	PRZOD_1,
	PRZOD_2,
	TYL_1,
	TYL_2,
	WSTRZAS,
	RUCH
//	PODNOSZENIE,
//	PODRZUCANIE,
//	OPADANIE,
//	SPADANIE,
};

typedef struct pomiar{
	float oX = 0.0f; //k¹t obrotu wzglêdem osi X, tu lewo/prawo
	float oY = 0.0f; //k¹t obrotu wzglêdem osi Y, tu przod /tyl
	float oZ = 0.0f; //(uwaga obrot jak nos w samolocie- skrêcanie)
	float pX = 0.0f; // przyspieszenie lewo/prawo
	float pY = 0.0f; // przyspieszenie przod/tyl
	float pZ = 0.0f; // przyspieszenie gora/dol
}pomiar_t; //bedzie 24 bajty



//Zastanowiæ siê czy aby napewno potrzebujê wszystkie obliczenia ? (+) oznaczenie co uzywam.
typedef struct obliczenia {
	float saOX = 0.0f;	// + srednia arytmetyczna os X
	float saOY = 0.0f;  // + ...
	float saOZ = 0.0f;  // - ...
	float saPX = 0.0f;	// - srednia arytmetyczna przyspieszenie X
	float saPY = 0.0f;  // -
	float saPZ = 0.0f;  // +
}obliczenia_t;


class YtesZyroskop {
	private:	
		const float X_LEWO_1 = 22.5f; // (+)wartoœæ wyrazona w stopniach
		const float X_LEWO_2 = 45.0f; //
		const float X_PRAWO_1 = -22.5f; //
		const float X_PRAWO_2 = -45.0f; //
		const float Y_PRZOD_1 = 15.0f; // (+) ale tu stromo (pod gore)
		const float Y_PRZOD_2 = 30.0f; // wywale sie na plecy
		const float Y_TYL_1 = -15.0f;// (-) boje sie zjechac
		const float Y_TYL_2 = -30.0f; // polece na morde
/*
		const float Z_GORA_PODNOSZENIE = 1.02f; // Na tym etapie totalna klêska.
		const float Z_GORA_PODRZUCANIE = 1.05f; // Jeœli obiekt nie porusza siê w osi X lub Y tylko gora/dol
		const float Z_DOL_OPADANIE = 0.98f;     // da siê to jakoœ okreœliæ. W przypadku ruchu lub wstrz¹sów ,uderzeñ
		const float Z_DOL_SPADANIE = 0.94f;     // generowanych  jest zbyt du¿o fa³szywych stanow
*/
		static const int ILOSC_POMIAROW = 50;
		pomiar pomiary[ILOSC_POMIAROW];

		MPU6050* myMpu;
		STAN_ZYROSKOP poprzedniStan = STAN_ZYROSKOP::BRAK; //zmienna aktualizowana, lecz narazie do nieczego nie uzywana
		STAN_ZYROSKOP mojStan = STAN_ZYROSKOP::BRAK;
		bool status = false;
		unsigned long msPomiarInterwal = 0;
		unsigned long msOstatniOdczyt = millis();
		float _Zmin = 1; //testwa do poszukiwania min,max przyspieszenia w osi Z
		float _Zmax = 1; //j.w
		pomiar ostatniPomiar;
		obliczenia obliczeniaPomocnicze; //Popracowaæ nad tym polowa pol nie u¿ywana
		void przesunPomiary();
		void przesunPomiary(int dolnyProgPrzesuniecia);
		float minPole(int ilePomiarow, float pomiar_t::* pole);
		float maxPole(int ilePomiarow, float pomiar_t::* pole);
		float sredniaArytmetyczna(int ilePomiarow, float pomiar_t::* pole);
		float odchylenieStandartowe(int ilePomiarow, float pomiar_t::* pole);
		void aktualizujOblSredAryt();
		String stanToString(STAN_ZYROSKOP stan);
		void okreslStan();
	public:
		YtesZyroskop(MPU6050* m, int interwalPomiaru); //jesli podamy zero uaktualniaj w kazdym przebiegu pêtli , 1000(ms) co sekunde
		void autoKalibracja();
		void wbudowanaKalibracja();
		void uaktualnijOdczyty(); //metoda do pêli g³ównej
		void wypiszStan(bool ignorujStanBrak = true);
		STAN_ZYROSKOP pobierzStan();
		bool pobierzStatus(); //czy udalo sie zainicjalizowac MPU6050
		void wypiszKluczowePomiary();
		//Debug testy
		void wypiszRuchWstrzas();
		float losowePrzyspieszenie(float progDol, float prog);
		void wypiszUstawieniaKalibracji();
		void wypiszPomiary(int ileWypisac); // 0- wypisz wszystkie
		void wypiszPrzyspieszenieZ();
		void wypiszSredniaArr_oXYGZ();
		void wypiszWszystkieOdczyty(); // funkcja do debugu
};

//------------KONSTRUKTOR , DESTRUKTOR, PRYWATNE ---------------------------------------

/**
* Konstruktor
* Interwa³ wyra¿ony w ms.
* Interwal ustawiony na 0 - uaktualniaj odczyty co przebieg g³ownej pêtli.
* Wartoœæ wiêksza okreœla aktualizacjê o okreœlony czas.
* Domyœlnie 100ms, intensywnoœæ dokonywania pomiarów istotnie wp³ywa na okreœlenie stanu ¿yskopu
*/
YtesZyroskop::YtesZyroskop(MPU6050* m, int interwalPomiaru) {
	msPomiarInterwal = interwalPomiaru;
	myMpu = m;
	/*
	* Po ludzku : mniejsze wartoœci parametrów to wiêksza dok³adnoœæ ale w mniejszym zakresie
	* Mapowanie
	* GYRO_CONFIG_[0, 1, 2, 3] range = +-[250, 500, 1000, 2000] deg / s
	* sensi = [131, 65.5, 32.8, 16.4] bit / (deg / s)
	*
	* ACC_CONFIG_[0, 1, 2, 3] range = +-[2, 4, 8, 16] times the gravity(9.81 m / s ^ 2)
	* sensi = [16384, 8192, 4096, 2048] bit / gravity
	*/
	byte s = myMpu->begin(1, 0); //1,0 domyslne ustawienie
	if (s == 0) status = true;
	autoKalibracja();
};

/**
* @biref Przesówa pomiary w dó³ do okreslonego poziomu. Pierwszy wiersz przygotowany na
* nowy pomiar. Nie rusza pomiarów ponizej progu przesuniecia.
*/
void YtesZyroskop::przesunPomiary(int dolnyProgPrzesuniecia) {
	pomiar_t tmp;
	for (int i = dolnyProgPrzesuniecia - 1; i > 0; i--) {
		memcpy(&tmp, &pomiary[i - 1], sizeof(pomiar_t));
		memcpy(&pomiary[i], &tmp, sizeof(pomiar_t));
	}
	// zerujemy pierwszy element, ¿eby zachowaæ ci¹g³oœæ danych
	memset(&pomiary[0], 0, sizeof(pomiar_t));
}

/**
* @brief Zrob miejsce dla nowego pomiaru. Zawartoœæ tablicy przesun o 1 wiersz w dó³
*/
void YtesZyroskop::przesunPomiary() {
	int rozmiar = sizeof(pomiary) / sizeof(pomiary[0]);
	if (rozmiar == 0) {
		return;
	}
	memmove(&pomiary[1], &pomiary[0], sizeof(pomiary) - sizeof(pomiary[0]));
	memset(&pomiary[0], 0, sizeof(pomiar_t));
}

/**
* @brief Znajdz minimaln¹ wartoœæ dla wskazanego parametru, spraj¹c
* od pocz¹tku tablicy okreœlon¹ iloœæ wartoœci.
*/
float YtesZyroskop::minPole(int ilePomiarow, float pomiar_t::* pole) {
	int ile = ilePomiarow;
	if (ile > ILOSC_POMIAROW) ile = ILOSC_POMIAROW;
	float minimum = pomiary[0].*pole;
	for (int i = 0; i < ile; i++) {
		if (pomiary[i].*pole < minimum) minimum = pomiary[i].*pole;
	};
	return minimum;
};

/**
* @brief Znajdz maksymaln¹ wartoœæ dla wskazanego parametru, spraj¹c
* od pocz¹tku tablicy okreœlon¹ iloœæ wartoœci.
*/
float YtesZyroskop::maxPole(int ilePomiarow, float pomiar_t::* pole) {
	int ile = ilePomiarow;
	if (ile > ILOSC_POMIAROW) ile = ILOSC_POMIAROW;

	float maximum = pomiary[0].*pole;
	for (int i = 0; i < ile; i++) {
		if (pomiary[i].*pole > maximum) maximum = pomiary[i].*pole;
	};
	return maximum;
};

/**
* Oblicz œredni¹ arytmetyczn¹ dla wskazanego pola. Œredni¹ policz dla pierwszych n pozycji.
*/
float YtesZyroskop::sredniaArytmetyczna(int ilePomiarow, float pomiar_t::* pole) {
	int ile = ilePomiarow;
	if (ile > ILOSC_POMIAROW) ile = ILOSC_POMIAROW;

	float srednia = 0.0f;
	for (int i = 0; i < ile; i++) {
		srednia += pomiary[i].*pole;
	}
	return srednia /= ile;
};

/**
* Oblicz odchylenie standardowe dla wskazanego pola.Odchylenie policz dla pierwszych n pozycji.
*/
float YtesZyroskop::odchylenieStandartowe(int ilePomiarow, float pomiar_t::* pole) {
	float srednia = 0.0f;
	float sredniaKwadratowRoznic = 0.0f;
	int ile = ilePomiarow;
	if (ile > ILOSC_POMIAROW) ile = ILOSC_POMIAROW;

	for (int i = 0; i < ile; i++) {
		srednia += pomiary[i].*pole;
	};
	srednia /= ile;
	// Obliczenie œredniej arytmetycznej kwadratów ró¿nic dla wskazanego pola
	for (int i = 0; i < ile; i++) {
		float roznica = pomiary[i].*pole - srednia;
		sredniaKwadratowRoznic += roznica * roznica;
	};
	sredniaKwadratowRoznic /= ile;
	// Obliczenie pierwiastka kwadratowego z œredniej arytmetycznej kwadratów ró¿nic
	float odchylenieStandardowe = sqrt(sredniaKwadratowRoznic);
	return odchylenieStandardowe;
};

/**
* Aktualizuj najistotniejsze obliczenia na ten moment;
* srednia oX , oY i  Pz
*/
void YtesZyroskop::aktualizujOblSredAryt() {
	obliczeniaPomocnicze.saOX = sredniaArytmetyczna(ILOSC_POMIAROW, &pomiar_t::oX);
	obliczeniaPomocnicze.saOY = sredniaArytmetyczna(ILOSC_POMIAROW, &pomiar_t::oY);
	obliczeniaPomocnicze.saPZ = sredniaArytmetyczna(10, &pomiar_t::pZ);
};

/**
* @brief Przet³umacz systemowy stan na String
*/
String YtesZyroskop::stanToString(STAN_ZYROSKOP stan) {
	switch (stan) {
	case STAN_ZYROSKOP::BRAK: return "brak";
	case STAN_ZYROSKOP::LEWO_1:return "lewo 1";
	case STAN_ZYROSKOP::LEWO_2:return "lewo 2";
	case STAN_ZYROSKOP::PRAWO_1:return "prawo 1";
	case STAN_ZYROSKOP::PRAWO_2:return "prawo 2";
	case STAN_ZYROSKOP::PRZOD_1:return "przod 1";
	case STAN_ZYROSKOP::PRZOD_2:return "przod 2";
	case STAN_ZYROSKOP::TYL_1:return "tyl 1";
	case STAN_ZYROSKOP::TYL_2:return "tyl 2";
	case STAN_ZYROSKOP::WSTRZAS:return "wstrzas";
	case STAN_ZYROSKOP::RUCH:return "ruch";
		//	case STAN_ZYROSKOP::PODNOSZENIE: return "podnoszenie";
		//	case STAN_ZYROSKOP::PODRZUCANIE:return "podrzucanie";
		//	case STAN_ZYROSKOP::OPADANIE:return "opadanie";
		//	case STAN_ZYROSKOP::SPADANIE:return "spadanie";
	default: return "BLAD";
	};
	return "BLAD";
}

/**
* @brief Okresl stan pojazdu na podstawie wskazan zyroskopu.
* Wynik przypisz do zmiennej prywatnej mojStan;
*/
void YtesZyroskop::okreslStan() {
	//obliczeniaPomocnicze.saOX, obliczeniaPomocnicze.saOY, obliczeniaPomocnicze.saPZ,
	float ox = obliczeniaPomocnicze.saOX;
	float oy = obliczeniaPomocnicze.saOY;
	float oz = obliczeniaPomocnicze.saPZ; //pamietac ,ze nie liczona dla calej tablicy a jej zakresu

#if SERIAL_ZYR_INFO == 1
	//Serial.printf("%.2f\t %.2f\t %.2f \n",ox,oy,oz);
#endif
	//lewo - prawo
	if (ox >= X_LEWO_1 && ox < X_LEWO_2) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::LEWO_1;
		return;
	}
	else if (ox >= X_LEWO_2) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::LEWO_2;
		return;
	}
	else if (ox <= X_PRAWO_1 && ox > X_PRAWO_2) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::PRAWO_1;
		return;
	}
	else if (ox <= X_PRAWO_2) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::PRAWO_2;
		return;
	};
	//przod - tyl
	if (oy >= Y_PRZOD_1 && oy < Y_PRZOD_2) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::PRZOD_1;
		return;
	}
	else if (oy >= Y_PRZOD_2) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::PRZOD_2;
		return;
	}
	else if (oy <= Y_TYL_1 && oy > Y_TYL_2) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::TYL_1;
		return;
	}
	else if (oy <= Y_TYL_2) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::TYL_2;
		return;
	};


	//oszukiwanie wstrzasem
	float odychPX = odchylenieStandartowe(20, &pomiar_t::pX);
	float odychPY = odchylenieStandartowe(20, &pomiar_t::pY);
	float t_odch = sqrt(pow(odychPX, 2) + pow(odychPY, 2));
	//0.13 0.21 0.32 (DRGANIA szturchanie generalnie wstrzas)
	if (t_odch > 0.31) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::WSTRZAS;
		return;
	};
	 if (t_odch >= 0.05) {
		poprzedniStan = mojStan;
		mojStan = STAN_ZYROSKOP::RUCH;
		return;
	};

	/*
		//wkluczanie
	//1)
		if (oz >= Z_GORA_PODNOSZENIE && oz < Z_GORA_PODRZUCANIE && prog1 > 3) {
			mojStan = STAN_ZYROSKOP::BRAK;
			return;
		};

		// spadanie - podnoszenie, koniecznosc dolozenia korekty
		if (oz >= Z_GORA_PODNOSZENIE && oz < Z_GORA_PODRZUCANIE ) {
			mojStan = STAN_ZYROSKOP::PODNOSZENIE;
			return;
		}
		else if (oz >= Z_GORA_PODRZUCANIE) {
			 mojStan = STAN_ZYROSKOP::PODRZUCANIE;
			return;
		}
		else if (oz <= Z_DOL_OPADANIE && oz > Z_DOL_SPADANIE ) {
			if (ox > 5 || ox < -5) {
				mojStan = STAN_ZYROSKOP::BRAK;
				return;
			};
			mojStan = STAN_ZYROSKOP::OPADANIE;
			return;
		}
		else if (oz <= Z_DOL_SPADANIE) {
			if (ox > 5 || ox < -5) {
				mojStan = STAN_ZYROSKOP::BRAK;
				return;
			};
			mojStan = STAN_ZYROSKOP::SPADANIE;
			return;
		};
	*/
	mojStan = STAN_ZYROSKOP::BRAK;
};

//---------------------- PUBLICZNE -----------------------------------------------------
/**
* @brief Rozpocznij proces automatycznej kalibracji.
* Uwaga metoda automatycznie wywo³ywana przez konstruktor.
* Podczas kalibracji urz¹dzenie  musi pozostaæ w kompletnym bezruchu.
*/
void YtesZyroskop::autoKalibracja() {
	ZYR_INFO("[zyroskop] autokalibracja rozpoczeta.");
	myMpu->calcOffsets(true, true);
	ZYR_INFO("[zyroskop] autokalibracja zakonczona.");
};

/**
* @brief Nadaj rêcznie parametry koryguj¹ce pozycjê ¿yroskopu i offsety akcelerometra.
*/
void YtesZyroskop::wbudowanaKalibracja() {
	ZYR_INFO("[zyroskop] kalibracja reczna.");
	myMpu->setGyroOffsets(-0.587786, 0.529863, -0.442504);
	myMpu->setAccOffsets(0.003646, -0.014142, 0.004524);
};

/**
* Pobierz dane z mpu i przypisz zmiennej prywatnej ostatniPomiar. Uaktualnij tablicê pomiarów
* przez usuniêcie najstarszego pomiaru oraz dodanie najnowszego. Dokonaj wspólnych obliczeñ
* dla ró¿nych metod okreœlaj¹cych stan pojazdu.
* Na koniec okreœl logiczny stan pojazdu.
*/
void YtesZyroskop::uaktualnijOdczyty() {
	//jesli  obieg petli lub uplynal  okreslony czasl
	//UWAGA ZMIANA!!
	if ((msPomiarInterwal == 0) || (millis() > msOstatniOdczyt)) {
		myMpu->update();
		ostatniPomiar.oX = myMpu->getAngleX();
		ostatniPomiar.oY = myMpu->getAngleY();
		ostatniPomiar.oZ = myMpu->getAngleZ();
		ostatniPomiar.pX = myMpu->getAccX();
		ostatniPomiar.pY = myMpu->getAccY();
		ostatniPomiar.pZ = myMpu->getAccZ();
		przesunPomiary(); //czyli  usun ostatni,wszystko w dó³, zrób miejsce dla pierwszego.
		memcpy(&pomiary[0], &ostatniPomiar, sizeof(pomiar_t));
		aktualizujOblSredAryt();
		msOstatniOdczyt = millis() + msPomiarInterwal;
		//wypiszSredniaArr_oXYGZ();		 
		//wypiszRuchWstrzas();
		okreslStan();		
	};
};

/**
* @brief Wypisz aktualny stan(pozycjê) urz¹dzenia.
*/
void YtesZyroskop::wypiszStan(bool ignorujStanBrak) {
	if (ignorujStanBrak) {
		if (mojStan == BRAK) return;
	};
	ZYR_INFO_V("", stanToString(mojStan));
};

/**
* @brief Zwróæ stan innej klasie.
*/
STAN_ZYROSKOP YtesZyroskop::pobierzStan() {
	return mojStan;
};

/**
* @brief Zwróæ informacjê czy uda³o siê poprawnie zainicjowaæ MPU6050
* False oznacza brak mo¿liwoœci ko¿ystania z klasy
*/
bool YtesZyroskop::pobierzStatus() {
	return status;
};
/**
* @brief Wypisz najwazniejsze pomiary , odchylenie w osiach X,Y wyrazone w osiach. Przyspieszenie w osi Z.
*/
void YtesZyroskop::wypiszKluczowePomiary() {
	Serial.printf("Zyroskop  x:%.2f , y:%.2f , z:%.3f \n", myMpu->getAccAngleX(), myMpu->getAccAngleY() , myMpu->getAccZ());

};

//---------------------------TESTY DEBUGI I INNE PIERDY----------------------------------
/**
* @brief Metoda testowa. Generuje wartoœci zblizone do odczytów akcelerometru.
* Dla parametrów progDol = 1.0 , prog = 0.87 bêd¹ generowane liczby w zakresie
* 1...1.87
*/
float YtesZyroskop::losowePrzyspieszenie(float progDol, float prog) {
	return  progDol + ((float)rand() / (float)RAND_MAX) * prog;
};

void YtesZyroskop::wypiszRuchWstrzas() {	
	/*
	myMpu->update();
	float x = myMpu->getAccX();
	float y = myMpu->getAccY();
	float z = myMpu->getAccZ()-1;
	float total_accel = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	Serial.printf("%0.2f\t %0.2f\t %0.2f\t\t %0.2f \n", x, y, z, total_accel);
	*/
	float odychPX = odchylenieStandartowe(20, &pomiar_t::pX);
	float odychPY = odchylenieStandartowe(20, &pomiar_t::pY);
	//float odychPZ = odchylenieStandartowe(20, &pomiar_t::pZ);
	float t_odch = sqrt(pow(odychPX, 2) + pow(odychPY, 2)// + pow(odychPZ, 2)
	);
	#if SERIAL_ZYR_INFO == 1
	Serial.printf("%0.2f\t %0.2f\t\t %0.3f \n", odychPX, odychPY, t_odch);
	#endif
};

void YtesZyroskop::wypiszUstawieniaKalibracji() {
#if SERIAL_ZYR_INFO == 1
	Serial.printf("AKCE. OFSETY : x:%f , y:%f , z:%f \n", myMpu->getAccXoffset(), myMpu->getAccYoffset(), myMpu->getAccZoffset());
	Serial.printf("ZYRO. OFSETY : x:%f , y:%f , z:%f \n", myMpu->getGyroXoffset(), myMpu->getGyroYoffset(), myMpu->getGyroZoffset());
	Serial.printf("AKCELEROMETR FILTR : %f \n", myMpu->getFilterAccCoef());
	Serial.printf("ZYROSKOP FILTR : %f \n", myMpu->getFilterGyroCoef());
#endif	
};

void YtesZyroskop::wypiszPomiary(int ileWypisac) {
#if SERIAL_ZYR_INFO == 1
	int ile = 0;
	if (ileWypisac == 0 || ileWypisac> ILOSC_POMIAROW) ile = ILOSC_POMIAROW;
	else ile = ileWypisac;

	Serial.println("oX\t oY\t oZ\t\t pX\t pY\t pZ");
	for (int i = 0; i < ile; i++) {
		Serial.printf("%.2f\t %.2f\t %.2f\t\t %.2f\t %.2f\t %.2f \n ", pomiary[i].oX, pomiary[i].oY, pomiary[i].oZ,
			pomiary[i].pX, pomiary[i].pY, pomiary[i].pZ);
	}
#endif
};

void YtesZyroskop::wypiszPrzyspieszenieZ(){
#if SERIAL_ZYR_INFO == 1
	float odczyt = myMpu->getAccZ();
	if (odczyt < _Zmin) _Zmin = odczyt;
	if (odczyt > _Zmax) _Zmax = odczyt;
	Serial.printf("ACC Z: %.3f    %.2f    %.2f \n", odczyt, _Zmin, _Zmax);
#endif
}

/**
* @brief jak na razie najlepsza sposob testowania danych aby okreslic stan
*/
void YtesZyroskop::wypiszSredniaArr_oXYGZ() {
	Serial.printf("Srednie %.2f\t %.2f\t %0.2f\t\t PZ : pZ %0.2f:\t %0.2f\t %.2f \n",
		obliczeniaPomocnicze.saOX,obliczeniaPomocnicze.saOY,obliczeniaPomocnicze.saPZ,
		ostatniPomiar.pZ,minPole(10, &pomiar_t::pZ), maxPole(10, &pomiar_t::pZ));
}
void YtesZyroskop::wypiszWszystkieOdczyty() {
#if SERIAL_ZYR_INFO == 1
	myMpu->update();
	Serial.printf("TEMP.    ->%.2f \n",myMpu->getTemp());
	Serial.printf("AKCEL.   -> X:%.2f , Y:%.2f , Z: %.2f \n", myMpu->getAccX(), myMpu->getAccY(), myMpu->getAccZ());
	Serial.printf("ZYROSKOP -> X:%.2f , Y:%.2f , Z: %.2f \n", myMpu->getGyroX(), myMpu->getGyroY(), myMpu->getGyroZ());
	Serial.printf("ACC KAT  -> X:%.2f , Y:%.2f \n", myMpu->getAccAngleX(), myMpu->getAccAngleY());
	Serial.printf("KAT      -> X:%.2f, Y:%.2f , Z: %.2f \n", myMpu->getAngleX(), myMpu->getAngleY(), myMpu->getAngleZ());
	Serial.println();
#endif

}


#endif