#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ospray/ospray.h>

#include "common.h"

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

	void loadState(Value &section)
	{
		alphas.clear();

		for (Value::ConstValueIterator itr = section["Opacity"].Begin(); itr != section["Opacity"].End(); ++itr)
		{
			float x, a;
			std::stringstream ss(itr->GetString());
			ss >> x >> a;
			alphas.push_back(osp::vec2f(x, a));
		}

		SetScale((float)section["Scale"].GetDouble());
		SetMin((float)section["Min"].GetDouble());
		SetMax((float)section["Max"].GetDouble());
	}

  void saveState(Document &doc, Value &section)
	{
		Value tf(kObjectType);

		Value a(kArrayType);

		for (int i = 0; i < alphas.size(); i++)
		{
			std::stringstream ss;
			ss << alphas[i].x << " " << alphas[i].y;
			a.PushBack(Value().SetString(ss.str().c_str(), doc.GetAllocator()), doc.GetAllocator());
		}

		tf.AddMember("Opacity", a, doc.GetAllocator());
		tf.AddMember("Scale", Value().SetDouble((double)GetScale()), doc.GetAllocator());
		tf.AddMember("Min", Value().SetDouble((double)GetMin()), doc.GetAllocator());
		tf.AddMember("Max", Value().SetDouble((double)GetMax()), doc.GetAllocator());

		section.AddMember("TransferFunction", tf, doc.GetAllocator());
	}

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
