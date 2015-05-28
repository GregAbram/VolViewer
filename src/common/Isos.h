#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <ospray/ospray.h>

#include "MyVolume.h"

using namespace std;

class Isos {
public:

  Isos()
	{
		for (int i = 0; i < 3; i++)
		{
			values[i] = 0.0;
			onoffs[i] = false;
		}

		min = 0.0;
		max = 1.0;
	}

	void  SetMinMax(float m, float M) { min = m; max = M; }

	void  SetValue(int i, int v)   { if (i > 2) i = 2; values[i] = v / 100.0; }
	void  SetValue(int i, float v)   { if (i > 2) i = 2; values[i] = v; }
	float GetValue(int i) { if (i > 2) i = 2; return values[i]; }

	void  SetOnOff(int i, bool b)   { if (i > 2) i = 2; onoffs[i] = b; }
	bool  GetOnOff(int i)   { if (i > 2) i = 2; return onoffs[i]; }

	void loadState(std::istream& in)
	{
		for (int i = 0; i < 3; i++)
			in >> values[i] >> onoffs[i];
	}

  void saveState(std::ostream& out)
	{
		for (int i = 0; i < 3; i++)
			out << values[i] << " " << onoffs[i] << "\n";
	}

	void commit(MyVolume *vol)
	{
		float v[3];

		int k = 0;
		for (int i = 0; i < 3; i++)
			if (onoffs[i])
			{
				v[k++]= min + values[i]*(max - min);
			}

		std::cerr << "max: " << max << " " << " min: " << min << " k: " << k << " value: " << v << "\n";
		vol->SetIsovalues(k, v);
  }

private:
	float values[3];
	float onoffs[3];

	float min, max;
};
