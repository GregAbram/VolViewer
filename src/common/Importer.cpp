#include <iostream>
#include <fstream>

#include "ospray/ospray.h"

#include "TransferFunction.h"
#include "Importer.h"

using namespace std;

void importVolume(MyVolume& mv, const std::string &filename, TransferFunction& tf)
{
	size_t x, y, z;
	std::string type;
	char rfile[256];

	string dir((filename.find_last_of("/") == std::string::npos) ? "" : filename.substr(0, filename.find_last_of("/")+1));
		
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

	in.open(rfile[0] == '/' ? rfile : (dir + rfile).c_str(), ios::binary | ios::in);
	in.read((char *)data, sz);
	in.close();

	mv.SetDimensions(x, y, z);
	mv.SetType(type);
	mv.SetSamplingRate(1.0);
	mv.SetTransferFunction(tf.getOSPTransferFunction());
	mv.SetVoxels(data);
	mv.commit();

	float m, M;
	mv.GetMinMax(m, M);
	tf.SetMin(m);
	tf.SetMax(M);
}
