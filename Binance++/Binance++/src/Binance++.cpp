#include "Binance++.h"

#include "Internal/BinanceErr.h"
#include "Internal/Utils.h"
#include "Internal/Query.h"

namespace binance
{
	void InitLibrary()
	{
		CURLcode ec = curl_global_init(CURL_GLOBAL_DEFAULT);
		if (ec != CURLE_OK)
			throw std::exception(CombineCurlErr("Failed to init curl: ", ec).c_str());
	}

	void CleanupLibrary()
	{
		curl_global_cleanup();
	}


	Binance::Binance(std::string apiKey, std::string secretKey)
	{
		this->apiKey = apiKey;
		this->secretKey = secretKey;
		curl = curl_easy_init();

		if (!curl)
		{
			errorMessage = "Failed to initialize curl easy handle";
			throw BinanceException(this);
		}
	}

	Binance::~Binance()
	{
		curl_easy_cleanup(curl);
	}

#pragma region requests
	// GET /sapi/v1/system/status
	bool Binance::GetSystemStatus(nlohmann::json& res)
	{
		std::string url(Binance_Host);
		url += "/sapi/v1/system/status";

		return CurlApi(url, res);
	}

	// GET /sapi/v1/capital/config/getall
	bool Binance::GetAllCoinInfo(long recvWindow, nlohmann::json& res)
	{
		std::string url(Binance_Host);
		url += "/sapi/v1/capital/config/getall";

		Query query;

		if (recvWindow > 0)
			query.AppendQuery("recvWindow", recvWindow);
		query.AppendQuery("timestamp", GetMsEpoch());
		SignQuery(query, secretKey);

		url.append(query.GetString());

		std::vector<std::string> headerData;
		AddApiKey(headerData, apiKey);

		return CurlApi(url, res, "GET", headerData);
	}

	// GET /sapi/v1/accountSnapshot
	bool Binance::GetDailyAccountSnapshot(
		std::string type,
		long startTime,
		long endTime,
		int limit,
		long recvWindow,
		nlohmann::json& res
	)
	{
		std::string url(Binance_Host);
		url += "/sapi/v1/accountSnapshot";

		Query query;

		query.AppendQuery("type", type);
		if (startTime > 0)
			query.AppendQuery("startTime", startTime);
		if (endTime > 0)
			query.AppendQuery("endTime", endTime);
		if (limit > 0)
			query.AppendQuery("limit", limit);
		if (recvWindow > 0)
			query.AppendQuery("recvWindow", recvWindow);
		query.AppendQuery("timestamp", GetMsEpoch());
		SignQuery(query, secretKey);

		url.append(query.GetString());

		std::vector<std::string> headerData;
		AddApiKey(headerData, apiKey);

		return CurlApi(url, res, "GET", headerData);
	}
#pragma endregion


#pragma region error helpers
	std::string Binance::GetErrorMessage()
	{
		return errorMessage;
	}


	bool Binance::ReturnError(std::string err)
	{
		errorMessage = err;
		return false;
	}

	std::string CombineCurlErr(std::string msg, CURLcode ec)
	{
		return (msg + curl_easy_strerror(ec));
	}
#pragma endregion


#pragma region curl helpers
	size_t CurlCallback(void* content, size_t size, size_t nmemb, std::string* buffer)
	{
		buffer->append((char*)content, size * nmemb);
		return size * nmemb;
	}

	bool Binance::CurlApi(std::string url, nlohmann::json& json, std::string action, std::vector<std::string> extraHeaderData, std::string postData)
	{
		CURLcode cec;
		std::string requestResult;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CurlCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &requestResult);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");

		if (extraHeaderData.size() > 0)
		{
			curl_slist* chunk = nullptr;
			for (int i = 0; i < extraHeaderData.size(); i++)
				chunk = curl_slist_append(chunk, extraHeaderData[i].c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		}

		if (postData.size() > 0 || action == "POST" || action == "PUT" || action == "DELETE")
		{
			if (action == "PUT" || action == "DELETE")
				curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, action.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		}

		cec = curl_easy_perform(curl);

		if (cec != CURLE_OK)
			return ReturnError(CombineCurlErr("Failed to perform request to API endpoint: ", cec));

		if (requestResult.size() == 0)
			return ReturnError("No return data from API request");

		json.parse(requestResult);

		return true;
	}
#pragma endregion
}
