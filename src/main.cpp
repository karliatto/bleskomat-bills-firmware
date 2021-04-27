#include "main.h"

void setup() {
	Serial.begin(MONITOR_SPEED);
	sdcard::init();
	logger::write("Bleskomat firmware version=" + firmwareVersion + ",commit=" + firmwareCommitHash);
	config::init();
	logger::write("Config OK");
	network::init();
	logger::write("Network OK");
	modules::init();
	logger::write("Modules OK");
	logger::write("Setup OK");
	if (config::isEnabled()) {
		screen::showSplashScreen();
	} else {
		screen::showDisabledScreen();
	}
}

float amountShown = 0;

void loop() {
	network::loop();
	modules::loop();
	const std::string currentScreen = screen::getCurrentScreen();
	if (!config::isEnabled()) {
		// Device is disabled.
		if (currentScreen != "disabled") {
			screen::showDisabledScreen();
		}
	} else {
		// Enabled.
		const double exchangeRate = config::getExchangeRate();
		const double buyLimit = config::getBuyLimit();
		float accumulatedValue = 0;
		#ifdef COIN_ACCEPTOR
			accumulatedValue += coinAcceptor::getAccumulatedValue();
		#endif
		#ifdef BILL_ACCEPTOR
			accumulatedValue += billAcceptor::getAccumulatedValue();
			float billAcceptorEscrowValue = billAcceptor::getEscrowValue();
			if (billAcceptorEscrowValue > 0 && billAcceptorEscrowValue + accumulatedValue > buyLimit) {
				billAcceptor::rejectEscrow();
			} else if (billAcceptorEscrowValue > 0) {
				billAcceptor::acceptEscrow();
			}
		#endif
		if (
			accumulatedValue > 0 &&
			currentScreen != "insertFiat" &&
			currentScreen != "transactionComplete"
		) {
			screen::showInsertFiatScreen(accumulatedValue);
			amountShown = accumulatedValue;
		}
		if (currentScreen == "splash") {
			if (button::isPressed()) {
				screen::showInstructionsScreen();
			}
		} else if (currentScreen == "instructions") {
			if (button::isPressed()) {
				screen::showInsertFiatScreen(0);
			}
		} else if (currentScreen == "insertFiat") {
			if (button::isPressed()) {
				if (accumulatedValue > 0) {
					// Button pushed while insert fiat screen shown and accumulated value greater than 0.
					// Generate random words that will be displayed to the user as a human-friendly proof/reference code.
					const std::string referencePhrase = util::generateRandomPhrase(5);
					Lnurl::Query customParams;
					customParams["r"] = referencePhrase;
					if (exchangeRate > 0) {
						customParams["er"] = std::to_string(exchangeRate);
					}
					// Create a withdraw request and render it as a QR code.
					const std::string signedUrl = util::createSignedLnurlWithdraw(accumulatedValue, customParams);
					const std::string encoded = util::lnurlEncode(signedUrl);
					const std::string uriSchemaPrefix = config::get("uriSchemaPrefix");
					std::string qrcodeData = "";
					if (uriSchemaPrefix != "") {
						// Allows upper or lower case URI schema prefix via a configuration option.
						// Some wallet apps might not support uppercase URI prefixes.
						qrcodeData += uriSchemaPrefix + ":";
					}
					// QR codes with only uppercase letters are less complex (easier to scan).
					qrcodeData += util::toUpperCase(encoded);
					screen::showTransactionCompleteScreen(accumulatedValue, qrcodeData, referencePhrase);
					// Save the transaction for debugging and auditing purposes.
					logger::logTransaction(signedUrl);
					#ifdef COIN_ACCEPTOR
						coinAcceptor::off();
					#endif
					#ifdef BILL_ACCEPTOR
						billAcceptor::off();
					#endif
				} else {
					// Button pressed with zero amount.
					screen::showInstructionsScreen();
				}
			} else {
				// Button not pressed.
				// Ensure that the amount shown is correct.
				if (amountShown != accumulatedValue) {
					screen::updateInsertFiatScreenAmount(accumulatedValue);
					amountShown = accumulatedValue;
				}
				#ifdef COIN_ACCEPTOR
					float maxCoinValue = coinAcceptor::getMaxCoinValue();
					if (buyLimit > 0 && coinAcceptor::isOn() && (accumulatedValue + maxCoinValue) > buyLimit) {
						// Possible to exceed tx limit, so disallow entering more coins.
						coinAcceptor::off();
					}
				#endif
			}
		} else if (currentScreen == "transactionComplete") {
			if (button::isPressed()) {
				// Button pushed while showing the transaction complete screen.
				// Reset accumulated values.
				#ifdef COIN_ACCEPTOR
					coinAcceptor::reset();
					coinAcceptor::on();
				#endif
				#ifdef BILL_ACCEPTOR
					billAcceptor::reset();
					billAcceptor::on();
				#endif
				amountShown = 0;
				screen::showSplashScreen();
			}
		}
	}
}
