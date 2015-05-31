#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ospray/ospray.h>

#include "common.h"

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

	void loadState(Document &doc)
	{
		if (! doc.HasMember("Isosurfaces"))
		{
			std::cerr << "No isosurface state\n";
		}
		else
		{
			int j, i = 0;
			for (Value::ConstValueIterator itr = doc["Isosurfaces"].Begin(); itr != doc["Isosurfaces"].End(); ++itr)
			{
				std::stringstream ss(itr->GetString());
				ss >> values[i] >> j;
				onoffs[i] = (j == 1);
				i++;
				if (i > 3) break;
			}
			for ( ; i < 3; i++)
				onoffs[i] = false;
		}
	}

	void saveState(Document &doc)
	{
		Value a(kArrayType);

		for (int i = 0; i < 3; i++)
		{
			std::stringstream ss;
			ss << values[i] << " " << (onoffs[i] ? 1 : 0);
			a.PushBack(Value().SetString(ss.str().c_str(), doc.GetAllocator()), doc.GetAllocator());
		}
			
		doc.AddMember("Isosurfaces", a, doc.GetAllocator());
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

		vol->SetIsovalues(k, v);
  }

private:
	float values[3];
	bool onoffs[3];

	float min, max;
};
