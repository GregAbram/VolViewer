#include <vector>
#include "ospray/include/ospray/ospray.h"
#include "../common/common.h"

#include "Renderer.h"
#include "Variable.h"

using namespace std;
using namespace rapidjson;

class Cinema 
{
public:
		Cinema(int *argc, const char **argv);
		~Cinema();

		void Load(Renderer&, char *, bool);

		void Render(Renderer&, int t);

		void WriteInfo();

		void AddVariable(Variable *v);

		int Count() 
		{ 
			int k = 0;
			for (int i = 0; i < stacks.size(); i++)
				k = k + stacks[i]->count();
			return k;
		}

		void setSaveState(bool s) { saveState = s; }
		bool getSaveState() { return saveState; }

private:
		vector<Variable *> stacks;
		vector<int> timesteps;
		bool saveState;
};

