#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <exception>
#include <dirent.h>
#include "ospray/ospray.h"
#include "TransferFunction.h"

using namespace std;

vector<osp::vec3f> 
load_colormap(string fname)
{
  vector<osp::vec3f> cmap;
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
    cmap.push_back(rgb);
  }

  if (ifs.eof())
    throw(std::string("invalid colormap file: ") + fname);

  ifs.close();
  return cmap;
}

map< string,  vector<osp::vec3f> >
load_colormap_directory()
{
  map< string, vector<osp::vec3f> > colormaps;
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
        colormaps[name] = load_colormap(name);
    }
  }

  return colormaps;
}
