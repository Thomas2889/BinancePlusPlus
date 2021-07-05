#pragma once

#include <exception>


namespace binance
{
	class Binance;

	class BinanceException final : std::exception
	{
	public:
		Binance* instance;

		BinanceException(Binance* inst) { instance = inst; }
	};
}
