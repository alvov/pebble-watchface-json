#pragma once
#include "pebble.h"

#ifdef PBL_COLOR
  #define COLOR_TIME GColorPastelYellow
  #define COLOR_DATE GColorSunsetOrange
  #define COLOR_SYSTEM GColorMintGreen
#else
  #define COLOR_TIME GColorWhite
  #define COLOR_DATE GColorWhite
  #define COLOR_SYSTEM GColorWhite
#endif