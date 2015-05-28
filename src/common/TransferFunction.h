#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <ospray/ospray.h>

using namespace std;

class TransferFunction {
public:

  TransferFunction() :
		minv(0.0),
		maxv(1.0),
		scale(1.0),
		doVolumeRendering(true)
	{
		alphas.push_back(osp::vec2f(0.0, 0.0));
		alphas.push_back(osp::vec2f(1.0, 1.0));

		tf = ospNewTransferFunction("piecewise_linear");
	}

	OSPTransferFunction& getOSPTransferFunction() { return tf; }

	void SetMin(float m)   { minv  = m; }
	void SetMax(float m)   { maxv  = m; }
	void SetScale(float s) { scale = s; }
	
	float GetMin()   { return minv;  }
	float GetMax()   { return maxv;  }
	float GetScale() { return scale; }

	void SetDoVolumeRendering(bool yesNo) { doVolumeRendering = yesNo; }

	void SetAlphas(vector<osp::vec2f> a) { alphas = a; }
	vector<osp::vec2f> GetAlphas() { return alphas; }

	void loadState(std::istream& in)
	{
		in >> scale;

		alphas.clear();

		int n;
		in >> n;
		for (int i = 0; i < n; i++)
		{
			float x, a;
			in >> x >> a;
			alphas.push_back(osp::vec2f(x, a));
		}
	}

  void saveState(std::ostream& out)
	{
		out << scale << "\n";
		out << alphas.size() << "\n";
		for (int i = 0; i < alphas.size(); i++)
			out << alphas[i].x << " " << alphas[i].y << "\n";
	}
	int foo;

	void commit(OSPRenderer& r)
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

	void setColors(vector<osp::vec3f> c) 
	{ 
		colors = c; 
  }

	void showColors()
	{
		std::cerr << colors.size() << "\n";
		for (vector<osp::vec3f>::iterator it = colors.begin(); it != colors.end(); ++it)
			std::cerr << it->x << " " << it->y << " " << it->z << "\n";
	}

private:
	float       				minv, maxv, scale;
	bool								doVolumeRendering;
	vector<osp::vec3f>  colors;
	vector<osp::vec2f> 	alphas;
	OSPTransferFunction tf;
};

vector<osp::vec3f> load_colormap(string fname);
map< string,  vector<osp::vec3f> > load_colormap_directory();
