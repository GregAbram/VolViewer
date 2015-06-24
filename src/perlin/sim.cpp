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
    cerr << "  -f frequency       noise frequency (8)\n";
    cerr << "  -P persistence     noise persistence (0.5)\n";
    cerr << "  -t dt nt           time series delta, number of timesteps (0, 1)\n";
    cerr << "  -s w h           	size of output images\n";
		cerr << "  -F                 save state files (first time step)\n";
		cerr << "  -D                 save each time step volume\n";
#if WITH_OPENGL == TRUE
		cerr << "  -S                 show images as they are rendered\n";
#endif
    exit(1);
}

CameraVariable *
cinema_setup(Renderer& renderer, Cinema& cinema)
{
#if 0
	vector<int> phis;
	int d = 20 / 2;
	for (int i = 0; i < 3; i++)
		phis.push_back(i*d);

	vector<int> thetas;
	d = 90 / 31;
	for (int i = 0; i < 32; i++)
		thetas.push_back(i*d);
#else
	vector<int> phis;
	phis.push_back(0);

	vector<int> thetas;
	thetas.push_back(0);
#endif

	CameraVariable *camvar = new CameraVariable(phis, thetas);
	cinema.AddVariable(camvar);

#if 0
	vector<int> clips;
	clips.push_back(0);

	vector<int> visible;
	visible.push_back(1);

	vector<int> flip;
	flip.push_back(1);

	vector<int> values;
	for (int i = 0; i < 1; i ++)
		values.push_back(40 + i*5);

	SlicePlaneVariable *clip = new SlicePlaneVariable(string("Z"), clips, visible, flip, values);
	cinema.AddVariable(clip);
#endif

#if 0
	vector<int> isovalues;
	for (int i = 0; i < 20; i++)
		isovalues.push_back(20 + (int)((i / 19.0) * 60));

	IsosurfaceVariable *iso = new IsosurfaceVariable(string("Iso"), isovalues);
	cinema.AddVariable(iso);
#else
#if 0
	vector<int> isovalues;
	isovalues.push_back(50);
	IsosurfaceVariable *iso = new IsosurfaceVariable(string("Iso"), isovalues);
	cinema.AddVariable(iso);
#endif
#endif

#if 0
	vector<int> doVR;
	doVR.push_back(1);
	VolumeRenderingVariable *vrvar = new VolumeRenderingVariable(doVR);
	cinema.AddVariable(vrvar);
#endif

	std::cerr << "Requires " << cinema.Count() << " images\n";
	return camvar;
}

int main(int argc, char *argv[])
{
	Cinema cinema(&argc, (const char **)argv);

  int xsz = 256, ysz = 256, zsz = 256;
	int width = 512, height = 512;
  float t = 3.1415926;
  float delta_t = 0.05;
  int   nt = 1;
	char *filename = NULL;
	CameraVariable *camvar = NULL;
	bool saveState = false;
	bool dump = false;
#if WITH_OPENGL == TRUE
  bool show = false;
#endif

  for (int i = 1; i < argc; i++)
    if (argv[i][0] == '-') 
      switch(argv[i][1])
      {
				case 'r': xsz = atoi(argv[++i]);
									ysz = atoi(argv[++i]);
									zsz = atoi(argv[++i]); break;
#if WITH_OPENGL == TRUE
				case 'S': show = true; break;
#endif
				case 'F': saveState = true; break;
				case 'D': dump = true; break;
				case 's': width = atoi(argv[++i]);
									height = atoi(argv[++i]); break;
				case 'P': ispc::SetPersistence(atof(argv[++i])); break;
				case 'f': ispc::SetFrequency(atof(argv[++i])); break;
				case 'O': ispc::SetOctaveCount(atoi(argv[++i])); break;
				case 't': delta_t = atof(argv[++i]); nt = atoi(argv[++i]); break;
				default:  syntax(argv[0]);
			}
		else if (filename == NULL)
			filename = argv[i];
		else
			syntax(argv[0]);


#if 0
	// Create renderer with shared volume
	Renderer renderer(width, height, true);
#else
	Renderer renderer(width, height, false);
#endif

#if WITH_OPENGL == TRUE
  renderer.getWindow()->setShow(show);
#endif

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

#if 0
#if 0
		renderer.getVolume()->commit(true);
		renderer.getVolume()->ResetMinMax();
#else
		renderer.getVolume()->SetVoxels(scalars);
		renderer.getVolume()->commit();
#endif

		camvar->ResetCount();

		cinema.setSaveState(saveState && (t == 0));
		cinema.Render(renderer, t);

		std::cerr << "timestep " << t << " done\n";

#endif
		if (dump)
		{
			char tt[256];
			sprintf(tt, "data_%05d.raw", t);
			ofstream f("data.raw", ofstream::binary);
			f.write((char *)scalars, np*sizeof(float));
			f.close();

			sprintf(tt, "data_%05d.vol", t);
			ofstream v(tt);
			v << xsz << " " << ysz << " " << zsz << " float data.raw\n";
			v.close();
		}
	}

	cinema.WriteInfo();
}

