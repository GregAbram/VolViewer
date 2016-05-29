#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ospray/ospray.h>

#include "common.h"

#include "Volume.h"

using namespace std;

class Slices {
public:

  Slices()
	{
		for (int i = 0; i < 3; i++)
		{
			values[i] = 0.0;
			flips[i]  = false;
			clips[i]  = false;
		}
	}

	void  SetValue(int i, int v)   { values[i] = v / 100.0; if (values[i] == 0.0) values[i] = -0.0001; else if (values[i] == 1.0) values[i] = 1.0001; }
	void  SetValue(int i, float v) { values[i] = v; if (values[i] == 0.0) values[i] = -0.0001; else if (values[i] == 1.0) values[i] = 1.0001; }
	float GetValue(int i) { return values[i]; }

	void  SetClip(int i, bool b)   { clips[i] = b; }
	bool  GetClip(int i)   { return clips[i]; }

	void  SetFlip(int i, bool b)   { flips[i] = b; }
	bool  GetFlip(int i)   { return flips[i]; }

	void  SetVisible(int i, bool b)   { visibility[i] = b; }
	bool  GetVisible(int i)   { return visibility[i]; }

	void loadState(Value &section)
	{
		int i = 0;
		for (Value::ConstValueIterator itr = section.Begin(); itr != section.End(); ++itr)
		{
			std::stringstream ss(itr->GetString());
			ss >> values[i] >> flips[i] >> visibility[i];
			i++;
		};
	}

  void saveState(Document &doc, Value &section)
	{
		Value a(kArrayType);

		for (int i = 0; i < 3; i++)
		{
			std::stringstream ss;
			ss << values[i] << " " << " " << flips[i] << " " << visibility[i];
			a.PushBack(Value().SetString(ss.str().c_str(), doc.GetAllocator()), doc.GetAllocator());
		}

		section.AddMember("Slices", a, doc.GetAllocator());
	}

	void commit(OSPRenderer& renderer, Volume *volume)
	{
		printf("slices commit\n");
		float planes[12];
		int   visible[3];
		int   clip[3];

		int xyz[3];
		volume->GetDimensions(xyz[0], xyz[1], xyz[2]);

		int k = 0;
		for (int i = 0; i < 3; i++)
			if (clips[i] || visibility[i])
			{
				if (flips[i])
				{
					planes[(k*4)+0] = (i == 0) ? -1.0 : 0.0;
					planes[(k*4)+1] = (i == 1) ? -1.0 : 0.0;
					planes[(k*4)+2] = (i == 2) ? -1.0 : 0.0;
					planes[(k*4)+3] = xyz[i]*values[i];
				}
				else
				{
					planes[(k*4)+0] = (i == 0) ? 1.0 : 0.0;
					planes[(k*4)+1] = (i == 1) ? 1.0 : 0.0;
					planes[(k*4)+2] = (i == 2) ? 1.0 : 0.0;
					planes[(k*4)+3] = -xyz[i]*values[i];
				}
				visible[k] = visibility[i];
				clip[k] = clips[i];
				// std::cerr << "Slice vis: " << visible[k] << " clip: " << clip[k] << " val: " << planes[3] << "\n";
				k++;
			}

		ospSetData(renderer, "nslices", ospNewData(1, OSP_INT, &k));
		if (k)
		{
			ospSetData(renderer, "slice planes", ospNewData(k, OSP_FLOAT4, planes));
			ospSetData(renderer, "slice visibility", ospNewData(k, OSP_INT, visible));
			ospSetData(renderer, "slice clips", ospNewData(k, OSP_INT, clip));
		}
  }

private:
	float clips[3];
	float values[3];
	int   onoffs[3];
	int 	flips[3];
	int 	visibility[3];
};
