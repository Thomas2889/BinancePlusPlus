#pragma once

#include <list>


class SMA
{
private:
	double value = 0;
	int period;
	std::list<double> data;

	bool primed = false;

public:
	SMA(int period) : period(period)
	{
		data.resize(period);
	}

	void PrimeValue(double* primingData, int primingSize)
	{
		if (primingSize < period)
			throw std::exception("Need more priming data.");
		if (primingSize > period)
		{
			int diff = primingSize - period;
			int i = 0;
			for (; i < diff; i++)
			{
				value += primingData[i] / period;
			}
			auto it = data.begin();
			for (; i < primingSize; i++)
			{
				*it++ = primingData[i];
				value += primingData[i] / period;
			}
		}
		else
		{
			auto it = data.begin();
			for (int i = 0; i < primingSize; i++)
			{
				value += primingData[i] / period;
				*it++ = primingData[i];
			}
		}

		primed = true;
	}
	void FeedNewValue(double newVal)
	{
		if (!primed)
			throw std::exception("Need to prime first.");

		value -= *data.begin() / period;
		value += newVal / period;

		data.pop_front();
		data.push_back(newVal);
	}

	operator double() { return value; }

	double Value() { return value; }
	int Period() { return period; }
	std::list<double> Data() { return data; }
};

class EMA
{
private:
	double value = 0;
	int period;

	bool primed = false;

public:
	EMA(int period) : period(period) {}

	void PrimeValue(double* primingData, int primingSize)
	{
		if (primingSize < period * 3)
			throw std::exception("Need more priming data.");
		
		int i = 0;
		for (; i < period; i++)
		{
			value += primingData[i] / period;
		}
		for (; i < primingSize; i++)
		{
			value = ((primingData[i] - value) * (2.0 / (period + 1))) + value;
		}

		primed = true;
	}
	void FeedNewValue(double newVal)
	{
		if (!primed)
			throw std::exception("Need to prime first.");

		value = ((newVal - value) * (2.0 / (period + 1))) + value;
	}

	operator double() { return value; }

	double Value() { return value; }
	int Period() { return period; }
};
