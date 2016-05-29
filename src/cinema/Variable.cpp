#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "Variable.h"
#include "Renderer.h"
#include "util.h"

using namespace std;
using namespace rapidjson;

VariableFactory::VariableFactory()
{
	Register("Isosurface", IsosurfaceVariable::New);
	Register("Slice", SlicePlaneVariable::New);
	Register("Camera", CameraVariable::New);
	Register("VolumeRendering", VolumeRenderingVariable::New);
}

void
VariableFactory::Register(const char *c, Variable *(*func)(Value&))
{
	VariableCtor v(string(c), func);
	registry.push_back(v);
}

Variable *
VariableFactory::Create(Value &json)
{
	if (json.HasMember("operator"))
	{
		string op(json["operator"].GetString());

		int i;
		for (i = 0; i < registry.size(); i++)
		{
			std::cerr << "trying: " << registry[i].name << "\n";
			if (registry[i].name == op) break;
		}

		if (i == registry.size())
		{
			std::cerr << "bad operator: " << op.c_str() << "\n";
			exit(1);
		}

		Variable *v = registry[i].func(json);

		if (json.HasMember("child"))
			v->setDown(Create(json["child"]));
		else
			v->setDown(NULL);

		return v;
	}
	
	std::cerr << "Error: bad Cinema json\n";
	std::exit(1);
}

VariableFactory theVariableFactory;

//===================================================

Variable::Variable(string op) :  name(op), down(NULL)
{
}

void  Variable::ReInitialize(Renderer& r)
{
	down->ReInitialize(r);
}

void Variable::RenderDown(Renderer& r, string s, Document& doc)
{
	down->Render(r, s, doc);
}

//===================================================

Variable *
SlicePlaneVariable::New(Value &json)
{
	vector<int>axes;
	if (json.HasMember("axes") && json["axes"].IsArray())
		for (int i = 0; i <  json["axes"].Size(); i++)
			axes.push_back(json["axes"][i].GetInt());
	else
		axes.push_back(0);

	vector<int>clips;
	if (json.HasMember("clips") && json["clips"].IsArray())
		for (int i = 0; i <  json["clips"].Size(); i++)
			clips.push_back(json["clips"][i].GetInt());
	else
		clips.push_back(0);

	vector<int>flips;
	if (json.HasMember("flips") && json["flips"].IsArray())
		for (int i = 0; i <  json["flips"].Size(); i++)
			flips.push_back(json["flips"][i].GetInt());
	else
		flips.push_back(0);

	vector<int>visibles;
	if (json.HasMember("visibility") && json["visibility"].IsArray())
		for (int i = 0; i <  json["visibility"].Size(); i++)
			visibles.push_back(json["visibility"][i].GetInt());
	else
		visibles.push_back(0);

	vector<int>values;
	if (json.HasMember("values") && json["values"].IsArray())
		for (int i = 0; i <  json["values"].Size(); i++)
			values.push_back(json["values"][i].GetInt());
	else
		values.push_back(50);

	return (Variable *) new SlicePlaneVariable(string("Slices"), axes, clips, visibles, flips, values);
}

SlicePlaneVariable::SlicePlaneVariable(string name, vector<int>a, vector<int> c, vector<int> v, vector<int> f, vector<int> vals) : Variable(name)
{
	axes  	 = a;
	clips  	 = c;
	visibles = v;
	flips    = f;
	values   = vals;
}

void  SlicePlaneVariable::Render(Renderer& r, string s, Document& doc)
{
	for (vector<int>::iterator ai = axes.begin(); ai != axes.end(); ai++)
	{
		int axis = *ai;

		string s1;
		if (axes.size() > 1)
		{
			char buf[256];
			sprintf(buf, "%s_%d", s.c_str(), axis);
			s1 = string(buf);
			SetIntAttr(doc, (name + "Axis").c_str(), axis);
		}
		else
			s1 = s;

		for (vector<int>::iterator ci = clips.begin(); ci != clips.end(); ci++)
		{
			int clip = *ci;
			r.getSlices().SetClip(axis, clip);

			string s2;
			if (clips.size() > 1)
			{
				char buf[256];
				sprintf(buf, "%s_%d", s1.c_str(), clip);
				s2 = string(buf);
				SetIntAttr(doc, (name + "Clip").c_str(), clip);
			}
			else
				s2 = s1;

			for (vector<int>::iterator vi = visibles.begin(); vi != visibles.end(); vi++)
			{
				int visibility = *vi;
				r.getSlices().SetVisible(axis, visibility);

				string s3;
				if (visibles.size() > 1)
				{
					char buf[256];
					sprintf(buf, "%s_%d", s2.c_str(), visibility);
					s3 = string(buf);
					SetIntAttr(doc, (name + "Visibility").c_str(), visibility);
				}
				else
					s3 = s2;

				for (vector<int>::iterator fi = flips.begin(); fi != flips.end(); fi++)
				{
					int flip = *fi;
					r.getSlices().SetFlip(axis, flip == 1);

					string s4;
					if (flips.size() > 1)
					{
						char buf[256];
						sprintf(buf, "%s_%d", s3.c_str(), flip);
						s4 = string(buf);
						SetIntAttr(doc, (name + "Flip").c_str(), flip);
					}
					else
						s4 = s3;

					for (vector<int>::iterator vi = values.begin(); vi != values.end(); vi++)
					{
						int value = *vi;
						r.getSlices().SetValue(axis, value);

						string s5;
						if (values.size() > 1)
						{
							char buf[256];
							sprintf(buf, "%s_%d", s4.c_str(), value);
							s5 = string(buf);
							SetIntAttr(doc, (name + "Value").c_str(), value);
						}
						else
							s5 = s4;

						RenderDown(r, s5, doc);
					}
				}
			}
		}
	}
}

string SlicePlaneVariable::GatherTemplate(string s, Document &doc)
{
	if (axes.size() > 1)
	{
		s = s + "_{" + name + "Axis}";
		AddIntRangeArg(doc, name + "Axis", axes);
	}

	if (clips.size() > 1)
	{
		s = s + "_{" + name + "Clip}";
		AddIntRangeArg(doc, name + "Clip", clips);
	}

	if (visibles.size() > 1)
	{
		s = s + "_{" + name + "Visiblity}";
		AddIntRangeArg(doc, name + "Visiblity", visibles);
	}

	if (flips.size() > 1)
	{
		s = s + "_{" + name + "Flip}";
		AddIntRangeArg(doc, name + "Flip", flips);
	}

	if (values.size() > 1)
	{
		s = s + "_{" + name + "Value}";
		AddIntRangeArg(doc, name + "Value", values);
	}
	
	return down->GatherTemplate(s, doc);
}

//===================================================

Variable *
IsosurfaceVariable::New(Value &json)
{
	int index;
	vector<int>values;

	if (!json.HasMember("index") || !json["index"].IsInt() || !json.HasMember("values") || !json["values"].IsArray())
	{
		std::cerr << "bad isosurface json\n";
		exit(1);
	}

	index = json["index"].GetInt();

	for (int i = 0; i <  json["values"].Size(); i++)
		values.push_back(json["values"][i].GetInt());

	return (Variable *) new IsosurfaceVariable(string("Isosurface"), index, values);
}

IsosurfaceVariable::IsosurfaceVariable(string n, int i, vector<int> v) : Variable(n)
{
	values  = v;
	index = i;
}

void IsosurfaceVariable::ReInitialize(Renderer& r)
{
	float min, max;
	r.getVolume()->GetMinMax(min, max);
  r.getIsos().SetMinMax(min, max);

	Variable::ReInitialize(r);
}

void IsosurfaceVariable::Render(Renderer& r, string s, Document& doc)
{
	Isos& isos = r.getIsos();
	isos.SetOnOff(index, true);

	for (int i = 0; i < values.size(); i++)
	{
		float v = values[i] / 99.0;
		isos.SetValue(index, v);
		isos.commit(r.getVolume());

		if (values.size() > 1)
		{
			char buf[256];
			sprintf(buf, "%s_%d", s.c_str(), values[i]);
			SetDoubleAttr(doc, (name + "Value").c_str(), values[i]);
			RenderDown(r, string(buf), doc);
		}
		else
		{
			RenderDown(r, s, doc);
		}
	}
}

string IsosurfaceVariable::GatherTemplate(string s, Document& doc)
{
	if (values.size() > 1)
	{
		s = s + "_{" + name + "Value}";
		AddIntRangeArg(doc, name + "Value", values);
	}

	return down->GatherTemplate(s, doc);
}

//===================================================

Variable *
CameraVariable::New(Value &json)
{
	vector<int>phi;
	if (json.HasMember("phi") && json["phi"].IsArray())
		for (int i = 0; i <  json["phi"].Size(); i++)
			phi.push_back(json["phi"][i].GetInt());
	else
		phi.push_back(50);

	vector<int>theta;
	if (json.HasMember("theta") && json["theta"].IsArray())
		for (int i = 0; i <  json["theta"].Size(); i++)
			theta.push_back(json["theta"][i].GetInt());
	else
		theta.push_back(50);

	return (Variable *) new CameraVariable(phi, theta);
}

CameraVariable::CameraVariable(vector<int> p, vector<int> t) : Variable(string("Camera")) 
{
	knt    = -1;
	phis   = p;
	thetas = t;
}

void CameraVariable::Render(Renderer& r, string s, Document& doc)
{
	Camera& camera = r.getCamera();
	string s1, s2;

	r.getVolume()->commit();

	for (int i = 0; i < thetas.size(); i++)
	{
		camera.setTheta(thetas[i]);

		if (thetas.size() > 1)
		{
			char buf[256];
			sprintf(buf, "%s_%d", s.c_str(), thetas[i]);
			s1 = string(buf);
			SetIntAttr(doc, "theta", thetas[i]);
		}
		else
			s1 = s;

		for (int j = 0; j < phis.size(); j++)
		{
			camera.setPhi(phis[j]);

			if (phis.size() > 1)
			{
				char buf[256];
				sprintf(buf, "%s_%d", s1.c_str(), phis[j]);
				s2 = string(buf);
				SetIntAttr(doc, "phi", phis[j]);
			}
			else
				s2 = s1;

			RenderShot(r, s2, doc);
		}
	}
}

void CameraVariable::RenderShot(Renderer& r, string s, Document& doc)
{
	struct stat info;

	if (stat((s + ".png").c_str(), &info))
	{
		r.getSlices().commit(r.getRenderer(), r.getVolume());
		r.getIsos().commit(r.getVolume());
		r.getCamera().commit();
		r.Render(s + ".png");

		StringBuffer sbuf;
		PrettyWriter<StringBuffer> writer(sbuf);
		doc.Accept(writer);
		
		ofstream out;
		out.open((s + ".__data__").c_str(), ofstream::out);
		out << sbuf.GetString() << "\n";
		out.close();

		if (knt != -1)
			std::cerr << knt++ <<  " (" << (s + ".png").c_str() << ") done\n";
	}
	else if (knt != -1)
			std::cerr << knt++ << " (" << (s + ".png").c_str() << ") skipped\n";
}

string CameraVariable::GatherTemplate(string s, Document& doc)
{
	if (thetas.size() > 1)
	{
		AddIntRangeArg(doc, string("Theta"), thetas);
		s = s + "_{Theta}";
	}

	if (phis.size() > 1)
	{
		AddIntRangeArg(doc, string("Phi"), phis);
		s = s + "_{Phi}";
	}

	return s;
}

//===================================================

Variable *
VolumeRenderingVariable::New(Value& json)
{
	vector<int>onoff;
	if (json.HasMember("onoff") && json["onoff"].IsArray())
		for (int i = 0; i <  json["onoff"].Size(); i++)
			onoff.push_back(json["onoff"][i].GetInt());
	else
		onoff.push_back(0);

	return (Variable *) new VolumeRenderingVariable(string("VolumeRendering"), onoff);

}

VolumeRenderingVariable::VolumeRenderingVariable(string name, vector<int> o) : Variable(name)
{
  onOff = o;
}

void VolumeRenderingVariable::Render(Renderer& r, string s, Document& doc)
{
	TransferFunction& tf = r.getTransferFunction();
	string s1;

	for (int i = 0; i < onOff.size(); i++)
	{
		tf.SetDoVolumeRendering(onOff[i] == 1);

		if (onOff.size() > 1)
		{
			char buf[256];
			sprintf(buf, "%s_%d", s.c_str(), onOff[i]);
			s1 = string(buf);
			SetIntAttr(doc, "VolumeRendering", onOff[i]);
		}
		else
			s1 = s;

		RenderDown(r, s1, doc);
	}
}

string VolumeRenderingVariable::GatherTemplate(string s, Document& doc)
{
	if (onOff.size() > 1)
	{
		AddIntRangeArg(doc, string("VolumeRendering"), onOff);
		s = s + "_{VolumeRendering}";
	}

	return down->GatherTemplate(s, doc);
}
