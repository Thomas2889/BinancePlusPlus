#pragma once

#include <string>
#include <vector>

#include "Query.h"


namespace binance
{
	void SignQuery(Query& query, std::string secretKey);
	void AddApiKey(std::vector<std::string>& headerData, std::string apiKey);

	int64_t GetMsEpoch();
}
