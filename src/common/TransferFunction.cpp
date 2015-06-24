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

void
TransferFunction::commit(OSPRenderer& r)
{
	ospSet2f(tf, "valueRange", minv, maxv);

	float xmin = alphas.front().x;
	float xmax = alphas.back().x;

	vector<float> interpolated;

	int i0 = 0, i1 = 1;
	for (int i = 0; i < 256; i++)
	{
		float x = xmin + (i / (255.0))*(xmax - xmin);
		if (x > xmax) x = xmax;
		while (alphas[i1].x < x)
			i0++, i1++;
		float d = (x - alphas[i0].x) / (alphas[i1].x - alphas[i0].x);
		interpolated.push_back(scale * (alphas[i0].y + d*(alphas[i1].y - alphas[i0].y)));
	}

	OSPData oAlphas = ospNewData(interpolated.size(), OSP_FLOAT, interpolated.data());
	ospSetData(tf, "opacities", oAlphas);

	OSPData oColors = ospNewData(colors.size(), OSP_FLOAT3, colors.data());
	ospSetData(tf, "colors", oColors);


	ospSet1i(r, "doVolumeRendering", doVolumeRendering ? 1 : 0);

	ospCommit(tf);
}


