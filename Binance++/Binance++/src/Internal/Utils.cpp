#include "Utils.h"

#include <chrono>

#include <openssl/hmac.h>


namespace binance
{
	std::string b2aHex(char* byte_arr, int n) {

		const static std::string Hex_Codes = "0123456789abcdef";
		std::string hexString;
		for (int i = 0; i < n; ++i) {
			unsigned char binValue = byte_arr[i];
			hexString += Hex_Codes[(binValue >> 4) & 0x0F];
			hexString += Hex_Codes[binValue & 0x0F];
		}
		return hexString;
	}

	void SignQuery(Query& query, std::string secretKey)
	{
		unsigned char* digest;
		digest = HMAC(EVP_sha256(),
			secretKey.c_str(),
			secretKey.length(),
			(unsigned char*)query.c_str(),
			query.length(),
			NULL, NULL);
		
		query.AppendQuery("siganture=", b2aHex((char*)digest, 32));
	}

	void AddApiKey(std::vector<std::string>& headerData, std::string apiKey)
	{
		headerData.push_back("X-MBX-APIKEY: " + apiKey);
	}


	int64_t GetMsEpoch()
	{
		return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
	}
}
