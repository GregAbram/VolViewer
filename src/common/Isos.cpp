#include "Isos.h"

using namespace std;

Isos::Isos()
{
	for (int i = 0; i < 3; i++)
	{
		values[i] = 0.0;
		onoffs[i] = false;
	}

	min = 0.0;
	max = 1.0;
}

void  
Isos::SetMinMax(float m, float M) 
{
	min = m; 
	max = M; 
}

void  
Isos::SetValue(int i, int v)   
{
	if (i > 2) i = 2; 
	values[i] = v / 100.0; 
}

void  
Isos::SetValue(int i, float v)   
{
	if (i > 2) i = 2; 
	values[i] = v; 
}

float 
Isos::GetValue(int i) 
{
	if (i > 2) i = 2;
	return values[i]; 
}

void  
Isos::SetOnOff(int i, bool b)   
{
	if (i > 2) i = 2;
	onoffs[i] = b; 
}

bool  
Isos::GetOnOff(int i)   
{
	if (i > 2) i = 2;
	return onoffs[i]; 
}

void 
Isos::loadState(Value &section)
{
	int j, i = 0;
	for (Value::ConstValueIterator itr = section.Begin(); itr != section.End(); ++itr)
	{
		std::stringstream ss(itr->GetString());
		ss >> values[i] >> j;
		onoffs[i] = (j == 1);
		i++;
		if (i > 3) break;
	}
	for ( ; i < 3; i++)
		onoffs[i] = false;
}

void 
Isos::saveState(Document &doc, Value &section)
{
	Value a(kArrayType);

	for (int i = 0; i < 3; i++)
	{
		std::stringstream ss;
		ss << values[i] << " " << (onoffs[i] ? 1 : 0);
		a.PushBack(Value().SetString(ss.str().c_str(), doc.GetAllocator()),doc.GetAllocator());
	}
		
	section.AddMember("Isosurfaces", a, doc.GetAllocator());
}

void 
Isos::commit(Volume *vol)
{
	float v[3];

	int k = 0;
	for (int i = 0; i < 3; i++)
		if (onoffs[i])
		{
			v[k++] = values[i];
		}

	printf("committing %d Isovalues\n", k);
	vol->SetIsovalues(k, v);
}
