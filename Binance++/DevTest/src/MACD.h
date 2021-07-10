#pragma once

#include <vector>

#include "MA.h"


class MACD
{
private:
	double v_macd = 0;
	EMA signal = 0;
	double diff = 0;
	EMA shortEMA;
	EMA longEMA;

	bool primed = false;

public:
	MACD(int shortPeriod, int longPeriod, int signalPeriod) : signal(signalPeriod), shortEMA(shortPeriod), longEMA(longPeriod) {}


	std::vector<double> PrimeValue(double* primingData, int primingSize)
	{
		if (primingSize < (longEMA.Period() * 3 + signal.Period() * 3))
			throw std::exception("Need more priming data.");

		longEMA.PrimeValue(primingData, longEMA.Period() * 3);
		shortEMA.PrimeValue(primingData + (((longEMA.Period() * 3) + (signal.Period() * 3)) - ((signal.Period() * 3) + (shortEMA.Period() * 3))), shortEMA.Period() * 3);

		std::vector<double> macdBacklog;
		macdBacklog.resize(primingSize - (longEMA.Period() * 3));
		for (int i = 0; i < primingSize - (longEMA.Period() * 3); i++)
		{
			longEMA.FeedNewValue(primingData[(longEMA.Period() * 3) + i]);
			shortEMA.FeedNewValue(primingData[(longEMA.Period() * 3) + i]);
			macdBacklog[i] = shortEMA.Value() - longEMA.Value();
		}
		v_macd = *(macdBacklog.end() - 1);

		signal.PrimeValue(macdBacklog.data(), signal.Period() * 3);
		diff = v_macd - signal.Value();

		primed = true;
		return macdBacklog;
	}
	void FeedNewValue(double newVal)
	{
		if (!primed)
			throw std::exception("Need to prime first.");

		longEMA.FeedNewValue(newVal);
		shortEMA.FeedNewValue(newVal);
		v_macd = shortEMA.Value() - longEMA.Value();
		signal.FeedNewValue(v_macd);
		diff = v_macd - signal;
	}

	double Macd() { return v_macd; }
	double Signal() { return signal.Value(); }
	double Diff() { return diff; }
	double ShortEMA() { return shortEMA.Value(); }
	double LongEMA() { return longEMA.Value(); }

	int ShortPeriod() { return shortEMA.Period(); }
	int LongPeriod() { return longEMA.Period(); }
	int SignalPeriod() { return signal.Period(); }
};
