#include <iostream>
#include <fstream>
#include "Volume.h"
#include "TransferFunction.h"

Volume::Volume(bool s) :
		shared(s), nIso(0), isoValues(NULL),
		voxels(NULL), mod(true), data(NULL),
		type("none"), x(-1), ospv(NULL)
{}

void
Volume::Initialize(bool s)
{
	if (ospv) 
	{
		ospRelease(ospv); 
		ospv = NULL;
	}

	if (data)
	{
		ospRelease(data); 
		data = NULL;
	}

	if (voxels) 
	{
		free(voxels); 
		voxels = NULL; 
	}

	shared = s;
	ospv = s ? ospNewVolume("shared_structured_volume") : ospNewVolume("block_bricked_volume");
}

Volume::~Volume()
{ 
	if (ospv) ospRelease(ospv); 
	if (data) ospRelease(data); 
	if (voxels) free(voxels); 
}

void
Volume::commit(bool commit_data)
{
	if (! ospv) 
	{
		std::cerr << "can't commit uninitialied volume\n";
		exit(1);
	}


	if (mod)
	{
		ospCommit(ospv);
		mod = false;
	}

	if (commit_data)
	{
		ResetMinMax();
		ospCommit(data);
	}
}

OSPVolume
Volume::getOSPVolume()
{
	commit();
	return ospv;
}
	
// The following can be used in the case of a shared volume
// so that the app can change the contents.  In the case of an
// unshared volume, its off in ispc land and can't be changed
// without storing a new set of voxels.

void
Volume:: ResetMinMax()
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
Volume:: SetDimensions(int _x, int _y, int _z)
{
		x = _x; y = _y; z = _z; mod = true;
		ospSetVec3i(ospv, "dimensions", osp::vec3i(x, y, z));
}
void
Volume:: GetDimensions(int& _x, int& _y, int& _z) {_x = x; _y = y; _z = z;}

void
Volume:: SetType(std::string _t)
{		
		type = _t; mod = true; 
		ospSetString(ospv, "voxelType", type.c_str());
}
void
Volume:: GetType(std::string& _t) {_t = type;}

void
Volume:: SetSamplingRate(float _r)
{
		samplingRate = _r; mod = true; 
		ospSet1f(ospv, "samplingRate", samplingRate);
}
void
Volume:: GetSamplingRate(float& _r) {_r = samplingRate;}

void
Volume:: SetTransferFunction(TransferFunction _tf) 
{
	if (! ospv)  return;
	ospTransferFunction = _tf.getOSPTransferFunction(); mod = true; 
	ospSetObject(ospv, "transferFunction", ospTransferFunction);
}

void
Volume:: SetVoxels(void*  _v)
{
	if (! ospv) 
	{
		std::cerr << "can't set voxels of uninitialied volume\n";
		exit(1);
	}

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
		ospCommit(data);
		ospSetObject(ospv, "voxelData", data);
	}
	else
		ospSetRegion(ospv, _v, osp::vec3i(0,0,0), osp::vec3i(x,y,z));
}
		
void
Volume:: GetVoxels(void*& _v) {_v = voxels;}

void
Volume:: GetMinMax(float& _m, float& _M) {_m = m; _M = M; }

void
Volume:: SetIsovalues(int n, float *v)
{
	if (! ospv) return;

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
Volume:: _setMinMax(void *v)
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

void 
Volume::Import(const std::string &filename, TransferFunction& tf)
{
  size_t x, y, z;
  std::string type;
  char rfile[256];

  std::string dir((filename.find_last_of("/") == std::string::npos) ? "" : filename.substr(0, filename.find_last_of("/")+1));
    
  std::ifstream in;
  in.open(filename.c_str());
  in >> x >> y >> z >> type >> rfile;
  std::cerr << x << " " << y << " " << z << " " << type << " " << rfile << "\n";
  in.close();

  size_t k = x * y * z;
  size_t sz;
  if (type == "float")
    sz = k * sizeof(float);
  else if (type == "uchar")
    sz = k;
  else
  {
    std::cerr << "unrecognized type: " << type << "\n";
    std::exit(1);
  }

  void *data = (void *)new char[sz];

  in.open(rfile[0] == '/' ? rfile : (dir + rfile).c_str(), std::ios::binary | std::ios::in);
  in.read((char *)data, sz);
  in.close();

	Initialize(false);

  SetDimensions(x, y, z);
  SetType(type);
  SetSamplingRate(1.0);
  SetTransferFunction(tf);
  SetVoxels(data);
  commit();

  float m, M;
  GetMinMax(m, M);
  tf.SetMin(m);
  tf.SetMax(M);
}

void
Volume::Attach(const std::string& type, int xsz, int ysz, int zsz, void *data, TransferFunction& tf)
{
	Initialize(true);
	SetType(type);
	SetDimensions(xsz, ysz, zsz);
	SetVoxels(data);
	SetTransferFunction(tf);
	commit();
}

void
VolumeSeries::Import(const std::string &filename, TransferFunction& tf)
{
	if (filename.substr(filename.rfind('.')) == ".vol")
	{
		series.resize(1);
		series[0].Import(filename, tf);
	}
	else
	{
		int series_x, series_y, series_z;
		std::string series_type;

		std::string dir((filename.find_last_of("/") == std::string::npos) ? "" : filename.substr(0, filename.find_last_of("/")+1));

		std::ifstream in;
		in.open(filename.c_str());

		int n;
		in >> n;

		series.resize(n);

		for (int i = 0; i < n; i++)
		{
			char vfile[256];
			in >> vfile;

			if (vfile[0] == '/' || vfile[0] == '.')
				series[i].Import(vfile, tf);
			else
				series[i].Import(dir + '/' + vfile, tf);

			if (i == 0)
			{
				series[i].GetDimensions(series_x, series_y, series_z);
				series[i].GetType(series_type);
			}
			else
			{
				int x, y, z;
				std::string type;

				series[i].GetDimensions(x, y, z);
				series[i].GetType(type);

				if (x != series_x || y != series_y || z != series_z || type != series_type)
				{
					std::cerr << "Series member mismatch\n";
					exit(1);
				}
			}
		}
	}
}

void
VolumeSeries::ResetMinMax()
{
	for (std::vector<Volume>::iterator v = series.begin(); v != series.end(); ++v)
		v->ResetMinMax();
}

void
VolumeSeries::GetDimensions(int& _x, int& _y, int& _z)
{
	if (series.size() == 0)
	{
		std::cerr << "Can't get dimensions of empty volume\n";
		exit(1);
	}
	series[0].GetDimensions(_x, _y, _z);
}

void
VolumeSeries::GetType(std::string& _t)
{
	if (series.size() == 0)
	{
		std::cerr << "Can't get type of empty volume\n";
		exit(1);
	}
	series[0].GetType(_t);
}

void
VolumeSeries::GetSamplingRate(float& _r)
{
	if (series.size() == 0)
	{
		std::cerr << "Can't get sampling rate of empty volume\n";
		exit(1);
	}

	series[0].GetSamplingRate(_r);
}

void
VolumeSeries::GetMinMax(float& _m, float& _M)
{
	if (series.size() == 0)
	{
		std::cerr << "Can't get minmax of empty volume\n";
		exit(1);
	}
	series[0].GetMinMax(_m, _M);
}

void
VolumeSeries::SetIsovalues(int n, float *v)
{
	if (series.size() == 0)
	{
		std::cerr << "Can't set isovalues of empty volume\n";
		exit(1);
	}

	for (std::vector<Volume>::iterator s = series.begin(); s != series.end(); ++s)
		s->SetIsovalues(n, v);
}

void
VolumeSeries::SetTransferFunction(TransferFunction _tf)
{
	if (series.size() == 0)
	{
		std::cerr << "Can't set transfer function of empty volume\n";
		exit(1);
	}
	for (std::vector<Volume>::iterator v = series.begin(); v != series.end(); ++v)
		v->SetTransferFunction(_tf);
}
