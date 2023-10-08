#ifndef defines_h
#define defines_h

#define EnA 21
#define EnB 16
#define In1 20
#define In2 19
#define In3 18
#define In4 17

#if ( defined(ARDUINO_RASPBERRY_PI_PICO_W) )
  #if defined(WEBSOCKETS_USE_RP2040W)
    #undef WEBSOCKETS_USE_RP2040W
  #endif
  #define WEBSOCKETS_USE_RP2040W            true
  #define USE_RP2040W_WIFI                  true
  #define USE_WIFI_NINA                     false
#else
  #error This code is intended to run only on the RP2040W boards ! Please check your Tools->Board setting.
#endif

#include <WiFi.h>

#define DEBUG_WEBSOCKETS_PORT     Serial
// Debug Level from 0 to 4
#define _WEBSOCKETS_LOGLEVEL_     0

const char* ssid  = "";       //Enter SSID
const char* pass  = "";   //Enter Password
const char* websockets_server_host = "";  //Enter server address
const uint16_t websockets_server_port = 8080;  // Enter server port

#endif      //defines_h
