#pragma once

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <dirent.h>
#include <map>
#include <vector>
#include "ospray/ospray.h"

#include "common.h"

#include "TransferFunction.h"

using namespace std;

class ColorMap
{
public:
  ColorMap();
  ColorMap(string fname);
	~ColorMap();

  void loadfile(string fname);
	vector<osp::vec3f> getColors();
	string getName();
	void saveState(Document &doc, Value &section);
	void loadState(Value &section);
	void commit(TransferFunction& t);
	
	static vector< ColorMap > load_colormap_directory();

private:
	vector<osp::vec3f> colors;
	string name;
};
