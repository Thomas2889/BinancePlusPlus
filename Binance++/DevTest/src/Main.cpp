#include <list>
#include <cmath>
#include <ctime>

#include <CppLog/Logger.h>
#include <talib/ta_func.h>

#include "Binance++/Binance++.h"

#include "Binance++/Internal/Utils.h"
#include "MA.h"
#include "MACD.h"

USING_LOGGER


struct PriceData
{
	double close;
	double high;
	double low;
};

int main()
{
	new Logger("log.txt", "Main", 4);

	binance::InitLibrary();


	binance::Binance bin;


	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	now = std::chrono::floor<std::chrono::hours>(now) - std::chrono::hours(1) - std::chrono::hours(6000); //7016 is max

	std::vector<double> primingData;
	
	{
		nlohmann::json json{};

		if (!bin.GetContinousContractKlineData("WAVESUSDT", "PERPETUAL", "1h", 0, now.time_since_epoch() / std::chrono::milliseconds(1), 948, json))
			Logger::Lock(Logger::ERROR) << bin.GetLastURLRequest() << "\n" << bin.GetErrorMessage() << Logger::endl;
		else
		{
			primingData.resize(json.size());
			for (int i = json.size() - 1; i >= 0; i--)
				primingData[i] = std::stod(json[i][4].get<std::string>());

			now += std::chrono::hours(1500);
		}
	} // with max have 6200 hours left, thats 4 * 1500 + 200


	SMA sma(240);
	sma.PrimeValue(&*(primingData.end() - 240), 240);
	SMA trendSMA(35);
	trendSMA.PrimeValue(&*(primingData.end() - 35), 35);

	EMA ema_super_short(10);
	ema_super_short.PrimeValue(&*(primingData.end() - 30), 30);
	EMA ema_short(20);
	ema_short.PrimeValue(&*(primingData.end() - 60), 60);
	EMA ema_long(72);
	ema_long.PrimeValue(&*(primingData.end() - 216), 216);

	MACD macd(120, 236, 36);
	std::vector<double> backlog = macd.PrimeValue(&*(primingData.end() - 948), 948);

	double money = 200;
	double reservedMoney = 0;
	double openPrice = 0;
	double openAmount = 0;
	double SL = 0;
	double SLSlope = 0;

	bool openLong = false;
	bool waitEmcCorrection = false;

	double crossPoint = 0;
	bool crossLong = false;
	int crosssPointTimeout = 0;

	struct
	{
		double price = 0;
		double sma = 0;
		double sma_short = 0;
		double ema_super_short = 0;
		double ema_short = 0;
		double ema_long = 0;
		double macd_diff = 0;
		double macd_avrg = 0;
		
		double crossThreshold = 0;
	} prev;
	prev.price = *--primingData.end();
	prev.sma = sma.Value();
	prev.ema_super_short = ema_super_short.Value();
	prev.ema_short = ema_short.Value();
	prev.ema_long = ema_long.Value();
	prev.macd_diff = macd.Diff();
	prev.crossThreshold = 0;


	for (int i = 0; i < 4; i++)
	{
		std::list<PriceData> data;
		std::list<double> trendSMAHistory;
		trendSMAHistory.resize(60);
		{
			nlohmann::json json{};

			if (!bin.GetContinousContractKlineData("WAVESUSDT", "PERPETUAL", "1h", 0, now.time_since_epoch() / std::chrono::milliseconds(1), 1500, json))
				Logger::Lock(Logger::ERROR) << bin.GetLastURLRequest() << "\n" << bin.GetErrorMessage() << Logger::endl;
			else
			{
				for (int i = json.size() - 1; i >= 0; i--)
				{
					PriceData newData;
					newData.high = std::stod(json[i][2].get<std::string>());
					newData.low = std::stod(json[i][3].get<std::string>());
					newData.close = std::stod(json[i][4].get<std::string>());
					data.push_front(newData);
				}

				now += std::chrono::hours(1500);
			}
		}

		auto it = data.begin();
		for (int i = 0; i < data.size(); i++)
		{
			std::chrono::system_clock::time_point currTime = now - std::chrono::hours(1500 + ((data.size() - 1) - i));

			double price = (*it).close;
			double price_high = (*it).high;
			double price_low = (*it).low;

			sma.FeedNewValue(price);
			trendSMA.FeedNewValue(price);
			ema_super_short.FeedNewValue(price);
			ema_short.FeedNewValue(price);
			ema_long.FeedNewValue(price);
			macd.FeedNewValue(price);

			it++;

			trendSMAHistory.push_front(trendSMA.Value());
			trendSMAHistory.pop_back();


			if (openPrice == 0)
			{
				bool longCross = prev.price < prev.sma&& price > sma.Value();
				bool shortCross = prev.price > prev.sma && price < sma.Value();
				if (crossPoint == 0)
				{
					if (longCross || shortCross)
					{
						if (longCross)
							crossLong = true;
						else
							crossLong = false;

						crossPoint = sma.Value();
						prev.crossThreshold = crossPoint;
						crosssPointTimeout = 20;
					}
				}
				else
				{
					longCross = price > sma.Value();
					shortCross = price < sma.Value();

					if ((crossLong && !longCross) || (!crossLong && !shortCross) || crosssPointTimeout == 0)
					{
						crossPoint = 0;
						crosssPointTimeout = 0;
						
					}
					else if ((abs(crossPoint - price) / crossPoint) > 0.05)
					{
						//crosssPointTimeout--;

						bool smaTrend = false; // (sma.Value() - prev.sma) > 0;
						bool smaShortTrend = false;
						bool macdTrend = false;
						bool emaSuperShortTrend = false;
						bool emaShortTrend = false;
						bool emaLongTrend = false;
						if (longCross)
						{
							smaTrend = (sma.Value() - prev.sma) > 0;
							macdTrend = macd.Diff() > 0;//&& macd.Macd() > (macd.Diff() * 0.0);
							emaSuperShortTrend = (ema_super_short.Value() - prev.ema_super_short) > 0;
							emaShortTrend = (ema_short.Value() - prev.ema_short) > 0;
							emaLongTrend = (ema_long.Value() - prev.ema_long) > 0;
						}
						else
						{
							smaTrend = (sma.Value() - prev.sma) < 0;
							macdTrend = macd.Diff() < 0;//&& macd.Macd() < (macd.Diff() * 0.0);
							emaSuperShortTrend = (ema_super_short.Value() - prev.ema_super_short) < 0;
							emaShortTrend = (ema_short.Value() - prev.ema_short) < 0;
							emaLongTrend = (ema_long.Value() - prev.ema_long) < 0;
						}

						//emaSuperShortTrend = true;

						if (smaTrend && macdTrend && emaSuperShortTrend && emaShortTrend && emaLongTrend)
						{
							openLong = longCross;
							if (longCross)
							{
								if (ema_short.Value() < ema_long.Value())
									waitEmcCorrection = true;
							}
							else
							{
								if (ema_short.Value() > ema_long.Value())
									waitEmcCorrection = true;
							}

							crossPoint = 0;
							double tradeAmount = money * 0.3;
							double amount = floor((tradeAmount / price) * 100) / 100;
							openPrice = price;
							openAmount = amount * 40;
							double oldWallet = money;
							if (longCross)
							{
								/*SL = price_low;
								
								auto backIt = it;
								auto trendIt = trendSMAHistory.end()--;
								
								double pointA = *trendIt;
								trendIt--; trendIt--;
								double pointB = *trendIt;
								int pointSpan = 2;
								double trendSlope = abs(pointB - pointA) / pointSpan;

								double lastMinimum

								while (true)
								{
									trendIt--;
									pointSpan++;

									pointB = *trendIt;
									double slope = *trendIt;

									if (abs(slope - trendSlope) > abs(trendSlope) * 0.3)
									{
										//end of trend, move back one
									}
								}*/

								money -= price * amount;
								std::time_t t = std::chrono::system_clock::to_time_t(currTime);
								Logger::Lock() << "Opened Long At: " << std::ctime(&t) <<
									"\nWallet: " << oldWallet << "  Price: " << price << "  PurchaseAmnt: " << amount << Logger::endl;
							}
							else
							{
								money -= price * amount;
								std::time_t t = std::chrono::system_clock::to_time_t(currTime);
								Logger::Lock() << "Opened Short At: " << std::ctime(&t) <<
									"\nWallet: " << oldWallet << "  Price: " << price << "  PurchaseAmnt: " << amount << Logger::endl;
							}
						}
					}
				}
			}
			else
			{
				if (openLong)
				{
					if (waitEmcCorrection && ema_short.Value() > ema_long.Value())
						waitEmcCorrection = false;

					if (false /*price < openSL*/)
					{
						double oldMoney = money;
						money += (SL - openPrice) * openAmount;

						if (money > oldMoney)
						{
							double diff = money - oldMoney;
							double saveAmnt = diff * 0.6;
							money -= saveAmnt;
							reservedMoney += saveAmnt;
						}

						std::time_t t = std::chrono::system_clock::to_time_t(currTime);
						Logger::Lock() << "Closed Long At: " << std::ctime(&t) <<
							"\nReasons:\n    price crossed over SL(" << SL << ")\n\n"
							"Wallet: " << money << "  Reserves: " << reservedMoney << "  Price: " << price << "  Trade Profit %: " << (SL - openPrice) / openPrice * 100 * 40 << "\n\n" << Logger::endl;
						openPrice = 0;
					}
					else if (price < sma.Value())
					{
						double oldMoney = money;
						money += (price - openPrice) * openAmount;

						if (money > oldMoney)
						{
							double diff = money - oldMoney;
							double saveAmnt = diff * 0.6;
							money -= saveAmnt;
							reservedMoney += saveAmnt;
						}

						std::time_t t = std::chrono::system_clock::to_time_t(currTime);
						Logger::Lock() << "Closed Long At: " << std::ctime(&t) <<
							"\nReasons:\n    price crossed back over sma(" << sma.Value() << ")\n\n"
							"Wallet: " << money << "  Reserves: " << reservedMoney << "  Price: " << price << "  Trade Profit %: " << (price - openPrice) / openPrice * 100 * 40 << "\n\n" << Logger::endl;
						openPrice = 0;
					}
					else if (ema_short.Value() < ema_long.Value() && !waitEmcCorrection)
					{
						double oldMoney = money;
						money += (price - openPrice) * openAmount;

						if (money > oldMoney)
						{
							double diff = money - oldMoney;
							double saveAmnt = diff * 0.6;
							money -= saveAmnt;
							reservedMoney += saveAmnt;
						}

						std::time_t t = std::chrono::system_clock::to_time_t(currTime);
						Logger::Lock() << "Closed Long At: " << std::ctime(&t) <<
							"Old Money vs New Money: " << oldMoney << ", " << money <<
							"\nReasons:\n    ema crossover(L:" << ema_long.Value() << ", S:" << ema_short.Value() << ")\n\n"
							"Wallet: " << money << "  Reserves: " << reservedMoney << "  Price: " << price << "  Trade Profit %: " << (price - openPrice) / openPrice * 100 * 40 << "\n\n" << Logger::endl;
						openPrice = 0;
					}
				}
				else
				{
					if (waitEmcCorrection && ema_short.Value() < ema_long.Value())
						waitEmcCorrection = false;

					if (false /*price > openSL*/)
					{
						double oldMoney = money;
						money += (openPrice - SL) * openAmount;

						if (money > oldMoney)
						{
							double diff = money - oldMoney;
							double saveAmnt = diff * 0.6;
							money -= saveAmnt;
							reservedMoney += saveAmnt;
						}

						std::time_t t = std::chrono::system_clock::to_time_t(currTime);
						Logger::Lock() << "Closed Short At: " << std::ctime(&t) <<
							"\nReasons:\n    price crossed over SL(" << SL << ")\n\n"
							"Wallet: " << money << "  Reserves: " << reservedMoney << "  Price: " << price << "  Trade Profit %: " << (openPrice - SL) / openPrice * 100 * 40 << "\n\n" << Logger::endl;
						openPrice = 0;
					}
					else if (price > sma.Value())
					{
						double oldMoney = money;
						money += (openPrice - price) * openAmount;

						if (money > oldMoney)
						{
							double diff = money - oldMoney;
							double saveAmnt = diff * 0.6;
							money -= saveAmnt;
							reservedMoney += saveAmnt;
						}

						std::time_t t = std::chrono::system_clock::to_time_t(currTime);
						Logger::Lock() << "Closed Short At: " << std::ctime(&t) <<
							"Old Money vs New Money: " << oldMoney << ", " << money <<
							"\nReasons:\n    price crossed back over sma(" << sma.Value() << ")\n\n"
							"Wallet: " << money << "  Reserves: " << reservedMoney << "  Price: " << price << "  Trade Profit %: " << (openPrice - price) / openPrice * 100 * 40 << "\n\n" << Logger::endl;
						openPrice = 0;
					}
					else if (ema_short.Value() > ema_long.Value() && !waitEmcCorrection)
					{
						double oldMoney = money;
						money += (openPrice - price) * openAmount;

						if (money > oldMoney)
						{
							double diff = money - oldMoney;
							double saveAmnt = diff * 0.6;
							money -= saveAmnt;
							reservedMoney += saveAmnt;
						}

						std::time_t t = std::chrono::system_clock::to_time_t(currTime);
						Logger::Lock() << "Closed Short At: " << std::ctime(&t) <<
							"Old Money vs New Money: " << oldMoney << ", " << money <<
							"\nReasons:\n    ema crossover(L:" << ema_long.Value() << ", S:" << ema_short.Value() << ")\n\n"
							"Wallet: " << money << "  Reserves: " << reservedMoney << "  Price: " << price << "  Trade Profit %: " << (openPrice - price) / openPrice * 100 * 40 << "\n\n" << Logger::endl;
						openPrice = 0;
					}
				}
			}


			prev.price = price;
			prev.sma = sma.Value();
			prev.ema_super_short = ema_super_short.Value();
			prev.ema_short = ema_short.Value();
			prev.ema_long = ema_long.Value();
			prev.macd_diff = macd.Diff();
			prev.crossThreshold = sma.Value() + (price - sma.Value() * 0.6);
		}
	}

	std::time_t t = std::chrono::system_clock::to_time_t(now - std::chrono::hours(1500));
	Logger::Lock() << "FINISH AT " << std::ctime(&t) << Logger::endl;
	Logger::Lock() << "Money: " << money << Logger::endl;


	/*auto it = primingData.end() - 10;
	for (int i = 0; i < 10; i++)
	{
		sma.FeedNewValue(*it);
		ema_short.FeedNewValue(*it);
		ema_long.FeedNewValue(*it);
		macd.FeedNewValue(*it);
		it++;

		Logger::Lock() << "MA: " << sma.Value() << Logger::endl;
		Logger::Lock() << "EMA Short: " << ema_short.Value() << Logger::endl;
		Logger::Lock() << "EMA Long: " << ema_long.Value() << Logger::endl;
		Logger::Lock() << "MACD: " << macd.Macd() << Logger::endl;
		Logger::Lock() << "MACD Signal: " << macd.Signal() << Logger::endl;
		Logger::Lock() << "MACD Diff: " << macd.Diff() << Logger::endl;
	}*/
	

	binance::CleanupLibrary();

	Logger::Destruct();

	return 0;
}