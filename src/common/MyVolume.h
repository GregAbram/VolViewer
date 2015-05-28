#pragma once

#include "ospray/ospray.h"

class MyVolume
{
public:
		MyVolume(bool s) :
				shared(s), nIso(0), isoValues(NULL),
				voxels(NULL), mod(true), 
				type("none"), x(-1)
	  {
			ospv = s ? ospNewVolume("shared_structured_volume") : ospNewVolume("block_bricked_volume");
			SetTransferFunction(ospNewTransferFunction("piecewise_linear"));
		};

		~MyVolume()
		{ 
			if (ospv) ospRelease(ospv); 
			if (voxels) free(voxels); 
		}

		void commit()
		{
			if (mod)
			{


				ospCommit((osp::ManagedObject *)ospv);
				mod = false;
			}
		}

		OSPVolume getOSPVolume()
		{
			commit();
			return ospv;
		}
			
		// The following can be used in the case of a shared volume
		// so that the app can change the contents.  In the case of an
		// unshared volume, its off in ispc land and can't be changed
		// without storing a new set of voxels.
		
		void ResetMinMax()
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

		void SetDimensions(int _x, int _y, int _z)
		{
				x = _x; y = _y; z = _z; mod = true;
				ospSetVec3i(ospv, "dimensions", osp::vec3i(x, y, z));
		}
		void GetDimensions(int& _x, int& _y, int& _z) {_x = x; _y = y; _z = z;}

		void SetType(std::string _t)
		{		
				type = _t; mod = true; 
				ospSetString(ospv, "voxelType", type.c_str());
		}
		void GetType(std::string& _t) {_t = type;}

		void SetSamplingRate(float _r)
		{
				samplingRate = _r; mod = true; 
				ospSet1f(ospv, "samplingRate", samplingRate);
		}
		void GetSamplingRate(float& _r) {_r = samplingRate;}

		void SetTransferFunction(OSPTransferFunction _tf) 
		{
				transferFunction = _tf; mod = true; 
				ospSetObject(ospv, "transferFunction", transferFunction);
		}
		void GetTransferFunction(OSPTransferFunction& _tf) {_tf = transferFunction;}

		void SetVoxels(void*  _v)
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
				osp::Data *data = ospNewData(k, type == "float" ? OSP_FLOAT : OSP_UCHAR, voxels, OSP_DATA_SHARED_BUFFER);
				ospSetObject(ospv, "voxelData", data);
			}
			else
				ospSetRegion(ospv, _v, osp::vec3i(0,0,0), osp::vec3i(x,y,z));
		}
				
		void GetVoxels(void*& _v) {_v = voxels;}

		void GetMinMax(float& _m, float& _M) {_m = m; _M = M; }

		void SetIsovalues(int n, float *v)
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

private:

		void _setMinMax(void *v)
		{
			if (type == "float")
			{
				float *ptr = (float *)v;
				m = *ptr;
				M = *ptr;
				for (int i = 0; i < x*y*z; i++, ptr++)
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
				for (int i = 0; i < x*y*z; i++, ptr++)
				{
					if (m > *ptr) m = *ptr;
					if (M < *ptr) M = *ptr;
				}
			}
		}

		OSPVolume						ospVolume;
		bool 								shared;

		int 							  x, y, z;
		std::string 			  type;
		float							  samplingRate;
		OSPTransferFunction transferFunction;
		void								*voxels;

		float 							m, M;

		int									nIso;
		float 							*isoValues;

		bool								mod;
		OSPVolume 					ospv;
};
