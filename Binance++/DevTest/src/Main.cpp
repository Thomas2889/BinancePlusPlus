#include <CppLog/Logger.h>

#include "Binance++/Binance++.h"


void Test(nlohmann::json& tmp)
{

}

int main()
{
	new cpplog::Logger("log.txt", "Main", 4);

	binance::InitLibrary();


	binance::Binance bin;

	nlohmann::json json{};
	if (!bin.GetContinousContractKlineData("WAVESUSDT", "PERPETUAL", "1m", 0, 0, 4, json))
		cpplog::Logger::Lock(cpplog::Logger::ERROR) << bin.GetLastURLRequest() << "\n" << bin.GetErrorMessage() << cpplog::Logger::endl;
	else
	{
		cpplog::Logger::Log(json[0][4].get<std::string>());
		cpplog::Logger::Log(json[1][4].get<std::string>());
		cpplog::Logger::Log(json[2][4].get<std::string>());
		cpplog::Logger::Log(json[3][4].get<std::string>());
	}

	binance::CleanupLibrary();

	cpplog::Logger::Destruct();

	return 0;
}