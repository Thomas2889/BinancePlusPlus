#pragma once

#include <string>

#include <curl/curl.h>
#include <json.hpp>


namespace binance
{
	void InitLibrary();

	void CleanupLibrary();

	class Binance
	{
	private:
		const std::string Binance_Host = "https://api.binance.com";

	private:
		std::string apiKey;
		std::string secretKey;
		CURL* curl;

		std::string errorMessage;

	public:
		Binance(std::string apiKey, std::string secretKey);
		~Binance();


		bool GetSystemStatus(nlohmann::json& res);
		bool GetAllCoinInfo(long recvWindow, nlohmann::json& res);
		bool GetDailyAccountSnapshot(
			std::string type,
			long startTime,
			long endTime,
			int limit,
			long recvWindow,
			nlohmann::json& res
		);


		std::string GetErrorMessage();

	private:
		bool ReturnError(std::string err);
		const char* CombineCurlErr(std::string msg, CURLcode ec);

		bool CurlApi(std::string url, nlohmann::json& json, std::string action = "GET", std::vector<std::string> extraHeaderData = {}, std::string postData = "");
	};
}
