#include "main.h"

unsigned int buttonDelay;
std::string initializeScreen = "";

void setup() {
	Serial.begin(MONITOR_SPEED);
	spiffs::init();
	config::init();
	logger::init();
	logger::write(firmwareName + ": Firmware version = " + firmwareVersion + ", commit hash = " + firmwareCommitHash);
	logger::write(config::getConfigurationsAsString());
	jsonRpc::init();
	network::init();
	screen::init();
	billAcceptor::init();
	button::init();
	initializeScreen = cache::getString("lastScreen");
	logger::write("Cache loaded lastScreen: " + initializeScreen);
	buttonDelay = config::getUnsignedInt("buttonDelay");
}

float getAccumulatedValue() {
	float accumulatedValue = billAcceptor::getAccumulatedValue();
	const float billAcceptorEscrowValue = billAcceptor::getEscrowValue();
	if (billAcceptorEscrowValue > 0) {
		const float buyLimit = config::getFloat("buyLimit");
		if (billAcceptorEscrowValue + accumulatedValue > buyLimit) {
			billAcceptor::rejectEscrow();
		} else {
			billAcceptor::acceptEscrow();
		}
	}
	return accumulatedValue;
}

float amountShown = 0;
unsigned long tradeCompleteTime = 0;

void writeTradeCompleteLog(const float &amount, const std::string &signedUrl) {
	std::string msg = "Trade completed:\n";
	msg += "  Amount  = " + util::floatToStringWithPrecision(amount, config::getUnsignedShort("fiatPrecision")) + " " + config::getString("fiatCurrency") + "\n";
	msg += "  URL     = " + signedUrl;
	logger::write(msg);
}

void runAppLoop() {
	screen::loop();
	network::loop();
	platform::loop();
	billAcceptor::loop();
	button::loop();
	const std::string currentScreen = screen::getCurrentScreen();
	if (currentScreen == "" && screen::isReady()) {
		if (config::getBool("enabled")) {
			if (initializeScreen == "insertFiat") {
				const std::string cacheAccumulatedValue = cache::getString("accumulatedValue");
				logger::write("Cache loaded accumulatedValue: " + cacheAccumulatedValue);
				if (cacheAccumulatedValue != "") {
					billAcceptor::setAccumulatedValue(util::stringToFloat(cacheAccumulatedValue));
					screen::showInsertFiatScreen(util::stringToFloat(cacheAccumulatedValue));
					billAcceptor::disinhibit();
				} else {
					screen::showSplashScreen();
					billAcceptor::disinhibit();
				}
			} else if (initializeScreen == "tradeComplete") {
				const std::string cachedQrcodeData = cache::getString("qrcodeData");
				const std::string cachedAccumulatedValue = cache::getString("accumulatedValue");
				const std::string cachedReferencePhrase = cache::getString("referencePhrase");
				logger::write("Cache loaded qrcodeData: " + cachedQrcodeData);
				logger::write("Cache loaded accumulatedValue: " + cachedAccumulatedValue);
				logger::write("Cache loaded referencePhrase: " + cachedReferencePhrase);
				if (cachedQrcodeData != "" && cachedAccumulatedValue != "" && cachedReferencePhrase != "") {
					screen::showTradeCompleteScreen(util::stringToFloat(cachedAccumulatedValue), cachedQrcodeData, cachedReferencePhrase);
					billAcceptor::inhibit();
				} else {
					screen::showSplashScreen();
					billAcceptor::disinhibit();
				}
			} else {
				screen::showSplashScreen();
				billAcceptor::disinhibit();
			}
		} else {
			screen::showDisabledScreen();
			billAcceptor::inhibit();
		}
		initializeScreen = "";
	}
	const float accumulatedValue = getAccumulatedValue();
	const bool tradeInProgress = accumulatedValue > 0 && (currentScreen == "insertFiat" || currentScreen == "tradeComplete");
	if (
		// Do not disable device when a trade is in progress.
		!tradeInProgress &&
		(
			// Device is disabled via configuration option.
			!config::getBool("enabled") ||
			// Online-only mode and not connected to platform.
			config::getBool("onlineOnly") && !platform::isConnected() ||
			// Bill acceptor is not connected.
			(!billAcceptor::isConnected() && millis() > 10000)
		)
	) {
		// Show device disabled screen and do not allow normal operation.
		if (currentScreen != "disabled") {
			billAcceptor::inhibit();
			screen::showDisabledScreen();
		}
	} else {
		// Device is enabled.
		if (currentScreen == "disabled") {
			// Previously disabled, return to normal operation.
			screen::showSplashScreen();
			billAcceptor::disinhibit();
		}
		if (
			accumulatedValue > 0 &&
			currentScreen != "insertFiat" &&
			currentScreen != "tradeComplete"
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
					const float exchangeRate = platform::getExchangeRate();
					if (exchangeRate > 0) {
						customParams["er"] = std::to_string(exchangeRate);
					}
					// Create a withdraw request and render it as a QR code.
					const std::string signedUrl = util::createSignedLnurlWithdraw(accumulatedValue, customParams);
					const std::string encoded = util::lnurlEncode(signedUrl);
					std::string qrcodeData = "";
					// Allows upper or lower case URI schema prefix via a configuration option.
					// Some wallet apps might not support uppercase URI prefixes.
					qrcodeData += config::getString("uriSchemaPrefix");
					// QR codes with only uppercase letters are less complex (easier to scan).
					qrcodeData += util::toUpperCase(encoded);
					screen::showTradeCompleteScreen(accumulatedValue, qrcodeData, referencePhrase);
					writeTradeCompleteLog(accumulatedValue, signedUrl);
					billAcceptor::inhibit();
					tradeCompleteTime = millis();
				} else {
					// Button pressed with zero amount.
					screen::showInstructionsScreen();
				}
			} else {
				// Button not pressed.
				// Ensure that the amount shown is correct.
				if (amountShown != accumulatedValue) {
					screen::showInsertFiatScreen(accumulatedValue);
					amountShown = accumulatedValue;
				}
			}
		} else if (currentScreen == "tradeComplete") {
			if (button::isPressed() && millis() - tradeCompleteTime > buttonDelay) {
				// Button pushed while showing the transaction complete screen.
				// Reset accumulated values.
				billAcceptor::resetAccumulatedValue();
				billAcceptor::disinhibit();
				amountShown = 0;
				screen::showSplashScreen();
			}
		}
	}
}

void loop() {
	logger::loop();
	jsonRpc::loop();
	if (!jsonRpc::hasPinConflict() || !jsonRpc::inUse()) {
		runAppLoop();
	}
}
