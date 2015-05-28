#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include "Cinema.h"

#include "perlin_ispc.h"

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

CameraVariable *
cinema_setup(Renderer& renderer, Cinema& cinema)
{
#if 1
	vector<int> phis;
	int d = 20 / 2;
	for (int i = 0; i < 3; i++)
		phis.push_back(i*d);

	vector<int> thetas;
	d = 90 / 15;
	for (int i = 0; i < 12; i++)
		thetas.push_back(i*d);
#else
	vector<int> phis;
	phis.push_back(45);

	vector<int> thetas;
	thetas.push_back(45);
#endif

	CameraVariable *camvar = new CameraVariable(phis, thetas);
	cinema.AddVariable(camvar);

#if 1
	vector<int> clips;
	clips.push_back(0);

	vector<int> visible;
	visible.push_back(0);
	visible.push_back(1);

	vector<int> flip;
	flip.push_back(0);
	flip.push_back(1);

	vector<int> values;
	for (int i = 20; i < 85; i += 5)
		values.push_back(i);

	SlicePlaneVariable *clip = new SlicePlaneVariable(string("X"), clips, visible, flip, values);
	cinema.AddVariable(clip);
#endif

#if 1
	vector<int> isovalues;
	for (int i = 0; i < 10; i++)
		isovalues.push_back(20 + (int)((i / 9.9) * 60));

	IsosurfaceVariable *iso = new IsosurfaceVariable(string("Iso"), isovalues);
	cinema.AddVariable(iso);
#endif

	vector<int> doVR;
	doVR.push_back(1);
	
	VolumeRenderingVariable *vrvar = new VolumeRenderingVariable(doVR);
	cinema.AddVariable(vrvar);

	std::cerr << "Requires " << cinema.Count() << " images\n";
	return camvar;
}

int main(int argc, char *argv[])
{
	Cinema cinema(&argc, (const char **)argv);

  int xsz = 512, ysz = 512, zsz = 512;
	int width = 512, height = 512;
  float t = 3.1415926;
  float delta_t = 0.05;
  int   nt = 1;
	char *filename = NULL;
	CameraVariable *camvar = NULL;

  for (int i = 1; i < argc; i++)
    if (argv[i][0] == '-') 
      switch(argv[i][1])
      {
				case 'r': xsz = atoi(argv[++i]);
									ysz = atoi(argv[++i]);
									zsz = atoi(argv[++i]); break;
				case 's': width = atoi(argv[++i]);
									height = atoi(argv[++i]); break;
				case 'P': ispc::SetPersistence(atof(argv[++i])); break;
				case 'F': ispc::SetFrequency(atof(argv[++i])); break;
				case 'O': ispc::SetOctaveCount(atoi(argv[++i])); break;
				case 't': delta_t = atof(argv[++i]); nt = atoi(argv[++i]); break;
				default:  syntax(argv[0]);
			}
		else if (filename == NULL)
			filename = argv[i];
		else
			syntax(argv[0]);

	// Create renderer with shared volume
	Renderer renderer(true);

	renderer.getWindow().resize(width, height);
	renderer.LoadState(std::string(filename), false);
	camvar = cinema_setup(renderer, cinema);

	int np = xsz*ysz*zsz;
	float *scalars = new float[np];

	renderer.getVolume()->SetType(std::string("float"));
	renderer.getVolume()->SetDimensions(xsz, ysz, zsz);
	renderer.getVolume()->SetVoxels((void *)scalars);
	renderer.getVolume()->SetTransferFunction(renderer.getTransferFunction().getOSPTransferFunction());
	renderer.CommitVolume();

  for (int t = 0; t < nt; t++)
  {
		ispc::PerlinT(scalars, xsz, ysz, zsz, t*delta_t);
		renderer.getVolume()->ResetMinMax();

		camvar->ResetCount();
		cinema.Render(renderer, t);

		std::cerr << "timestep " << t << " done\n";
	}

	cinema.WriteInfo();
}

