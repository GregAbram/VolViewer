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

class Isos {
public:

  Isos();
	void  SetMinMax(float m, float M);
	void  SetValue(int i, int v);
	void  SetValue(int i, float v);
	float GetValue(int i);
	void  SetOnOff(int i, bool b);
	bool  GetOnOff(int i);
	void loadState(Value &section);
	void saveState(Document &doc, Value &section);
	void commit(Volume *vol);

private:
	float values[3];
	bool onoffs[3];

	float min, max;
};
