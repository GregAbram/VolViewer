#include "MyVolume.h"

MyVolume::MyVolume(bool s) :
		shared(s), nIso(0), isoValues(NULL),
		voxels(NULL), mod(true), 
		type("none"), x(-1)
{
	ospv = s ? ospNewVolume("shared_structured_volume") : ospNewVolume("block_bricked_volume");
	SetTransferFunction(ospNewTransferFunction("piecewise_linear"));
};

MyVolume::~MyVolume()
{ 
	if (ospv) ospRelease(ospv); 
	if (data) ospRelease(data); 
	if (voxels) free(voxels); 
}

void
MyVolume::commit(bool do_anyway)
{
	ospCommit(ospv);
	if (mod | do_anyway)
	{
		OSPData d;
		if (ospGetData(ospv, "voxelData", &d))
			ospCommit(d);
		mod = false;
	}
}

OSPVolume
MyVolume::getOSPVolume()
{
	commit();
	return ospv;
}
	
// The following can be used in the case of a shared volume
// so that the app can change the contents.  In the case of an
// unshared volume, its off in ispc land and can't be changed
// without storing a new set of voxels.

void
MyVolume:: ResetMinMax()
{
	if (! shared)
	{
		std::cerr << "cannot reset minmax of unshared volume\n";
		return;
	}

	if (x == -1 || type == "none")
	{
		std::cerr << "cannot reset minmax of uninitialized shared volume\n";
		return;
	}

	_setMinMax(voxels);
}

void
MyVolume:: SetDimensions(int _x, int _y, int _z)
{
		x = _x; y = _y; z = _z; mod = true;
		ospSetVec3i(ospv, "dimensions", osp::vec3i(x, y, z));
}
void
MyVolume:: GetDimensions(int& _x, int& _y, int& _z) {_x = x; _y = y; _z = z;}

void
MyVolume:: SetType(std::string _t)
{		
		type = _t; mod = true; 
		ospSetString(ospv, "voxelType", type.c_str());
}
void
MyVolume:: GetType(std::string& _t) {_t = type;}

void
MyVolume:: SetSamplingRate(float _r)
{
		samplingRate = _r; mod = true; 
		ospSet1f(ospv, "samplingRate", samplingRate);
}
void
MyVolume:: GetSamplingRate(float& _r) {_r = samplingRate;}

void
MyVolume:: SetTransferFunction(OSPTransferFunction _tf) 
{
		transferFunction = _tf; mod = true; 
		ospSetObject(ospv, "transferFunction", transferFunction);
}
void
MyVolume:: GetTransferFunction(OSPTransferFunction& _tf) {_tf = transferFunction;}

void
MyVolume:: SetVoxels(void*  _v)
{
	if (x == -1 || type == "none")
	{
		std::cerr << "cannot reset minmax of uninitialized shared volume\n";
		return;
	}

	_setMinMax(_v);

	if (shared)
	{
		voxels = _v;
		size_t k = x * y * z;
		data = ospNewData(k, type == "float" ? OSP_FLOAT : OSP_UCHAR, voxels, OSP_DATA_SHARED_BUFFER);
		ospSetObject(ospv, "voxelData", data);
	}
	else
		ospSetRegion(ospv, _v, osp::vec3i(0,0,0), osp::vec3i(x,y,z));
}
		
void
MyVolume:: GetVoxels(void*& _v) {_v = voxels;}

void
MyVolume:: GetMinMax(float& _m, float& _M) {_m = m; _M = M; }

void
MyVolume:: SetIsovalues(int n, float *v)
{
	if (n)
	{
		if (isoValues) delete[] isoValues;
		isoValues = new float[n];
		memcpy((void *)isoValues, (void *)v, n*sizeof(float));
		ospSetData(ospv, "isovalues", ospNewData(nIso, OSP_FLOAT, isoValues));
	}
	else
			ospSetData(ospv, "isovalues", NULL);
	nIso = n;
	mod = true;
}

void
MyVolume:: _setMinMax(void *v)
{
	if (type == "float")
	{
		float *ptr = (float *)v;
		m = *ptr;
		M = *ptr;
		size_t n = ((size_t)x)*((size_t)y)*((size_t)z);
		for (size_t i = 0; i < n; i++, ptr++)
		{
			if (m > *ptr) m = *ptr;
			if (M < *ptr) M = *ptr;
		}
	}
	else
	{
		unsigned char *ptr = (unsigned char *)v;
		m = *ptr;
		M = *ptr;
		size_t n = ((size_t)x)*((size_t)y)*((size_t)z);
		for (size_t i = 0; i < n; i++, ptr++)
		{
			if (m > *ptr) m = *ptr;
			if (M < *ptr) M = *ptr;
		}
	}
}
