#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "Cinema.h"

using namespace std;
using namespace rapidjson;

static void
AddIntRangeArg(Document& doc, string name, vector<int> values)
{
  Value::MemberIterator m = doc["arguments"].FindMember(name.c_str());
  if (m != doc["arguments"].MemberEnd())
    doc["arguments"].RemoveMember(name.c_str());

	Value arg(kObjectType);
	Value ov(kObjectType), av(kArrayType);

	ov.SetInt(values[0]);
	arg.AddMember("default", ov, doc.GetAllocator());

	av.Clear();
	for (int i = 0; i < values.size(); i++)
	{
		Value v(values[i]);
		av.PushBack(v, doc.GetAllocator());
	}
	arg.AddMember("values", av, doc.GetAllocator());

	ov.SetString("range", doc.GetAllocator());
	arg.AddMember("type", ov, doc.GetAllocator());
	
	ov.SetString(name.c_str(), doc.GetAllocator());
	arg.AddMember("label", ov, doc.GetAllocator());

	ov.SetString(name.c_str(), doc.GetAllocator());
	doc["arguments"].AddMember(ov, arg, doc.GetAllocator());
}

static void
AddBoolRangeArg(Document& doc, string name, vector<bool> values)
{
  Value::MemberIterator m = doc["arguments"].FindMember(name.c_str());
  if (m != doc["arguments"].MemberEnd())
    doc["arguments"].RemoveMember(name.c_str());

	Value arg(kObjectType);
	Value ov(kObjectType), av(kArrayType);

	ov.SetInt(values[0]);
	arg.AddMember("default", ov, doc.GetAllocator());

	av.Clear();
	for (int i = 0; i < values.size(); i++)
	{
		Value v(values[i] ? 1 : 0);
		av.PushBack(v, doc.GetAllocator());
	}
	arg.AddMember("values", av, doc.GetAllocator());

	ov.SetString("range", doc.GetAllocator());
	arg.AddMember("type", ov, doc.GetAllocator());
	
	ov.SetString(name.c_str(), doc.GetAllocator());
	arg.AddMember("label", ov, doc.GetAllocator());

	ov.SetString(name.c_str(), doc.GetAllocator());
	doc["arguments"].AddMember(ov, arg, doc.GetAllocator());
}

static void
SetIntAttr(Document& doc, string name, int val)
{
  Value::MemberIterator m = doc["descriptor"].FindMember(name.c_str());
  if (m != doc["descriptor"].MemberEnd())
    doc["descriptor"].RemoveMember(name.c_str());

	Value v(kObjectType);
	v.SetInt(val);
	Value n; n.SetString(name.c_str(), doc.GetAllocator());
	doc["descriptor"].AddMember(n, v, doc.GetAllocator());
}

static void
SetDoubleAttr(Document& doc, string name, double val)
{
  Value::MemberIterator m = doc["descriptor"].FindMember(name.c_str());
  if (m != doc["descriptor"].MemberEnd())
    doc["descriptor"].RemoveMember(name.c_str());

	Value v(kObjectType);
	v.SetDouble(val);
	Value n; n.SetString(name.c_str(), doc.GetAllocator());
	doc["descriptor"].AddMember(n, v, doc.GetAllocator());
}

//===================================================

Variable::Variable(string n) : name(n), down(NULL) {}

void  Variable::ReInitialize(Renderer& r)
{
	down->ReInitialize(r);
}

void Variable::RenderDown(Renderer& r, string s, Document& doc)
{
	down->Render(r, s, doc);
}

//===================================================

SlicePlaneVariable::SlicePlaneVariable(string n, vector<int> c, vector<int> v, vector<int> f, vector<int> vals) : Variable(n)
{
	for (int i = 0; i < strlen(n.c_str()); i++)
	{
		int c = n.c_str()[i];
		if (c == 'X')
			axes.push_back(0);
		else if (c == 'Y')
			axes.push_back(1);
		else 
			axes.push_back(2);
	}

	clip  	= c;
	visible = v;
	flip    = f;
	values  = vals;
	for (int i = 0; i < values.size(); i++)
		std::cerr << "SPV: " << i << " " << values[i] << "\n";
}

SlicePlaneVariable::SlicePlaneVariable(string n, vector<int> v, vector<int> f, vector<int> vals) : Variable(n)
{
	for (int i = 0; i < strlen(n.c_str()); i++)
	{
		int c = n.c_str()[i];
		if (c == 'X')
			axes.push_back(0);
		else if (c == 'Y')
			axes.push_back(1);
		else 
			axes.push_back(2);
	}

	visible = v;
	flip    = f;
	values  = vals;
}

void  SlicePlaneVariable::Render(Renderer& r, string s, Document& doc)
{
	for (int i = 0; i < clip.size(); i++)
	{
		string s1;

		std::cerr << "CLIP SET TO " << clip[i] << "\n";
		for (int i = 0; i < axes.size(); i++)
			r.getSlices().SetClip(axes[i], clip[i]);

		if (clip.size() > 1)
		{
			char buf[256];
			sprintf(buf, "%s_%d", s.c_str(), clip[i]);
			s1 = string(buf);
			SetIntAttr(doc, (name + "Slice").c_str(), clip[i]);
		}
		else
			s1 = s;

		for (int j = 0; j < visible.size(); j++)
		{
			string s2;

			for (int i = 0; i < axes.size(); i++)
				r.getSlices().SetVisible(axes[i], visible[j]);
			std::cerr << "VISIBLE SET TO " << visible[j] << "\n";

			if (visible.size() > 1)
			{
				char buf[256];
				sprintf(buf, "%s_%d", s1.c_str(), visible[j]);
				s2 = string(buf);
				SetIntAttr(doc, (name + "Visible").c_str(), visible[j]); 
			}
			else 
				s2 = s1;

			for (int k = 0; k < flip.size(); k++)
			{
				string s3;

				for (int i = 0; i < axes.size(); i++)
					r.getSlices().SetFlip(axes[i], flip[k] == 1);

				std::cerr << "FLIP SET TO " << flip[k] << "\n";

				if (flip.size() > 1)
				{
					char buf[256];
					sprintf(buf, "%s_%d", s2.c_str(), flip[k]);
					s3 = string(buf);
					SetIntAttr(doc, (name + "Flip").c_str(), flip[k]); 
				}
				else 
					s3 = s2;

				for (int l = 0; l < values.size(); l++)
				{
					string s4;

					for (int i = 0; i < axes.size(); i++)
						r.getSlices().SetValue(axes[i], values[l]);

					std::cerr << "VALUE SET TO " << values[l] << "\n";

					if (values.size() > 1)
					{
						char buf[256];
						sprintf(buf, "%s_%d", s3.c_str(), values[l]);
						s4 = string(buf);
						SetIntAttr(doc, (name + "Value").c_str(), values[l]);
					}
					else 
						s4 = s3;
	
					RenderDown(r, s4, doc);
				}
			}
		}
	}
}

string SlicePlaneVariable::GatherTemplate(string s, Document &doc)
{
	if (clip.size() > 1)
	{
		s = s + "_{" + name + "Clip}";
		AddIntRangeArg(doc, name + "Clip", clip);
	}

	if (visible.size() > 1)
	{
		s = s + "_{" + name + "Visible}";
		AddIntRangeArg(doc, name + "Visible", visible);
	}

	if (flip.size() > 1)
	{
		s = s + "_{" + name + "Flip}";
		AddIntRangeArg(doc, name + "Flip", flip);
	}

	if (values.size() > 1)
	{
		s = s + "_{" + name + "Value}";
		AddIntRangeArg(doc, name + "Value", values);
	}
	
	return down->GatherTemplate(s, doc);
}

//===================================================

IsosurfaceVariable::IsosurfaceVariable(string n, vector<int> v) : Variable(n)
{
	values  = v;
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
	isos.SetOnOff(0, true);

	for (int i = 0; i < values.size(); i++)
	{
		float v = values[i] / 99.0;
		isos.SetValue(0, v);
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

CameraVariable::CameraVariable(vector<int> p, vector<int> t) : Variable(string(""))
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
		// r.getTransferFunction().commit(r.getRenderer());
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

VolumeRenderingVariable::VolumeRenderingVariable(vector<int> o) : Variable(string("VolumeRendering"))
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

//===================================================

Cinema::Cinema(int *argc, const char **argv)
{
  ospInit(argc, (const char **)argv);
	variableStack = NULL;
}

Cinema::~Cinema()
{
	Variable *vn;
	for (Variable *v = variableStack; v != NULL; v = vn)
	{
		vn = v->getDown();
		// delete v;
	}
}

void
Cinema::AddVariable(Variable *v)
{
	v->setDown(variableStack);
	variableStack = v;
}

void
Cinema::Render(Renderer& r, int timestep)
{
	std::cerr << "This will generate " << variableStack->count() << " frames\n";
	sleep(3);

	timesteps.push_back(timestep);

	Document doc;
	doc.Parse("{}");

	Value attrs(kObjectType);
	doc.AddMember("attributes", attrs, doc.GetAllocator());

	Value desc(kObjectType);
	doc.AddMember("descriptor", desc, doc.GetAllocator());
	SetIntAttr(doc, "Timestep", timestep);
	
	char buf[256];
	sprintf(buf, "cinema_%d", timestep);

	variableStack->ReInitialize(r);
	variableStack->Render(r, string(buf), doc);
}

void
Cinema::WriteInfo()
{
	Document doc;
	doc.Parse("{}");

	Value args(kObjectType);
	doc.AddMember("arguments", args, doc.GetAllocator());
	AddIntRangeArg(doc, string("Timestep"), timesteps);
	string t = variableStack->GatherTemplate(string("cinema_{Timestep}"), doc);

	Value v;
	v.SetString((t + ".png").c_str(), doc.GetAllocator());
	doc.AddMember("name_pattern", v, doc.GetAllocator());

	v.SetString("parametric-image-stack", doc.GetAllocator());

	Value m(kObjectType);
	m.AddMember("type", v, doc.GetAllocator());

	doc.AddMember("metadata", m, doc.GetAllocator());
	
	StringBuffer sbuf;
	PrettyWriter<StringBuffer> writer(sbuf);
	doc.Accept(writer);
	
	ofstream out;
	out.open("info.json", ofstream::out);
	out << sbuf.GetString() << "\n";
	out.close();
};

