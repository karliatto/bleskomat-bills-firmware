const apiKeyId = process.env.API_KEY_ID || null;
const useDummyFlags = process.env.API_KEY_NONE || null;
if (!apiKeyId && !useDummyFlags) {
	console.error('ERROR: `API_KEY_ID` or `API_KEY_NONE` env variable is required.');
	process.exit(1);
}

if (!useDummyFlags && !require('../config').fileExists(false)) {
	// Intent to use real configuration values.
	// Config file is required.
	if (!require('../config').fileExists(true)) {
		// No configuration file exists.
		console.error('ERROR: Config file not found. Use `npm run config -- init` to create it.');
	} else {
		// An encrypted config file exists.
		// But the build process requires it to be unencrypted.
		console.error('ERROR: Unencrypted config file not found. Use `npm run config -- decrypt` to create it.');
	}
	process.exit(1);
}

const printBuildFlags = function(buildFlags) {
	const output = Object.keys(buildFlags).map(function(name) {
		const value = buildFlags[name];
		// Use single quotes around whole build flag definition + double-quotes around value:
		return `'-D ${name}="${value}"'`;
	}).join('\n');
	process.stdout.write(output);
};

const dummyFlags = {
	API_KEY_ID: '',
	API_KEY_SECRET: '',
	CALLBACK_URL: 'http://localhost:3000/u',
	FIAT_CURRENCY: 'CZK',
};

if (useDummyFlags) {
	printBuildFlags(dummyFlags);
	process.exit();
}

require('../config').load().then(config => {
	if (!config) {
		// Print dummy values for required build flags.
		return dummyFlags;
	}
	const db = require('../db')(config);
	return db.onReady().then(() => {
		return db.getApiKey(apiKeyId).then(apiKey => {
			if (!apiKey) {
				throw new Error(`API key not found: "${apiKeyId}"`);
			}
			const { host, port, url, protocol, endpoint } = config.lnurl;
			return {
				API_KEY_ID: apiKey.id,
				API_KEY_SECRET: apiKey.key,
				CALLBACK_URL: (url ? url : `${protocol}://${host}:${port}`) + endpoint,
				FIAT_CURRENCY: apiKey.fiatCurrency || 'CZK',
			};
		});
	});
}).then(buildFlags => {
	printBuildFlags(buildFlags);
	process.exit();
}).catch(error => {
	console.error(error);
	process.exit(1);
});
