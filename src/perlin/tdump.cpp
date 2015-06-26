#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include "perlin.h"

using namespace std;

void
syntax(char *a)
{
    cerr << "syntax: " << a << " [options] statefile\n";
    cerr << "options:\n";
    cerr << "  -r xres yres zres  overall grid resolution (512x512x512)\n";
    cerr << "  -O octave          noise octave (4)\n";
    cerr << "  -F frequency       noise frequency (8)\n";
    cerr << "  -P persistence     noise persistence (0.5)\n";
    cerr << "  -t dt nt           time series delta, number of timesteps (0, 1)\n";
    cerr << "  -s w h           	size of output images\n";
    exit(1);
}

int main(int argc, char *argv[])
{
  int xsz = 512, ysz = 512, zsz = 512;
	int width = 512, height = 512;
  float t = 3.1415926;
  float delta_t = 0.05;
  int   nt = 1;
	char *filename = NULL;
	bool dump_data = false;

  for (int i = 1; i < argc; i++)
    if (argv[i][0] == '-') 
      switch(argv[i][1])
      {
				case 'r': xsz = atoi(argv[++i]);
									ysz = atoi(argv[++i]);
									zsz = atoi(argv[++i]); break;
				case 's': width = atoi(argv[++i]);
									height = atoi(argv[++i]); break;
				case 'P': SetPersistence(atof(argv[++i])); break;
				case 'F': SetFrequency(atof(argv[++i])); break;
				case 'O': SetOctaveCount(atoi(argv[++i])); break;
				case 't': delta_t = atof(argv[++i]); nt = atoi(argv[++i]); break;
				case 'D': dump_data = true; break;
				default:  syntax(argv[0]);
			}
		else if (filename == NULL)
			filename = argv[i];
		else
			syntax(argv[0]);

	int np = xsz*ysz*zsz;
	float *scalars = new float[np];

  for (int t = 0; t < nt; t++)
  {
		PerlinT(scalars, xsz, ysz, zsz, t*delta_t);

		char fname[256];
		sprintf(fname, "timestep_%04d.raw", t);
		ofstream f(fname, ofstream::binary);
		f.write((char *)scalars, np*sizeof(float));
		f.close();

		std::cerr << "timestep " << t << " done\n";
	}
}

