#include <vector>
#include "ospray/include/ospray/ospray.h"
#include "../common/common.h"

#include "Renderer.h"

using namespace std;
using namespace rapidjson;

class Cinema;

class Variable
{
public:
	Variable(string n);

	virtual void ReInitialize(Renderer& r);
	virtual void  Render(Renderer& r, string s, Document& doc) = 0;
	virtual string GatherTemplate(string, Document& doc) = 0;

	void RenderDown(Renderer& r, string s, Document& doc);

	Variable *getDown() {return down;}
	void setDown(Variable *v) {down = v;}
	void setCinema(Cinema *c) {cinema = c;}

	virtual int count() = 0;

protected:
	Variable *down;
	string name;
	Cinema *cinema;
};

class CameraVariable : public Variable
{
public:
	CameraVariable(vector<int> p, vector<int> t);

	virtual void  ReInitialize(Renderer& r) {};								// Stop downward pass
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
	VolumeRenderingVariable(vector<int>);

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
	SlicePlaneVariable(string n, vector<int> c, vector<int> v, vector<int> f, vector<int> val);
	SlicePlaneVariable(string n, vector<int> v, vector<int> f, vector<int> vals);

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
	IsosurfaceVariable(string n, vector<int> v);

	virtual void ReInitialize(Renderer& r);
	void  Render(Renderer& r, string s, Document& doc);
	virtual string GatherTemplate(string, Document& doc);

	int count() {
		return values.size() * down->count();
	}

private:
	vector<int> values;
};

class Cinema 
{
public:
		Cinema(int *argc, const char **argv);
		~Cinema();

		void Render(Renderer&, int t);

		void WriteInfo();

		void AddVariable(Variable *v);

		int Count() { return variableStack ? variableStack->count() : -1 ; }

		void setSaveState(bool s) { saveState = s; }
		bool getSaveState() { return saveState; }

private:
		Variable *variableStack;
		vector<int> timesteps;
		bool saveState;
};

