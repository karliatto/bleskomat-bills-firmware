#ifndef BLESKOMAT_MAIN_H
#define BLESKOMAT_MAIN_H

#include "cache.h"
#include "bill-acceptor.h"
#include "button.h"
#include "config.h"
#include "i18n.h"
#include "logger.h"
#include "json-rpc.h"
#include "network.h"
#include "platform.h"
#include "screen.h"
#include "spiffs.h"
#include "util.h"

#include <lnurl.h>
#include <string>

#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s

#ifndef FIRMWARE_NAME
	#error "Missing required build flag: FIRMWARE_NAME"
#endif

#ifndef FIRMWARE_COMMIT_HASH
	#error "Missing required build flag: FIRMWARE_COMMIT_HASH"
#endif

#ifndef FIRMWARE_VERSION
	#error "Missing required build flag: FIRMWARE_VERSION"
#endif

namespace {
	std::string trimQuotes(const std::string &str) {
		return str.substr(1, str.length() - 2);
	}
}

const std::string firmwareName(trimQuotes(STRINGIFY(FIRMWARE_NAME)));
const std::string firmwareCommitHash(trimQuotes(STRINGIFY(FIRMWARE_COMMIT_HASH)));
const std::string firmwareVersion(trimQuotes(STRINGIFY(FIRMWARE_VERSION)));

#endif
