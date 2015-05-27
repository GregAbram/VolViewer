#pragma once

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <exception>
#include <dirent.h>
#include <map>
#include <vector>
#include "ospray/ospray.h"

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

	void saveState(std::ostream& out)
	{
		out << getName() << "\n";
	}

	void loadState(std::istream& in)
	{
		in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		string name;
		in >> name;
		loadfile(name);
		in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
