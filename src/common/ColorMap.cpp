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

#include "ColorMap.h"
#include "TransferFunction.h"

using namespace std;

ColorMap::ColorMap() { name = string("none"); }
ColorMap::ColorMap(string fname) { loadfile(fname); }
ColorMap::~ColorMap() {}

void
ColorMap::loadfile(string fname)
{
	name = fname;

	ifstream ifs;
	ifs.open(fname.c_str(), ios::in);
	if (ifs.fail())
		throw(std::string("error opening colormap file: ") + fname);

	int n;
	ifs >> n;

	for (int i = 0; i < n; i++)
	{
		float x;
		osp::vec3f rgb;
		ifs >> x >> rgb[0] >> rgb[1] >> rgb[2];
		colors.push_back(rgb);
	}

	if (ifs.eof())
		throw(std::string("invalid colormap file: ") + fname);

	ifs.close();
}

vector<osp::vec3f>
ColorMap::getColors()
{
	return colors;
}

string
ColorMap::getName()
{
	return name;
}

void
ColorMap::saveState(Document &doc, Value &section)
{
	Value cmap(kObjectType), fname(kObjectType);
	fname.SetString(getName().c_str(), doc.GetAllocator());
	cmap.AddMember("filename", fname, doc.GetAllocator());
	section.AddMember("Colormap", cmap, doc.GetAllocator());
}

void
ColorMap::loadState(Value &section)
{
	loadfile(section["filename"].GetString());
}

void
ColorMap::commit(TransferFunction& t)
{
	t.setColors(colors);
}

vector< ColorMap >

ColorMap::load_colormap_directory()
{
	vector< ColorMap > colormaps;

	string foo = getenv("COLORMAP_DIR") ? getenv("COLORMAP_DIR") :
								getenv("HOME") ? string(getenv("HOME")) + "/colormaps"  :
								"";

	DIR *dir;
	if ((dir = opendir(foo.c_str())) != NULL)
	{
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
		{
			string name(foo + "/" + ent->d_name);

			int p = name.find_last_of('.');
			if ((p != string::npos) && (p != (name.length()-1)) && (name.substr(p) == ".cmap"))
				colormaps.push_back(ColorMap(name));
		}
	}

	return colormaps;
}
