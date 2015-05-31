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
  ColorMap() { name = string("none"); }
  ColorMap(string fname) { loadfile(fname); }

  void loadfile(string fname)
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

	~ColorMap() {}

	vector<osp::vec3f> getColors() {return colors; }
	string getName() { return name; }

	void saveState(Document &doc)
	{
		Value cmap(kObjectType), fname(kObjectType);
		doc.AddMember("Colormap", cmap, doc.GetAllocator());
		fname.SetString(getName().c_str(), doc.GetAllocator());
		doc["Colormap"].AddMember("filename", fname, doc.GetAllocator());
	}
		

	void loadState(Document &doc)
	{
		if (! doc.HasMember("Colormap"))
		{
			std::cerr << "No colormap state\n";
		}
		else
		{
			loadfile(doc["Colormap"]["filename"].GetString());
		}
	}

	void commit(TransferFunction& t)
	{
    t.setColors(colors);
	}
	
	static vector< ColorMap > load_colormap_directory()
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

private:
	vector<osp::vec3f> colors;
	string name;
};
