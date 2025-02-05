#ifndef BLESKOMAT_SCREEN_EPAPER_H
#define BLESKOMAT_SCREEN_EPAPER_H

#include "fonts/u8g2/courier_prime_code_48pt.h"
#include "fonts/u8g2/courier_prime_code_44pt.h"
#include "fonts/u8g2/courier_prime_code_40pt.h"
#include "fonts/u8g2/courier_prime_code_36pt.h"
#include "fonts/u8g2/courier_prime_code_32pt.h"
#include "fonts/u8g2/courier_prime_code_28pt.h"
#include "fonts/u8g2/courier_prime_code_24pt.h"
#include "fonts/u8g2/courier_prime_code_20pt.h"
#include "fonts/u8g2/courier_prime_code_16pt.h"
#include "fonts/u8g2/courier_prime_code_14pt.h"
#include "fonts/u8g2/courier_prime_code_12pt.h"
#include "fonts/u8g2/courier_prime_code_11pt.h"
#include "fonts/u8g2/courier_prime_code_10pt.h"
#include "fonts/u8g2/courier_prime_code_9pt.h"
#include "fonts/u8g2/courier_prime_code_8pt.h"

#include "fonts/u8g2/checkbooklightning_48pt.h"
#include "fonts/u8g2/checkbooklightning_44pt.h"
#include "fonts/u8g2/checkbooklightning_40pt.h"
#include "fonts/u8g2/checkbooklightning_36pt.h"

#include "config.h"
#include "i18n.h"
#include "logger.h"
#include "network.h"
#include "platform.h"
#include "util.h"

#ifndef EPAPER_CLK
#define EPAPER_CLK 13
#endif

#ifndef EPAPER_MISO
#define EPAPER_MISO 12
#endif

#ifndef EPAPER_DIN
#define EPAPER_DIN 14
#endif

#ifndef EPAPER_CS
#define EPAPER_CS 15
#endif

#ifndef EPAPER_DC
#define EPAPER_DC 27
#endif

#ifndef EPAPER_RST
#define EPAPER_RST 26
#endif

#ifndef EPAPER_BUSY
#define EPAPER_BUSY 25
#endif

#define ENABLE_GxEPD2_GFX 0

#include <Arduino.h>
#include <qrcode.h>
#include <Wire.h>
#include <GxEPD2_GFX.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

#include <cmath>
#include <stdexcept>
#include <sstream>
#include <vector>

namespace screen_epaper {
	void init();
	void loop();
	bool isReady();
	void showSplashScreen();
	void showDisabledScreen();
	void showInstructionsScreen();
	void showInsertFiatScreen(const float &amount);
	void showTradeCompleteScreen(
		const float &amount,
		const std::string &qrcodeData,
		const std::string &referencePhrase = ""
	);
}

#endif
