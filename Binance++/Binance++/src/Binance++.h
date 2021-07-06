#pragma once

#include <string>
#include <chrono>

#include <curl/curl.h>
#include <json.hpp>


namespace binance
{
	void InitLibrary();

	void CleanupLibrary();

	class Binance
	{
	private:
		const std::string Spot_Host = "https://api.binance.com";
		const std::string Futures_Host = "https://fapi.binance.com";

	private:
		std::string apiKey;
		std::string secretKey;
		CURL* curl;
		bool timeout = false;
		std::chrono::steady_clock::time_point timeoutPoint;


		std::string errorMessage;
		std::string lastURLRequest;

	public:
		Binance(std::string apiKey = "", std::string secretKey = "");
		~Binance();


		bool GetSystemStatus(nlohmann::json& res);
		bool GetAllCoinInfo(int64_t recvWindow, nlohmann::json& res);
		bool GetDailyAccountSnapshot(
			std::string type,
			int64_t startTime,
			int64_t endTime,
			int limit,
			int64_t recvWindow,
			nlohmann::json& res
		);

		bool GetContinousContractKlineData(
			std::string pair,
			std::string contractType,
			std::string interval,
			int64_t startTime,
			int64_t endTime,
			int limit,
			nlohmann::json& res
		);


		std::string GetErrorMessage();
		std::string GetLastURLRequest();

	private:
		bool ReturnError(std::string err);
		static std::string CombineCurlErr(std::string msg, CURLcode ec);

		bool CurlApi(std::string url, nlohmann::json& json, std::string action = "GET", std::vector<std::string> extraHeaderData = {}, std::string postData = "");
	};
}
