#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "Cinema.h"
#include "util.h"

using namespace std;
using namespace rapidjson;

Cinema::Cinema(int *argc, const char **argv) : saveState(false)
{
  ospInit(argc, (const char **)argv);
}

Cinema::~Cinema()
{
}

void
Cinema::AddVariable(Variable *v)
{
	v->setDown(stacks.back());
	stacks.back() = v;
}

void
Cinema::Render(Renderer& r, int timestep)
{
	std::cerr << "This will generate " << Count() << " images\n";
	timesteps.push_back(timestep);

	for (int i = 0; i < stacks.size(); i++)
	{
		Document doc;
		doc.Parse("{}");

		Value attrs(kObjectType);
		doc.AddMember("attributes", attrs, doc.GetAllocator());

		Value desc(kObjectType);
		doc.AddMember("descriptor", desc, doc.GetAllocator());
		SetIntAttr(doc, "Timestep", timestep);
	
		char buf[256];
		sprintf(buf, "cinema%d_%d", i, timestep);

		stacks[i]->ReInitialize(r);
		stacks[i]->Render(r, string(buf), doc);
	}
}

void
Cinema::WriteInfo()
{
	for (int i = 0; i < stacks.size(); i++)
	{
		char buf[256];
		Document doc;

		doc.Parse("{}");

		Value args(kObjectType);
		doc.AddMember("arguments", args, doc.GetAllocator());
		AddIntRangeArg(doc, string("Timestep"), timesteps);
		sprintf(buf, "cinema%d_{Timestep}", i);
		string t = stacks[i]->GatherTemplate(buf, doc);

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
		sprintf(buf, "info%02d.json", i);
		out.open(buf, ofstream::out);
		out << sbuf.GetString() << "\n";
		out.close();
	}
}

void
Cinema::Load(Renderer& r, char *filename, bool inSitu_flag)
{
	Document doc;

	ifstream in;
	in.open(filename, istream::in);
	in.seekg(0, ios::end);
	streamsize size = in.tellg();
	in.seekg(0, ios::beg);

	static char *buf = new char[size+1];
	in.read(buf, size);
	doc.Parse(buf);
	in.close();
	delete[] buf;

	if (!doc.HasMember("Statefile") || !doc["Statefile"].IsString())
	{
		cerr << "bad cinema file\n";
		exit(1);
	}

	if (!doc.HasMember("Cinema") || !doc["Cinema"].IsArray())
	{
		cerr << "bad cinema file\n";
		exit(1);
	}

	r.Load(doc["Statefile"].GetString(), inSitu_flag);
	
	Value& kids = doc["Cinema"];
	for (int i = 0; i < kids.Size(); i++)
		stacks.push_back(theVariableFactory.Create(kids[i]));
}
