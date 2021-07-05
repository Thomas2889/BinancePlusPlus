#include <CppLog/Logger.h>

#include "Binance++/Binance++.h"


int main()
{
	new cpplog::Logger("log.txt", "Main", 4);

	cpplog::Logger::Destruct();

	return 0;
}