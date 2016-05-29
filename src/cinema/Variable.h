#include <vector>
#include "ospray/include/ospray/ospray.h"
#include "../common/common.h"

using namespace std;
using namespace rapidjson;

class Renderer;
class Cinema;
class Variable;


class VariableFactory
{
	class VariableCtor {
	public:
		VariableCtor(string n, Variable *(*f)(Value&)) : name(n), func(f) {};
		string name;
		Variable *(*func)(Value&);
	};

public:
	VariableFactory();
	~VariableFactory(){};
	
	void Register(const char *name, Variable *(*func)(Value &));
	Variable *Create(Value& value);

private:
	vector<VariableCtor> registry;
};

extern VariableFactory theVariableFactory;

class Variable
{
protected:
	Variable(string op);

public:
	virtual void ReInitialize(Renderer& r);
	virtual void  Render(Renderer& r, string s, Document& doc) = 0;
	virtual string GatherTemplate(string, Document& doc) = 0;

	void RenderDown(Renderer& r, string s, Document& doc);

	Variable *getDown() {return down;}
	void setDown(Variable *v) {down = v;}

	virtual int count() = 0;

	void Parse(Value&);

protected:
	Variable *down;
	string name;
};

class CameraVariable : public Variable
{
public:
	static Variable *New(Value&);
	CameraVariable(vector<int> p, vector<int> t);

	virtual void  ReInitialize(Renderer& r) {};
	virtual void  Render(Renderer& r, string s, Document& doc);
	virtual string GatherTemplate(string, Document& doc);

	void ResetCount() { knt = 0; }

	int count() {
		return phis.size() * thetas.size();
	}

protected:
	void RenderShot(Renderer&, string, Document&);

	int knt;

	vector<int> phis;
	vector<int> thetas;
};

class VolumeRenderingVariable : public Variable
{
public:
	static Variable *New(Value&);
	VolumeRenderingVariable(string n, vector<int>);

	virtual void  Render(Renderer& r, string s, Document& doc);
	virtual string GatherTemplate(string, Document& doc);

	int count() {
		return onOff.size() * down->count();
	}

protected:
	void RenderShot(Renderer&, string, Document&);

	vector<int> onOff;
};

class SlicePlaneVariable : public Variable
{
public:
	static Variable *New(Value&);
	SlicePlaneVariable(string n, vector<int>a, vector<int> c, vector<int> v, vector<int> f, vector<int> val);

	void  Render(Renderer& r, string s, Document& doc);
	string GatherTemplate(string, Document& doc);

	int count() {
		return axes.size() * clips.size() * visibles.size() * flips.size() * values.size() * down->count();
	}

private:
	vector<int> axes;
	vector<int> clips;
	vector<int> visibles;
	vector<int> flips;
	vector<int> values;
};

class IsosurfaceVariable : public Variable
{
public:
	static Variable *New(Value&);
	IsosurfaceVariable(string n, int i, vector<int> v);

	virtual void ReInitialize(Renderer& r);
	void  Render(Renderer& r, string s, Document& doc);
	virtual string GatherTemplate(string, Document& doc);

	int count() {
		return values.size() * down->count();
	}

private:
	vector<int> values;
	int index;
};

