#include "modules/coin-acceptor.h"

namespace {
	std::string fiatCurrency = "";
	float VALUE_ACCUMULATED = 0.00;
	uint8_t LAST_PIN_READ;
	unsigned long LAST_PIN_READ_TIME = 0;
	bool COIN_INSERTED = false;
	unsigned long LAST_COIN_INSERTED_TIME = 0;

	bool coinWasInserted() {
		unsigned long diffTime = millis() - LAST_PIN_READ_TIME;
		return LAST_PIN_READ == LOW && diffTime > 25 && diffTime < 35;
	}

	void flipPinState() {
		// Flip the state of the last read.
		LAST_PIN_READ = LAST_PIN_READ == HIGH ? LOW : HIGH;
		LAST_PIN_READ_TIME = millis();
	}

	uint8_t readPin() {
		return digitalRead(COIN_ACCEPTOR_PIN);
	}

	bool pinStateHasChanged() {
		return readPin() != LAST_PIN_READ;
	}

	float getValueIncrement() {
		if (fiatCurrency == "EUR") {
			return 0.05;
		}
		return 1.0;
	}

	void incrementAccumulatedValue() {
		VALUE_ACCUMULATED = VALUE_ACCUMULATED + getValueIncrement();
	}
}

namespace coinAcceptor {

	void init() {
		pinMode(COIN_ACCEPTOR_PIN, INPUT_PULLUP);
		LAST_PIN_READ = readPin();
	}

	void loop() {
		COIN_INSERTED = false;
		if (pinStateHasChanged()) {
			if (coinWasInserted()) {
				// A coin was inserted.
				// This code executes once for each value unit the coin represents.
				// For example: A coin worth 5 CZK will execute this code 5 times.
				incrementAccumulatedValue();
				LAST_COIN_INSERTED_TIME = millis();
				COIN_INSERTED = true;
			}
			flipPinState();
		}
	}

	bool coinInserted() {
		return COIN_INSERTED;
	}

	unsigned long getTimeSinceLastCoinInserted() {
		return LAST_COIN_INSERTED_TIME > 0 ? millis() - LAST_COIN_INSERTED_TIME : 0;
	}

	float getAccumulatedValue() {
		return VALUE_ACCUMULATED;
	}

	void setFiatCurrency(const std::string &str) {
		fiatCurrency = str;
	}

	void reset() {
		COIN_INSERTED = false;
		LAST_COIN_INSERTED_TIME = 0;
		VALUE_ACCUMULATED = 0.00;
	}
}
