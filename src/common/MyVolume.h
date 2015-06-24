#pragma once

#include "ospray/ospray.h"

class MyVolume
{
public:
		MyVolume(bool s);
		~MyVolume();
		void commit(bool do_anyway = false);
		OSPVolume getOSPVolume();
			
		// The following can be used in the case of a shared volume
		// so that the app can change the contents.  In the case of an
		// unshared volume, its off in ispc land and can't be changed
		// without storing a new set of voxels.
		
		void ResetMinMax();
		void SetDimensions(int _x, int _y, int _z);
		void GetDimensions(int& _x, int& _y, int& _z);
		void SetType(std::string _t);
		void GetType(std::string& _t);
		void SetSamplingRate(float _r);
		void GetSamplingRate(float& _r);
		void SetTransferFunction(OSPTransferFunction _tf);
		void GetTransferFunction(OSPTransferFunction& _tf);
		void SetVoxels(void*  _v);
		void GetVoxels(void*& _v);
		void GetMinMax(float& _m, float& _M);
		void SetIsovalues(int n, float *v);

private:

		void _setMinMax(void *v);

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
		OSPData 						data;
};
