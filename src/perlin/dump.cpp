#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include "perlin.h"

#include "ospray/include/ospray/ospray.h"

using namespace std;

void
syntax(char *a)
{
    cerr << "syntax: " << a << " [options]\n";
    cerr << "options:\n";
    cerr << "  -r xres yres zres  overall grid resolution (512x512x512)\n";
    cerr << "  -O octave          noise octave (4)\n";
    cerr << "  -F frequency       noise frequency (8)\n";
		cerr << "  -t dt nt           time series delta, number of timesteps (0, 1)\n";
    cerr << "  -P persistence     noise persistence (0.5)\n";
    exit(1);
}

int main(int argc, char *argv[])
{
  int xsz = 512, ysz = 512, zsz = 512;
  float t = 3.1415926;
	float delta_t;
	int nt = 1;

  for (int i = 1; i < argc; i++)
    if (argv[i][0] == '-') 
      switch(argv[i][1])
      {
				case 'r': xsz = atoi(argv[++i]);
									ysz = atoi(argv[++i]);
									zsz = atoi(argv[++i]); break;
				case 'P': SetPersistence(atof(argv[++i])); break;
				case 'F': SetFrequency(atof(argv[++i])); break;
				case 'O': SetOctaveCount(atoi(argv[++i])); break;
				case 't': delta_t = atof(argv[++i]); nt = atoi(argv[++i]); break;
				default:  syntax(argv[0]);
			}
		else
			syntax(argv[0]);

	ospInit(&argc, (const char **)argv);

	ofstream v("data.ser");
	v << nt << "\n";

	size_t np = ((size_t)xsz)*((size_t)ysz)*((size_t)zsz);
	float *scalars = new float[np];

	for (int i = 0; i < nt; i++)
	{
		PerlinT(scalars, xsz, ysz, zsz, i*delta_t);

		string raw_name = string("timestep-") + to_string(i) + ".raw";
		
		ofstream f(raw_name.c_str(), ofstream::binary);
		f.write((char *)scalars, np*sizeof(float));
		f.close();

		string timestep_name = string("timestep-") + to_string(i) + ".vol";
		v << timestep_name << "\n";

		ofstream t(timestep_name.c_str());
		t << xsz << " " << ysz << " " << zsz << " float " << raw_name << "\n";
		t.close();
	}

	v.close();
}

