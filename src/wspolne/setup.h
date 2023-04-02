// setup.h
#pragma once
#ifndef _SETUP_h
#define _SETUP_h

#include "arduino.h"
#include "SoftwareSerial.h"      // EspSoftwareSerial by Dirk Kaar, Peter Lerup Version 6.17.1
#include <HardwareSerial.h>
#include "Wire.h"
#include <MPU6050_light.h>
#include <ArduinoJson.h>

#define JSON_INCOMMING_BUFFER	1024	//Maksymalny rozmiar odpowiedzi od apki

#define SERIAL1_PRAWY_RX 33	//34	//Zmiana z blednego 14!! PODLUTOWAC SIE POD 14(RX) 33(TX)
#define SERIAL1_PRAWY_TX 34 //33
#define SERIAL2_LEWY_RX 16
#define SERIAL2_LEWY_TX 17

//Uwaga! hardwareSerial lepiej inicjalizowac globalnie , technicznie mozna by to zrobic w klasie YtesAudio
//ale robi siê straszny bajzel w kodzie.

#define UZYJ_HARDWARE	0	// takich typów portow UART u¿ywaæ do komunikacji z playerami mp3

#if UZYJ_HARDWARE == 1
HardwareSerial serialLewy(2);  // - urzyj uart2. (technicznie piny 16,17); - do komunikacji z playerLewy
HardwareSerial serialPrawy(1); // - mapuj uart1 (14,33); - do komunikacji z playerPrawy
#else 
SoftwareSerial serialLewy(SERIAL2_LEWY_RX, SERIAL2_LEWY_TX); // RX, TX
SoftwareSerial serialPrawy(SERIAL1_PRAWY_RX, SERIAL1_PRAWY_TX); // RX, TX
#endif // UZYJ_HARDWARE == 1

#define LEWE_KOLO_PIN 0		// domyslnie 4
#define PRAWE_KOLO_PIN 4	// domyslni 2 - dziala poprawnie
#define SERWO_RADAR_PIN 2	// serwo radau

MPU6050 mpu(Wire);

#define HCSR_TRIG_PIN 5
#define HCSR_ECHO_PIN 18

#define LED_PIN 13		//Uwaga, dla wersji z pojedyncza zwykla dioda led oraz dla paska Ledow ws2812B
#define COLOR_ORDER GRB                   //pozadek kolorow
#define LED_TYPE WS2812B                  //typ leda
#define NUM_LEDS 13                       // ilosc
#define BRIGHTNESS 128					  // polowa mocy	 

//Pin 0 - Srodkowy pomiêdzy 4 i 2  w przyszloœci dla  serwa radaru (4,0,2-sprzetowe piny PWM)
//---------------Makra do debugowania-------------
#define SERIAL_PD 					1	// wlacza i wylacza podleg³e makra
#define SERIAL_AUDIO_INFO			1	// co sie dzieje ww YtesAudio
#define SERIAL_AUDIO_ERROR			1	// tylko bledy z YtesAudio
#define SERIAL_NAPED_INFO			1	// co sie dzieje w YtesNaped
#define SERIAL_NAPED_ERROR			1	// tylko bledy z YtesNaped
#define SERIAL_ZYR_INFO				1	// co siê dzieje w YtesZyroskop
#define SERIAL_ZYR_ERROR			1	// wypisuj tylko b³êdy z YtesZyroskop
#define SERIAL_RADAR_INFO			1	// co siê dzieje w klasie YtesRadar
#define SEROAL_RADAR_ERROR			1	// wypisuj tylko b³êdy z YtesRadar

#if SERIAL_PD == 0
#define SERIAL_AUDIO_INFO	0
#define SERIAL_AUDIO_ERROR	0
#define SERIAL_NAPED_INFO	0
#define SERIAL_NAPED_ERROR	0
#define SERIAL_ZYR_INFO		0
#define SERIAL_ZYR_ERROR	0
#define SERIAL_RADAR_INFO	0
#define SERIAL_RADAR_ERROR	0
#endif

// PRINT ------------------------------------------
#define PRINT_MESSAGE(s) { Serial.println(F(s)); }
#define PRINT_MESSAGE_S(s,s1)  { Serial.print(F(s)); Serial.println(s1.c_str()); }
#define PRINT_MESSAGE_V(s,v) {Serial.print(F(s)); Serial.println(v); }
#define PRINT_MESSAGE_VV(s,v1,v2) {Serial.print(F(s)); Serial.print(v1); Serial.print(" , "); Serial.println(v2); }
// PD ---------------------------------------------
#if SERIAL_PD == 1
#define PD_INFO(s) PRINT_MESSAGE(s)
#define PD_INFO_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define PD_INFO_V(s,v) PRINT_MESSAGE_V(s,v)
#define PD_INFO_VV(s,v1,v2) PRINT_MESSAGE_VV(s,v1,v2)

#define PD_ERROR(s) PRINT_MESSAGE(s)
#define PD_ERROR_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define PD_ERROR_V(s,v) PRINT_MESSAGE_V(s,v)
#define PD_ERROR_VV(s,v1,v2) PRINT_MESSAGE_VV(s,v1,v2)


#else
#define PD_INFO(s)    
#define PD_INFO_S(s,s1)   
#define PD_INFO_V(s,v)
#define PD_INFO_VV(s,v1,v2)

#define PD_ERROR(s)
#define PD_ERROR_S(s,s1)
#define PD_ERROR_V(s,v)
#define PD_ERROR_VV(s,v1,v2)

#endif

// AUDIO -------------------------------------------
#if SERIAL_AUDIO_INFO == 1
#define AUDIO_INFO(s) PRINT_MESSAGE(s)
#define AUDIO_INFO_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define AUDIO_INFO_V(s,v) PRINT_MESSAGE_V(s,v)
#define AUDIO_INFO_VV(s,v1,v2) PRINT_MESSAGE_VV(s,v1,v2)

#else
#define AUDIO_INFO(s)    
#define AUDIO_INFO_S(s,s1)   
#define AUDIO_INFO_V(s,v)   
#define AUDIO_INFO_VV(s,v1,v2)
#endif

#if SERIAL_AUDIO_ERROR == 1
#define AUDIO_ERROR(s) PRINT_MESSAGE(s)
#define AUDIO_ERROR_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define AUDIO_ERROR_V(s,v) PRINT_MESSAGE_V(s,v)
#else
#define AUDIO_ERROR(s)
#define AUDIO_ERROR_S(s,s1)
#define AUDIO_ERROR_V(s,v)
#endif
// NAPED -------------------------------------------
#if SERIAL_NAPED_INFO == 1
#define NAPED_INFO(s) PRINT_MESSAGE(s)
#define NAPED_INFO_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define NAPED_INFO_V(s,v) PRINT_MESSAGE_V(s,v)
#define NAPED_INFO_VV(s,v1,v2) PRINT_MESSAGE_VV(s,v1,v2)

#else
#define NAPED_INFO(s)    
#define NAPED_INFO_S(s,s1)   
#define NAPED_INFO_V(s,v)   
#define NAPED_INFO_VV(s,v1,v2)
#endif

#if SERIAL_AUDIO_ERROR == 1
#define NAPED_ERROR(s) PRINT_MESSAGE(s)
#define NAPED_ERROR_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define NAPED_ERROR_V(s,v) PRINT_MESSAGE_V(s,v)
#else
#define NAPED_ERROR(s)
#define NAPED_ERROR_S(s,s1)
#define NAPED_ERROR_V(s,v)
#endif

// ZYROSKOP -------------------------------------------
#if SERIAL_ZYR_INFO == 1
#define ZYR_INFO(s) PRINT_MESSAGE(s)
#define ZYR_INFO_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define ZYR_INFO_V(s,v) PRINT_MESSAGE_V(s,v)
#define ZYR_INFO_VV(s,v1,v2) PRINT_MESSAGE_VV(s,v1,v2)

#else
#define ZYR_INFO(s)    
#define ZYR_INFO_S(s,s1)   
#define ZYR_INFO_V(s,v)   
#define ZYR_INFO_VV(s,v1,v2)
#endif

#if SERIAL_ZYR_ERROR == 1
#define ZYR_ERROR(s) PRINT_MESSAGE(s)
#define ZYR_ERROR_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define ZYR_ERROR_V(s,v) PRINT_MESSAGE_V(s,v)
#else
#define ZYR_ERROR(s)
#define ZYR_ERROR_S(s,s1)
#define ZYR_ERROR_V(s,v)
#endif

// RADAR -------------------------------------------
#if SERIAL_RADAR_INFO == 1
#define RADAR_INFO(s) PRINT_MESSAGE(s)
#define RADAR_INFO_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define RADAR_INFO_V(s,v) PRINT_MESSAGE_V(s,v)
#define RADAR_INFO_VV(s,v1,v2) PRINT_MESSAGE_VV(s,v1,v2)

#else
#define RADAR_INFO(s)    
#define RADAR_INFO_S(s,s1)   
#define RADAR_INFO_V(s,v)   
#define RADAR_INFO_VV(s,v1,v2)
#endif

#if SERIAL_RADAR_ERROR == 1
#define RADAR_ERROR(s) PRINT_MESSAGE(s)
#define RADAR_ERROR_S(s,s1) PRINT_MESSAGE_S(s,s1)
#define RADAR_ERROR_V(s,v) PRINT_MESSAGE_V(s,v)
#else
#define RADAR_ERROR(s)
#define RADAR_ERROR_S(s,s1)
#define RADAR_ERROR_V(s,v)
#endif


#endif

