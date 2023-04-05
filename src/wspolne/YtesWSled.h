#pragma once
#include "setup.h"
#include <FastLED.h>

class YtesWSled {
private:
	CRGB _ustawionyKolor = CRGB(0, 255, 128);
	CRGB ledy[NUM_LEDS];
public:
	boolean saWlaczone = true;
	YtesWSled();
	void ustawKolorRgb(byte r, byte g, byte b);
	void wylacz();
	void wlacz();
	void wzorNiePolaczony();
	void wzorPolaczony();
	void obslozPolecenieDane(JsonObject* dane);
	String odpowiedz();
};

YtesWSled::YtesWSled() {
	LEDS.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(ledy, NUM_LEDS);
	FastLED.setBrightness(BRIGHTNESS);	
}

void YtesWSled::ustawKolorRgb(byte r, byte g, byte b) {
	_ustawionyKolor.r = r;
	_ustawionyKolor.g = g;
	_ustawionyKolor.b = b;
	ledy->setRGB(r, g, b);
	FastLED.show();
};

void YtesWSled::wzorNiePolaczony() {
	//fill_gradient_RGB(leds + START_LED, NUM_LEDS - START_LED, COLOR_START, COLOR_END);
	saWlaczone = true;
	fill_gradient_RGB(ledy, NUM_LEDS, CRGB::Green, CRGB::Black);
	FastLED.show();
};

void YtesWSled::wzorPolaczony() {
	saWlaczone = true;
	fill_gradient_RGB(ledy, NUM_LEDS, CRGB::Red, CRGB::Black);
	FastLED.show();
};

void YtesWSled::wylacz() {
	saWlaczone = false;
	FastLED.clear();
	FastLED.show();
};

void YtesWSled::wlacz() {
	saWlaczone = true;
	fill_gradient_RGB(ledy, NUM_LEDS, _ustawionyKolor, CRGB::Black);
	FastLED.show();
};

/**
* @brief z obiektu ledy , obsloz dane:
* {
* "WLACZONE" : 1,0 - wlacz lub wylacz ledy
* "KOLOR" {"R":xxx,"G":xxx,"B":xxx} - ustaw kolor, xxx w zakresie 0..255
*/

void YtesWSled::obslozPolecenieDane(JsonObject* dane) {
	int intWlaczone = 0;

	JsonVariant vWlaczone = (*dane)["WLACZONE"];
	if (!vWlaczone.isNull()) {		
		intWlaczone = vWlaczone.as<int>();
		if (intWlaczone == 1) saWlaczone = true; else false;
	};

	JsonVariant vKolor = (*dane)["KOLOR"];
	if (!vKolor.isNull()) {
		JsonObject kolor = vKolor.as<JsonObject>();
		byte tmpR = 0, tmpG = 0, tmpB = 0;
		JsonVariant vR = kolor["R"];
		JsonVariant vG = kolor["G"];
		JsonVariant vB = kolor["B"];
		if (!vR.isNull() && !vG.isNull() && !vB.isNull()) {
			_ustawionyKolor.r = vR.as<byte>();
			_ustawionyKolor.g = vG.as<byte>();
			_ustawionyKolor.b = vB.as<byte>();
		};
	};

	//zgodnie z ostatnim wskazaniem
	if (saWlaczone) wlacz();
	else wylacz();
};

/**
* @brief Wygeneruj stringa reprezentujacego aktualne ustawienia klasy
*/
String YtesWSled::odpowiedz() {
	StaticJsonDocument<512> doc;	
	JsonObject objData = doc.createNestedObject("ledy");
	objData["WLACZONE"] = (byte)saWlaczone;
	JsonObject objKolor = objData.createNestedObject("KOLOR");
	objKolor["R"] = _ustawionyKolor.r;
	objKolor["G"] = _ustawionyKolor.g;
	objKolor["B"] = _ustawionyKolor.b;

	String tmp = "";
	size_t resSize = serializeJson(doc, tmp);
	int firstIndex = tmp.indexOf('{');
	int lastIndex = tmp.lastIndexOf('}');
	String odp = tmp.substring(firstIndex + 1, lastIndex);
	return odp;
};