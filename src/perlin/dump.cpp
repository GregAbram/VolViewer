#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include "ospray/include/ospray/ospray.h"
#include "MyVolume.h"

#include "perlin_ispc.h"

using namespace std;

void
syntax(char *a)
{
    cerr << "syntax: " << a << " [options]\n";
    cerr << "options:\n";
    cerr << "  -r xres yres zres  overall grid resolution (512x512x512)\n";
    cerr << "  -O octave          noise octave (4)\n";
    cerr << "  -F frequency       noise frequency (8)\n";
    cerr << "  -P persistence     noise persistence (0.5)\n";
    exit(1);
}

int main(int argc, char *argv[])
{
  int xsz = 512, ysz = 512, zsz = 512;
  float t = 3.1415926;

  for (int i = 1; i < argc; i++)
    if (argv[i][0] == '-') 
      switch(argv[i][1])
      {
				case 'r': xsz = atoi(argv[++i]);
									ysz = atoi(argv[++i]);
									zsz = atoi(argv[++i]); break;
				case 'P': ispc::SetPersistence(atof(argv[++i])); break;
				case 'F': ispc::SetFrequency(atof(argv[++i])); break;
				case 'O': ispc::SetOctaveCount(atoi(argv[++i])); break;
				default:  syntax(argv[0]);
			}
		else
			syntax(argv[0]);

	ospInit(&argc, (const char **)argv);

	size_t np = ((size_t)xsz)*((size_t)ysz)*((size_t)zsz);
	float *scalars = new float[np];

	ispc::PerlinT(scalars, xsz, ysz, zsz, 0.0);

	ofstream f("data.raw", ofstream::binary);
	f.write((char *)scalars, np*sizeof(float));
	f.close();

	ofstream v("data.vol");
	v << xsz << " " << ysz << " " << zsz << " float data.raw\n";
	v.close();
}

