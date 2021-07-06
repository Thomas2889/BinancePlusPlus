#pragma once

#include <string>


namespace binance
{
	class Query
	{
	private:
		std::string queryString;
		bool markStart;


	public:
		Query(bool markStart = true) : markStart(markStart) {}

		std::string GetString() { return queryString; }
		const char* c_str() { return queryString.c_str(); }
		size_t length() { return queryString.length(); }

		void AppendQuery(std::string queryName, std::string value)
		{
			if (queryString.length() == 0 && markStart)
				queryString.append("?");
			else
				queryString.append("&");
			queryString.append(queryName).append("=").append(value);
		}
		void AppendQuery(std::string queryName, int64_t value)
		{
			AppendQuery(queryName, std::to_string(value));
		}
		void AppendQuery(std::string queryName, int value)
		{
			AppendQuery(queryName, std::to_string(value));
		}

		operator std::string () { return queryString; }
		operator const char* () { return queryString.c_str(); }
	};
}
