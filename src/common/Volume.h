#pragma once

#include "ospray/ospray.h"
#include <vector>

class TransferFunction;

class Volume
{
public:
		Volume();
		~Volume();

		void Initialize(bool shared);

		void commit(bool commit_data = false);
		OSPVolume getOSPVolume();
		
		void ResetMinMax();
		void SetDimensions(int _x, int _y, int _z);
		void GetDimensions(int& _x, int& _y, int& _z);
		void SetType(std::string _t);
		void GetType(std::string& _t);
		void SetSamplingRate(float _r);
		void GetSamplingRate(float& _r);
		void SetTransferFunction(TransferFunction _tf);
		void SetVoxels(void*  _v);
		void GetVoxels(void*& _v);
		void GetMinMax(float& _m, float& _M);
		void SetIsovalues(int n, float *v);

		void Import(const std::string& s, TransferFunction& t);
		void Import(const char *s, TransferFunction& t) { Import(std::string(s), t); }
		void Attach(const std::string&, int, int, int, void *, TransferFunction&);

private:

		void _setMinMax(void *v);

		bool 								shared;

		int 							  x, y, z;
		std::string 			  type;
		float							  samplingRate;
		OSPTransferFunction ospTransferFunction;
		void								*voxels;

		float 							m, M;

		int									nIso;
		float 							*isoValues;

		bool								mod;
		OSPVolume 					ospv;
		OSPData 						data;
};

class VolumeSeries
{
public:
		VolumeSeries() {}
		~VolumeSeries() {}

		void Import(const std::string &filename, TransferFunction& tf);
		void ResetMinMax();
		void GetDimensions(int& _x, int& _y, int& _z);
		void GetType(std::string& _t);
		void GetSamplingRate(float& _r);
		void GetMinMax(float& _m, float& _M);
		void SetIsovalues(int n, float *v);
		void SetTransferFunction(TransferFunction _tf);

		int GetNumberOfMembers() { return series.size(); }
		Volume *GetMember(int i) { return &series[i]; }

private:
		std::vector<Volume> series;
};

