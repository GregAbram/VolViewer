#if WITH_ISPC == 1
#include "perlin_ispc.h"
using namespace ispc;
#else
void SetFrequency(float f);
void SetLacunarity(float l);
void SetQuality(int q);
void SetOctaveCount(int o);
void SetPersistence(float p);
void SetSeed(int s);
void PerlinT(float buf[], int xsz, int ysz, int zsz, float t);
#endif


