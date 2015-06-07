#define AMBIENT 0.9f
#define GDA 0
#define GDA1 0
#define GDA2 0
#define GDA3 1
#define NAO 16
#define AO_RADIUS 100
#define AO_RAYS_PER_PIXEL	640

// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "ospray/fb/FrameBuffer.ih"
#include "ospray/render/util.ih"
#include "VisRenderer.ih"

#include "ospray/volume/SharedStructuredVolume.ih"

#include "UV.ih"

// ContinuationRays are either primary rays that continue
// beyond the current partition or secondary rays, which 
// right now are limited to AO rays.  If its a primary ray,
// then its a ScreenSample.  If its a AO ray, we just need
// the distance it has travelled so far and the contribution
// it will add to the pixel if it doesn't hit anything before
// it hits the AO distance limit.  We'll use the 'Z' field of
// the ScreenSample structure to hold that, so we just need
// a 'type' field in addition to the ScreenSample structure.

enum CRayType {
	Primary, AO
};

struct ContinuationRay {
	ScreenSample sample;
	CRayType type;
	int userData;
};

uniform int ran_initialized = 0;
RNGState state;

struct RNGstate;
uint32 ran()
{

  if (ran_initialized == 0)
  {
    ran_initialized = 1;
    seed_rng(&state, programIndex*783);
  }

	return  0xff & random(&state);
}

bool VisRenderer_AO_intersect(uniform Renderer *uniform, varying ScreenSample &);
void VisRenderer_renderSample(Renderer *uniform, varying ScreenSample &, varying int &, varying ContinuationRay *);

void VisRenderer_getBinormals(vec3f &biNorm0, vec3f &biNorm1, const vec3f &gNormal)
{
  biNorm0 = make_vec3f(1.f,0.f,0.f);
  if (abs(dot(biNorm0,gNormal)) > .95f)
    biNorm0 = make_vec3f(0.f,1.f,0.f);
  biNorm1 = normalize(cross(biNorm0,gNormal));
  biNorm0 = normalize(cross(biNorm1,gNormal));
}

inline vec3f getRandomDir(const int i,
                          const vec3f biNorm0,
                          const vec3f biNorm1,
                          const vec3f gNormal,
													const float epsilon)
{
	const int j = (i >> 3) & 0xff;

  const float r0 = VisRenderer_randomU[j];
  const float r1 = VisRenderer_randomV[j];

  const float w = sqrt(1.f-r1);
  const float x = cos((2.f*M_PI)*r0)*w;
  const float y = sin((2.f*M_PI)*r0)*w;
  const float z = sqrt(r1)+epsilon;
  vec3f rd = x*biNorm0+y*biNorm1+z*gNormal;

	return rd;
}

void generateAORays(VisRenderer *uniform renderer, ScreenSample sample, vec3f ambientColor, const uniform float epsilon, varying int &n_continuation_rays, varying ContinuationRay *continuation_rays)
{
	if (renderer->numAO > 0)
	{
#if GDA
		print("GAOR\norg:\nX %\nY %\nZ %\ndir:\nX %\nY %\nZ %\nt %\n",
				sample.ray.org.x,
				sample.ray.org.y,
				sample.ray.org.z,
				sample.ray.dir.x,
				sample.ray.dir.y,
				sample.ray.dir.z,
				sample.ray.t);
		print("AC\nR %\nG %\nB %\n", ambientColor.x, ambientColor.y, ambientColor.z);
#endif

		ambientColor = (1.0 / renderer->numAO) * ambientColor;
#if GDA2
		print("AC2\nR %\nG %\nB %\n", ambientColor.x, ambientColor.y, ambientColor.z);
#endif

		vec3f b0, b1;
		VisRenderer_getBinormals(b0, b1, sample.ray.dir);
	
		for (int i = 0; i < renderer->numAO; i++)
		{
			if (n_continuation_rays >= AO_RAYS_PER_PIXEL)
				break;

#if 0
			int j = i % 0xff;
			const float r0 = VisRenderer_randomU[j];
			const float r1 = VisRenderer_randomV[j];

			const float w = sqrt(1.f-r1);
			const float x = cos((2.f*M_PI)*r0)*w;
			const float y = sin((2.f*M_PI)*r0)*w;
			const float z = sqrt(r1)+epsilon;
			vec3f d = x*b0+y*b1+z*sample.ray.dir;
#else
			int r = (sample.sampleID.x * 9949 + sample.sampleID.y * 9613 + i * 9151) & 0xff;
			vec3f d = getRandomDir(r, b0, b1, sample.ray.dir, 0.001); //renderer->inherited.epsilon);
#endif

			continuation_rays[n_continuation_rays].sample.sampleID = sample.sampleID;
			continuation_rays[n_continuation_rays].sample.ray.org  = sample.ray.org;
			continuation_rays[n_continuation_rays].sample.ray.dir  = d;
			continuation_rays[n_continuation_rays].sample.ray.t		 = 0;
			continuation_rays[n_continuation_rays].sample.ray.t0	 = 0;
			continuation_rays[n_continuation_rays].sample.rgb.x		 = ambientColor.x;
			continuation_rays[n_continuation_rays].sample.rgb.y		 = ambientColor.y;
			continuation_rays[n_continuation_rays].sample.rgb.z		 = ambientColor.z;
			continuation_rays[n_continuation_rays].sample.alpha		 = 0.0;
			continuation_rays[n_continuation_rays].type 					 = AO;
			n_continuation_rays++;
		}
	}
}

void VisRenderer_renderFramePostamble(Renderer *uniform renderer, const uniform int32 accumID)
{ 
  if (renderer->fb) renderer->fb->accumID = accumID;  renderer->fb = NULL; 
}

void VisRenderer_renderFramePreamble(Renderer *uniform renderer, FrameBuffer *uniform framebuffer)
{ 
  renderer->fb = framebuffer; 
}

inline void addRGBAZ(uniform Tile &tile, const varying uint32 i, 
											 const varying vec3f rgb,
                     const varying float alpha,
                     const varying float z)
{
  tile.r[i] += rgb.x;
  tile.g[i] += rgb.y;
  tile.b[i] += rgb.z;
  tile.a[i] += alpha;
  tile.z[i] += z;
}

vec3f VisRenderer_computeTotalLambertianIntensity(VisRenderer *uniform renderer, vec3f point, vec3f normal)
{
	vec3f lightDirection;
	float lightDistance;

	vec3f totalRadiance = make_vec3f(0.0, 0.0, 0.0);
	for (uniform int i = 0; i < renderer->numLights; i++)
	{
		vec3f lightRadiance = renderer->lights[i]->computeRadiance(renderer->lights[i], point, lightDirection, lightDistance);

		float cosNL;
		if (isnan(normal.x + normal.y + normal.z))
			cosNL = 1.0f;
		else
		{
			cosNL = -dot(normalize(lightDirection), normalize(normal));			
			if (cosNL < 0) cosNL = -cosNL;
		}
		totalRadiance = totalRadiance + cosNL * lightRadiance;
	}
	return totalRadiance;
}

void VisRenderer_renderTile(uniform Renderer *uniform self, uniform Tile &tile)
{
#if GDA1
	print("====================================\n");
#endif

  uniform FrameBuffer *uniform fb     = self->fb;
  uniform Camera      *uniform camera = self->camera;

  float pixel_du = .5f, pixel_dv = .5f;
  float lens_du = 0.f,  lens_dv = 0.f;
  uniform int32 spp = self->spp;

  precomputeZOrder();
  
	ScreenSample screenSample;
	screenSample.sampleID.z = fb->accumID;
	screenSample.z = inf;
	screenSample.alpha = 0.f;

	CameraSample cameraSample;

	uniform ContinuationRay continuation_rays[AO_RAYS_PER_PIXEL*programCount];
	uniform int n_continuation_rays[programCount];

	for (uint32 I = 0; I < TILE_SIZE*TILE_SIZE; I += programCount)
	{
		const uint32 pixel = z_order.xs[I+programIndex] + (z_order.ys[I+programIndex] * TILE_SIZE);
		assert(pixel < TILE_SIZE*TILE_SIZE);

		screenSample.sampleID.x        = tile.region.lower.x + z_order.xs[I+programIndex];
		screenSample.sampleID.y        = tile.region.lower.y + z_order.ys[I+programIndex];

		n_continuation_rays[programIndex] = 0;

		if ((screenSample.sampleID.x < fb->size.x) & (screenSample.sampleID.y < fb->size.y)) 
		{
			cameraSample.screen.x = (screenSample.sampleID.x + pixel_du) * fb->rcpSize.x;
			cameraSample.screen.y = (screenSample.sampleID.y + pixel_dv) * fb->rcpSize.y;

			camera->initRay(camera, screenSample.ray, cameraSample);

			int n = 0;
			ContinuationRay ptr[AO_RAYS_PER_PIXEL];
			VisRenderer_renderSample(self, screenSample, n, ptr);

			n_continuation_rays[programIndex] = n;
			for (int j = 0; j < n; j++)
				continuation_rays[programIndex*AO_RAYS_PER_PIXEL + j] = ptr[j];

			n_continuation_rays[programIndex] = n;

			setRGBAZ(tile,pixel,screenSample.rgb,screenSample.alpha,screenSample.z);
		}

		int k = 0;
#if GDA3
		if (n_continuation_rays[programIndex])
			print("\nPIXEL % %\n", pixel, n_continuation_rays[programIndex]);
#endif
		for (uint32 j = 0; j <  n_continuation_rays[programIndex]; j++)
		{
			if (continuation_rays[AO_RAYS_PER_PIXEL*programIndex + j].type == AO)
			{
				ScreenSample s = continuation_rays[AO_RAYS_PER_PIXEL*programIndex + j].sample;
#if 0
				addRGBAZ(tile, pixel, s.rgb, 0.0, 0.0);
#else
				s.ray.t0 = 1e-6f;
				s.ray.t  = infinity;
				if (! VisRenderer_AO_intersect(self,  s))
				{
					addRGBAZ(tile, pixel, s.rgb, 0.0, 0.0);
					k = k + 1;
#if GDA3
					print("n");
#endif
				}
				else
				{
#if GDA3
					print("y");
#endif
				}
#endif
			}
		}
#if GDA3
		print("K %\n", k);
#endif
  }
}

// Test the origin point against the clipping planes to see if we start the ray in 
// a clipped region.  If so, we figure out the t where the ray resumes.

inline void VisRenderer_initializeClip(VisRenderer *uniform renderer, varying Ray &ray, varying int *slicesThatHaveBeenHit)
{
	// Assume no more than 32 slices
	float t[32];
  vec3f pt = ray.org + ray.t * ray.dir;

	float t_save = ray.t;
	float t1_save = ray.t1;

	// Note: sample is clipped if its behind any clipping plane
	// So visible intervals START with at the intersection point with the 
	// farthest front-facer and end with the closest back-facer.

  for (int i = 0; i < renderer->sliceCount; i++)
  {
		float cos_ang = dot(ray.dir, renderer->slicenorms[i]);
		if (cos_ang == 0)
			t[i] = ray.t1;
		else
		{
			t[i] = -(dot(ray.org, renderer->slicenorms[i]) + renderer->sliceds[i]) / cos_ang;
			if (t[i] < ray.t0) t[i] = ray.t0;
			else if (t[i] > ray.t1) t[i] = ray.t1;
		}

		if (renderer->sliceClips[i] == 1)
		{
			if (cos_ang < 0)
			{
				// Then plane is frontfacer and clips everything away in front of it.  Is 
				// it the farthest?

				if (t[i] > ray.t0) ray.t0 = t[i];
			}
			else if (cos_ang > 0)
			{
				// Then plane is backfacer and clips everything away in behind of it.  Is 
				// it the nearest?

				if (t[i] < ray.t1) ray.t1 = t[i];
			}
			else
			{
				// cos_ang == 0 means the ray will never intersect the plane.  So if the 
				// viewpoint is in front of it, the entire ray is clipped.  Otherwise,
				// the clip plane does not affect the ray.

				float d_viewpoint = dot(ray.org, renderer->slicenorms[i]);
				if (d_viewpoint < 0) 
					ray.t = ray.t1;
			}
		}
	}

	ray.t = ray.t0;

  // Now we have hit any slices that are before t0
	for (int i = 0; i < renderer->sliceCount; i++)
	  slicesThatHaveBeenHit[i] = (t[i] < ray.t);
}

inline void VisRenderer_intersectVisibleSlice(VisRenderer *uniform renderer, Volume *uniform volume, varying Ray &ray)
{
	// Find the closest OPAQUE slice

	float t,  nearest_t = infinity;
	int ni = -1;

#if GDA1
	print("\nIVS \nOX %\nOY %\nOZ %\nDX %\nDY %\nDZ %\n",
			ray.org.x, ray.org.y, ray.org.z,
			ray.dir.x, ray.dir.y, ray.dir.z);
#endif

  for (int i = 0; i < renderer->sliceCount; i++)
  {
#if GDA1
			print("==== slice %\nSA %\nSB %\nSC %\nSD %\n", i, renderer->slicenorms[i].x, renderer->slicenorms[i].y, renderer->slicenorms[i].z, renderer->sliceds[i]);
#endif
		
		if (renderer->sliceVisibility[i] == 1)
		{
			float d = dot(ray.dir, renderer->slicenorms[i]);
#if GDA1
			print("D %\n", d);
#endif

			if (d != 0.0)
			{
				float t = -(dot(ray.org, renderer->slicenorms[i]) + renderer->sliceds[i]) / d;
#if GDA1
				print("T %\n", t);
#endif
				if (t > 0)
				{
					if (t < nearest_t)
					{
						nearest_t = t;
						ni = i;
					}
				}
#if GDA1
				else
					print("t: %\n", t);
#endif
			}
#if GDA1
			else
				print("d: %\n", d);
#endif
		}
#if GDA1
			else
				print("NOT VISIBLE\n");
#endif
	}

	ray.t = nearest_t;
#if GDA1
	print("NT %\nI %\n", nearest_t, ni);
#endif
}

inline void VisRenderer_computeSliceSample(VisRenderer *uniform renderer,
                                                      Volume *uniform volume,
                                                      varying Ray &ray,
																											varying vec3f &ambientColor,
																											varying vec4f &lambertianColor,
																											varying int *slicesThatHaveBeenHit,
																											varying int &sliceThatWasHit)
{
	// Find the closest slice

	float t,  nearest_t = infinity;
  sliceThatWasHit = -1;
  for (int i = 0; i < renderer->sliceCount; i++)
  {
		// Ignore any slices we've already hit
		if (slicesThatHaveBeenHit[i]) 
				continue;

    float d = dot(ray.dir, renderer->slicenorms[i]);
    if (d != 0.0)
		{
      float t = -(dot(ray.org, renderer->slicenorms[i]) + renderer->sliceds[i]) / d;
			if (t >= ray.t && t <= ray.t1)
			{
				if (t < nearest_t)
				{
					sliceThatWasHit = i;
					nearest_t = t;
				}
			}
		}
  }

	// If we didn't hit a slice, OK

	if (sliceThatWasHit == -1)
	{
		ray.t = infinity;
		return;
	}

	slicesThatHaveBeenHit[sliceThatWasHit] = true;

	ray.t = nearest_t;

	// If this slice is visible...

	if (renderer->sliceVisibility[sliceThatWasHit] == 1)
	{
		varying vec3f hit = ray.org + nearest_t * ray.dir;

		// Convert world-space hit point to local grid space and test against grid bounds

		varying vec3f lHit;
		StructuredVolume *uniform svolume = (StructuredVolume *uniform) volume;
		svolume->transformWorldToLocal(svolume, hit, lHit);

		if (((lHit.x >= 0.0) && (lHit.x <= svolume->dimensions.x)) &&
			  ((lHit.y >= 0.0) && (lHit.y <= svolume->dimensions.y)) && 
			  ((lHit.z >= 0.0) && (lHit.z <= svolume->dimensions.z)))
		{
			const float sample = volume->computeSample(volume, hit);
			const vec3f sampleColor = volume->transferFunction->getColorForValue(volume->transferFunction, sample);

			float opacity = volume->transferFunction->getOpacityForValue(volume->transferFunction, sample);
			opacity = 1.0;

			vec3f totalRadiance = opacity * sampleColor * VisRenderer_computeTotalLambertianIntensity(renderer, hit, renderer->slicenorms[sliceThatWasHit]);

			ambientColor    = renderer->ambient * opacity * sampleColor;

			vec3f t = (1.f - renderer->ambient) * totalRadiance;
			lambertianColor = make_vec4f(t.x, t.y, t.z, opacity);
    }
  }
}

inline void VisRenderer_computeVolumeSample(VisRenderer *uniform renderer,
                                                      Volume *uniform volume,
                                                      varying Ray &ray,
                                                      varying vec4f &color)
{
  //! Advance the ray.
  volume->intersect(volume, ray);  if (ray.t > ray.t1) return;

	vec3f coordinates = ray.org + ray.t * ray.dir;

  //! Sample the volume at the hit point in world coordinates.
  const float sample = volume->computeSample(volume, coordinates);

  //! Look up the color associated with the volume sample.
  vec3f sampleColor = volume->transferFunction->getColorForValue(volume->transferFunction, sample);

  //! Compute gradient shading, if enabled.
  if (volume->gradientShadingEnabled) {

    //! Constant ambient lighting term.

    //! Use volume gradient as the normal.
    const vec3f gradient = normalize(volume->computeGradient(volume, coordinates));
		vec3f totalRadiance = VisRenderer_computeTotalLambertianIntensity(renderer, coordinates, gradient);
			
    const float ambient = renderer->ambient;
		sampleColor = sampleColor * (ambient + ((1.f - ambient)* totalRadiance));
  }

  //! Look up the opacity associated with the volume sample.
  const float sampleOpacity = volume->transferFunction->getOpacityForValue(volume->transferFunction, sample);

  //! Set the color contribution for this sample only (do not accumulate).
  color = clamp(sampleOpacity / volume->samplingRate) * make_vec4f(sampleColor.x, sampleColor.y, sampleColor.z, 1.0f);
}

inline void VisRenderer_intersectIsosurface(VisRenderer *uniform renderer, Volume *uniform volume, varying Ray &ray)
{
	print("VisRenderer_intersectIsosurface % % %\n", ray.t0, ray.t1, ray.t);
  //! Terminate if there are no isovalues.
  if (volume->numIsovalues == 0) {
    ray.t = infinity;
#if GDA3
 		print("X0\n");
#endif
    return;
  }

  //! Advance the ray.
  volume->intersectIsosurface(volume, volume->isovalues, volume->numIsovalues, ray); 
	if (ray.t > ray.t1)
	{
#if GDA3
 		print("X1 % %\n", ray.t, ray.t1);
#endif
		return;
	}

  //! Sample the volume at the hit point in world coordinates.

  float t0 = ray.t;
  float sample0 = volume->computeSample(volume, ray.org + ray.t * ray.dir);

  while(1) {

    //! Advance the ray.
    volume->intersectIsosurface(volume, volume->isovalues, volume->numIsovalues, ray);  
		if (ray.t > ray.t1) 
		{
#if GDA3
			print("X2 % %\n", ray.t, ray.t1);
#endif
			return;
		}

    float t = ray.t;
    float sample = volume->computeSample(volume, ray.org);

    //! Find the t value and record the isovalue for the first isosurface intersection.

    float tHit = infinity;
    if (!isnan(sample0+sample))
		{
      for (uniform int i=0; i<volume->numIsovalues; i++)
			{
        if ((volume->isovalues[i] - sample0) * (volume->isovalues[i] - sample) <= 0.f) {
          const float tIso = t0 + (volume->isovalues[i] - sample0) / (sample - sample0) * (t - t0);

          if (tIso < tHit) tHit = tIso;
        }
      }
    }

    if (tHit <= ray.t1)
		{
			print("AO Y %\n", tHit);
			ray.t = tHit;
      return;
    }
		else
			print("AO N %\n", tHit);

    t0 = t;
    sample0 = sample;
  }

#if GDA3
  print("X3\n");
#endif
}

inline void VisRenderer_computeIsosurfaceSample(VisRenderer *uniform renderer,
                                                          Volume *uniform volume,
                                                          varying Ray &ray,
                                                          varying vec3f &ambientColor,
                                                          varying vec4f &lambertianColor,
																													varying vec3f &normal)
{
  //! Terminate if there are no isovalues.
  if (volume->numIsovalues == 0) {
    ray.t = infinity;
    return;
  }

  //! Advance the ray.
  volume->intersectIsosurface(volume, volume->isovalues, volume->numIsovalues, ray); 
	if (ray.t > ray.t1) return;

  //! Sample the volume at the hit point in world coordinates.
  float t0 = ray.t;
  float sample0 = volume->computeSample(volume, ray.org + ray.t * ray.dir);

  while(1) {

    //! Advance the ray.
    volume->intersectIsosurface(volume, volume->isovalues, volume->numIsovalues, ray);  
		if (ray.t > ray.t1) 
		{
			return;
		}

    float t = ray.t;
    float sample = volume->computeSample(volume, ray.org + ray.t * ray.dir);

    //! Find the t value and record the isovalue for the first isosurface intersection.
    float tHit = infinity;
    float isovalueHit;

    if (!isnan(sample0+sample)) {
      for (uniform int i=0; i<volume->numIsovalues; i++) {
        if ((volume->isovalues[i] - sample0) * (volume->isovalues[i] - sample) <= 0.f) {
          const float tIso = t0 + (volume->isovalues[i] - sample0) / (sample - sample0) * (t - t0);

          if (tIso < tHit) {
            tHit = tIso;
            isovalueHit = volume->isovalues[i];
          }
        }
      }
    }

    if (tHit <= ray.t1)
		{
      //! Isosurface hit point.
      const vec3f coordinates = ray.org + tHit * ray.dir;

      //! Look up the color associated with the isovalue.
      vec3f sampleColor = volume->transferFunction->getColorForValue(volume->transferFunction, isovalueHit);

			//! Use volume gradient as the normal.
			normal = normalize(volume->computeGradient(volume, coordinates));

      //! Assume fully opaque isosurfaces for now.
      const float opacity = 1.f;

			ambientColor    = renderer->ambient * opacity * sampleColor;

			vec3f t = (1.f - renderer->ambient) * opacity * sampleColor * VisRenderer_computeTotalLambertianIntensity(renderer, coordinates, normal);
			lambertianColor = make_vec4f(t.x, t.y, t.z, opacity);

#if GDA3
			print("I:\nA %\nNX %\nNY %\nNZ %\nAX %\nAY %\nAZ %\nLX %\nLY %\nLZ %\n", renderer->ambient, 
						normal.x, normal.y, normal.z,
						ambientColor.x, ambientColor.y, ambientColor.z,
						lambertianColor.x, lambertianColor.y, lambertianColor.z);
#endif

      ray.t = tHit;
      return;
    }

    t0 = t;
    sample0 = sample;
  }
}

inline void VisRenderer_intersectGeometry(VisRenderer *uniform renderer, varying Ray &ray)
{
  Ray dynamicRay = ray;
  traceRay(renderer->dynamicModel, dynamicRay);
  traceRay(renderer->model, ray);

  if(dynamicRay.t < ray.t) {
    ray.t = dynamicRay.t;
  }
}

inline void VisRenderer_computeGeometrySample(VisRenderer *uniform renderer,
                                                        varying Ray &ray,
                                                        varying vec4f &color)
{
  //! We compute intersections on both the model and dynamicModel, and provide the contribution for the closest hit.
  bool hitDynamicModel = false;

  Ray dynamicRay = ray;
  traceRay(renderer->dynamicModel, dynamicRay);

  traceRay(renderer->model, ray);

  if(dynamicRay.t < ray.t) {
    hitDynamicModel = true;
    ray = dynamicRay;
  }

  //! No hit found.
  if(ray.geomID < 0) {
    ray.t = infinity;
    return;
  }

  //! Post intersect on the hit geometry.
  DifferentialGeometry dg;

  if(hitDynamicModel)
    postIntersect(renderer->dynamicModel, dg, ray, DG_NG|DG_NS|DG_NORMALIZE|DG_FACEFORWARD|DG_MATERIALID|DG_COLOR|DG_TEXCOORD);
  else
    postIntersect(renderer->model, dg, ray, DG_NG|DG_NS|DG_NORMALIZE|DG_FACEFORWARD|DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

  //! Color of the geometry.
  vec3f sampleColor = make_vec3f(dg.color.x, dg.color.y, dg.color.z);

  //! Default opacity of 1.
  float sampleOpacity = 1.0f;

  //! If no color found in the geometry, map through transfer function.
  if(sampleColor.x < 0.f) {

    //! Coordinates of the geometry hit.
    const vec3f geometryCoordinates = ray.org + ray.t * ray.dir;

    //! Sample the volume.
    const float sample = renderer->model->volumes[0]->computeSample(renderer->model->volumes[0], geometryCoordinates);

    //! Look up the color associated with the volume sample. NaN values will have a color of (0,0,0) and a 0 opacity.
    sampleColor = isnan(sample) ? sampleOpacity = 0.f, make_vec3f(0.f) : renderer->model->volumes[0]->transferFunction->getColorForValue(renderer->model->volumes[0]->transferFunction, sample);
  }

  //! Compute lighting.
  vec3f lightDirection;
  float lightDistance;
  vec3f lightRadiance = renderer->lights[0]->computeRadiance(renderer->lights[0], dg.P, lightDirection, lightDistance);

  //! Constant ambient lighting term.
  const float ambient = renderer->ambient;

  float cosNL = abs(dot(normalize(lightDirection), normalize(dg.Ns)));
	if (cosNL < 0) cosNL = -cosNL;

  sampleColor = sampleColor * (ambient + cosNL*(1.f-ambient)) * lightRadiance;

  //! Set the color contribution for this sample only (do not accumulate).
  color = sampleOpacity * make_vec4f(sampleColor.x, sampleColor.y, sampleColor.z, 1.0f);
}

inline void VisRenderer_intersectBox(const uniform box3f &box, 
                                               varying Ray &ray) 
{
  //! Intersection interval minimum per axis.
  const vec3f minimum = rcp(ray.dir) * (box.lower - ray.org);

  //! Intersection interval maximum per axis.
  const vec3f maximum = rcp(ray.dir) * (box.upper - ray.org);

  //! Distance along the ray to the entry point.
  ray.t0 = max(max(ray.t0, min(minimum.x, maximum.x)), max(min(minimum.y, maximum.y), min(minimum.z, maximum.z)));

  //! Distance along the ray to the exit point.
  ray.t1 = min(min(ray.t, max(minimum.x, maximum.x)), min(max(minimum.y, maximum.y), max(minimum.z, maximum.z)));
}

inline void VisRenderer_AO_intersectBox(const uniform box3f &box, 
                                               varying Ray &ray) 
{
  //! Intersection interval minimum per axis.
  vec3f minimum = rcp(ray.dir) * (box.lower - ray.org);
  vec3f maximum = rcp(ray.dir) * (box.upper - ray.org);

	if (ray.dir.x == 0) minimum.x = maximum.x = infinity;
	if (ray.dir.y == 0) minimum.y = maximum.y = infinity;
	if (ray.dir.z == 0) minimum.z = maximum.z = infinity;

  //! Distance along the ray to the entry point.
  ray.t0 = 0.0;
  ray.t1 = max(max(ray.t0, min(minimum.x, maximum.x)), max(min(minimum.y, maximum.y), min(minimum.z, maximum.z)));

#if 0
	if (ray.t1 < 0)
	{
	print("AOIB\nlower\nx %\ny %\nz %\nupper\nx %\ny %\nz %\norg\nx %\ny %\nz %\ndir\nx %\ny %\nz %\n",
			box.lower.x, box.lower.y, box.lower.z,
			box.upper.x, box.upper.y, box.upper.z,
			ray.org.x, ray.org.y, ray.org.z,
			ray.dir.x, ray.dir.y, ray.dir.z);
	print("min:\nX %\nY %\nZ %\n", minimum.x, minimum.y, minimum.z);
	print("max:\nX %\nY %\nZ %\n", maximum.x, maximum.y, maximum.z);
	print("T0 %\nT1 %\n", ray.t0, ray.t1);
	}
#endif
#if 0
	print("AOIB\norg\nx %\ny %\nz %\ndir\nx %\ny %\nz %\n",
			ray.org.x, ray.org.y, ray.org.z,
			ray.dir.x, ray.dir.y, ray.dir.z);
	print("T0 %\nT1 %\n", ray.t0, ray.t1);
	}
#endif
}

//==================================================

//! This function intersects the volume and geometries.
inline bool VisRenderer_intersect(uniform VisRenderer *uniform renderer,
                                            varying ScreenSample &screenSample,
                                            const varying float &rayOffset,
                                            varying vec4f &color,
																						varying int &n_continuation_rays, 
																						varying ContinuationRay *continuation_rays)
{
	varying Ray &ray = screenSample.ray;

	// Assume no more than 32(?) slices
	varying int slicesThatHaveBeenHit[32];

  //! Assume just one volume.

  //! Bounding box of the volume.
  const uniform box3f boundingBox = renderer->model->volumes[0]->boundingBox;

  //! Ray epsilon.
  const uniform float epsilon = 1e-4 * distance(boundingBox.lower, boundingBox.upper);

  //! Compute the intersection interval over the ray and volume bounds.
  VisRenderer_intersectBox(boundingBox, ray);

	// If the exit point is in front of the entry point (say, what?) or the ray hits a clipping before the 
	// volume itself, quit.   This is a ray termination.
  if (ray.t0 > ray.t1)
    return true;

  ray.t = ray.t0;

	// Lop off any part of the ray.t -> ray.t1 interval thats
	// clipped away.

	VisRenderer_initializeClip(renderer, ray, slicesThatHaveBeenHit);

  //! Maximum extent for the volume bounds.
  const float tMax = ray.t1;

  //! Offset ray by a fraction of the nominal ray step.
  const uniform float step = renderer->model->volumes[0]->samplingStep / renderer->model->volumes[0]->samplingRate;
  ray.t += 0.01 * step;

  //! Copy of the ray for volume isosurface intersection. The original ray is used for volume intersection.
  Ray isosurfaceRay = ray;
  isosurfaceRay.t = ray.t0 + rayOffset * renderer->model->volumes[0]->samplingStep; // don't consider sampling rate, but still allow offset.
  isosurfaceRay.primID = -1;
  isosurfaceRay.geomID = -1;
  isosurfaceRay.instID = -1;

  //! Copy of the ray for geometry intersection.
  Ray geometryRay = isosurfaceRay;
  geometryRay.t = tMax; //! end of valid ray interval for traceRay().

  //! Copy of the ray for slice intersection.
  Ray sliceRay = isosurfaceRay;
  sliceRay.t = ray.t0;

  //! Separate color contributions for the volume, isosurfaces, and geometries. 
	// Initialize to provided color in case of no contribution...
  vec4f volumeColor = color;
  vec4f isosurfaceColor = color;
  vec4f geometryColor = color;
  vec4f sliceColor = color;

	vec3f isosurfaceNormal;
	vec4f isosurfaceLambertian;
	vec3f isosurfaceAmbient;

	vec4f sliceLambertian;
	vec3f sliceAmbient;
	int sliceThatWasHit;

  //! Initial trace through the volume and geometries.
  VisRenderer_computeVolumeSample(renderer, renderer->model->volumes[0], ray, volumeColor);
  VisRenderer_computeIsosurfaceSample(renderer, renderer->model->volumes[0], isosurfaceRay, isosurfaceAmbient, isosurfaceLambertian, isosurfaceNormal);
#if GDA
	print("CIS\nR %\nG %\nB%\n", isosurfaceAmbient.x, isosurfaceAmbient.y, isosurfaceAmbient.z);
#endif

  VisRenderer_computeGeometrySample(renderer, geometryRay, geometryColor);
  VisRenderer_computeSliceSample(renderer, renderer->model->volumes[0], sliceRay, sliceAmbient, sliceLambertian, slicesThatHaveBeenHit, sliceThatWasHit);
#if 0
	print("CSS\nAR %\nAG %\nAB%\n", sliceAmbient.x, sliceAmbient.y, sliceAmbient.z);
	print("LR %\nLG %\nLB %\n", sliceLambertian.x, sliceLambertian.y, sliceLambertian.z);
#endif

  //! Trace the ray through the volume and geometries.
  float firstHit;

	int k = 0;

  while ((firstHit = min(min(min(ray.t, sliceRay.t), isosurfaceRay.t), geometryRay.t)) < tMax && 
					min(min(color.x, color.y), color.z) < 1.0f && color.w < 0.99f)
	{
    if (firstHit == ray.t) {

      //! Volume contribution.
      color = color + (1.0f - color.w) * volumeColor;

      //! Trace next volume ray.
      VisRenderer_computeVolumeSample(renderer, renderer->model->volumes[0], ray, volumeColor);
    }

// TODO - hey I think this skips the last interval of volume in front of surfaces!

    else if (firstHit == isosurfaceRay.t) {

#if GDA3
			print("HI\n");
#endif

      color = color + (1.0f - color.w) * isosurfaceLambertian;

			screenSample.ray.t = isosurfaceRay.t;

			ScreenSample hitPoint = screenSample;
			hitPoint.ray.org = screenSample.ray.org + (screenSample.ray.t * screenSample.ray.dir) + (0.01 * isosurfaceNormal);
			hitPoint.ray.dir = isosurfaceNormal;

			generateAORays(renderer, hitPoint, isosurfaceAmbient, renderer->inherited.epsilon, n_continuation_rays, continuation_rays);

			if (min(min(color.x, color.y), color.z) >= 1.0f || color.w >= 0.99f)
			{
				// This is a termination.
				return true;
			}

      //! Reset isosurface ray.
      isosurfaceRay.t = isosurfaceRay.t + epsilon;
      isosurfaceRay.primID = -1;
      isosurfaceRay.geomID = -1;
      isosurfaceRay.instID = -1;

      //! Trace next isosurface ray.
			VisRenderer_computeIsosurfaceSample(renderer, renderer->model->volumes[0], isosurfaceRay, isosurfaceAmbient, isosurfaceLambertian, isosurfaceNormal);
    }
    else if (firstHit == geometryRay.t) {

      //! Geometry contribution.
      color = color + (1.0f - color.w) * geometryColor;
			if (min(min(color.x, color.y), color.z) >= 1.0f || color.w >= 0.99f)
			{
#if GDA
				print("g");
#endif
				// This is a termination.
				return true;
			}

      //! Reset geometry ray.
      geometryRay.t0 = geometryRay.t + epsilon;
      geometryRay.t = tMax; //! end of valid ray interval for traceRay()
      geometryRay.primID = -1;
      geometryRay.geomID = -1;
      geometryRay.instID = -1;

      //! Trace next geometry ray.
      VisRenderer_computeGeometrySample(renderer, geometryRay, geometryColor);
    }
    else if (firstHit == sliceRay.t) {

#if GDA3
			print("HS\n");
#endif
      color = color + (1.0f - color.w) * sliceLambertian;

			screenSample.ray.t = sliceRay.t;

			ScreenSample hitPoint = screenSample;
			hitPoint.ray.org = screenSample.ray.org + (screenSample.ray.t * screenSample.ray.dir) + (0.01 * renderer->slicenorms[sliceThatWasHit]);
			hitPoint.ray.dir = renderer->slicenorms[sliceThatWasHit];
			generateAORays(renderer, hitPoint, sliceAmbient, renderer->inherited.epsilon, n_continuation_rays, continuation_rays);


			if (min(min(color.x, color.y), color.z) >= 1.0f || color.w >= 0.99f)
			{
#if GDA
				print("s");
#endif
				// This is a termination.
				return true;
			}

			//! Reset slice ray.
			sliceRay.t = sliceRay.t + epsilon;
			sliceRay.primID = -1;
			sliceRay.geomID -1;
			sliceRay.instID = -1;

			//! Trace next slice ray.
			VisRenderer_computeSliceSample(renderer, renderer->model->volumes[0], sliceRay, sliceAmbient, sliceLambertian, slicesThatHaveBeenHit, sliceThatWasHit);
		}
  }

	if (firstHit >= tMax)
	{
#if GDA
		print("x firstHit %\n", firstHit);
		print("x tMax %\n", tMax);
#endif

		// This will be the continuation of a primary ray.

		continuation_rays[n_continuation_rays].sample.sampleID = screenSample.sampleID;
		continuation_rays[n_continuation_rays].sample.ray = ray;
		continuation_rays[n_continuation_rays].sample.rgb.x = color.x;
		continuation_rays[n_continuation_rays].sample.rgb.y = color.y;
		continuation_rays[n_continuation_rays].sample.rgb.z = color.z;
		continuation_rays[n_continuation_rays].sample.alpha = color.w;
		continuation_rays[n_continuation_rays].type = Primary;
		n_continuation_rays++;

		// NOT a termination - hit the back of the partition
		return false;
	}
	else
	{
		// Broke out due to opacity - volume accumulation, I guess
		return true;
		print("y");
	}
}

void VisRenderer_renderSample(Renderer *uniform pointer, varying ScreenSample &sample, 
												varying int &n_continuation_rays, varying ContinuationRay *continuation_rays)
{

  //! Cast to the actual Renderer subtype.
  VisRenderer *uniform renderer = (VisRenderer *uniform) pointer;

  //! Background color.
  const vec4f background = make_vec4f(0.0f, 0.0f, 0.0f, 1.0f);

  //! Ray offset for this sample, as a fraction of the nominal step size.
  float rayOffset = precomputedHalton2(sample.sampleID.z);
  int ix = sample.sampleID.x % 4;
  int iy = sample.sampleID.y % 4;
  int patternID = ix + 4 * iy;
  rayOffset += precomputedHalton3(patternID);

  if(rayOffset > 1.f) rayOffset -= 1.f;

  //! Provide the renderer to the intersector as it contains all volumes, geometries, etc.
  vec4f color = make_vec4f(0.0f);

	// Intersect will return true if the ray resulted in something that needs to be added into the
  // frame buffer.   If the ray terminated in the current partition it will.
  // If it continued pass into the next partition, it won't. Note: we've moved the gamma correnction
	// and the merging of the background until the end of the whole raycasting process (since we won't
	// know the final opacity and color till then).

  if (VisRenderer_intersect(renderer, sample, rayOffset, color, n_continuation_rays, continuation_rays))
	{
		sample.rgb.x = color.x;
		sample.rgb.y = color.y;
		sample.rgb.z = color.z;
		sample.alpha = color.w;
	}
	else
	{
		sample.rgb.x = 0.0;
		sample.rgb.y = 0.0;
		sample.rgb.z = 0.0;
		sample.alpha = 0.0;
	}
}

//=======================================================

export void VisRenderer_setCamera(void *uniform pointer, void *uniform camera) 
{
  //! Cast to the actual Renderer subtype.
  VisRenderer *uniform renderer = (VisRenderer *uniform) pointer;

  //! Set the camera view to render.
  renderer->inherited.camera = (Camera *uniform) camera;
}

export void VisRenderer_setLights(void *uniform pointer, void **uniform lights) 
{
  //! Cast to the actual Renderer subtype.
  VisRenderer *uniform renderer = (VisRenderer *uniform) pointer;

	if (lights)
	{
		//! Set the light sources.
		renderer->lights = (Light **uniform) lights;

		uniform int i;
		for (i = 0; renderer->lights[i] != NULL; i++);
		renderer->numLights = i;
	}
	else
		renderer->numLights = 0;
}

export void VisRenderer_setModel(void *uniform pointer, 
                                           void *uniform model) 
{
  //! Cast to the actual Renderer subtype.
  VisRenderer *uniform renderer = (VisRenderer *uniform) pointer;

  //! Set the model to be rendered.
  renderer->model = (Model *uniform) model;
}

export void VisRenderer_setDynamicModel(void *uniform pointer, 
                                                  void *uniform model) 
{
  //! Cast to the actual Renderer subtype.
  VisRenderer *uniform renderer = (VisRenderer *uniform) pointer;

  //! Set the model to be rendered.
  renderer->dynamicModel = (Model *uniform) model;
}

export void VisRenderer_set_ambient(void *uniform pointer, uniform float a)
{
  VisRenderer *uniform visRenderer = (VisRenderer *uniform) pointer;
	visRenderer->ambient = a;
}

export void VisRenderer_set_AO_number(void *uniform pointer, uniform int n)
{
  VisRenderer *uniform visRenderer = (VisRenderer *uniform) pointer;
	visRenderer->numAO = n;
}

export void VisRenderer_set_AO_radius(void *uniform pointer, uniform float r)
{
  VisRenderer *uniform visRenderer = (VisRenderer *uniform) pointer;
	visRenderer->AOradius = r;
}

export void VisRenderer_setSlices(void *uniform pointer, 
			const uniform size_t &count, vec4f *uniform planes, 
			int *uniform clips, int *uniform visible)
{
  VisRenderer *uniform visRenderer = (VisRenderer *uniform) pointer;
  if (visRenderer->slicenorms != NULL) delete[] visRenderer->slicenorms;
  if (visRenderer->sliceds != NULL) delete[] visRenderer->sliceds;
  if (visRenderer->sliceVisibility != NULL) delete[] visRenderer->sliceVisibility;
  if (visRenderer->sliceClips != NULL) delete[] visRenderer->sliceClips;
  visRenderer->sliceCount = count;
  if (count)
  {
    visRenderer->slicenorms = uniform new uniform vec3f[count];
    visRenderer->sliceds = uniform new uniform float[count];
    visRenderer->sliceVisibility = uniform new uniform int[count];
    visRenderer->sliceClips = uniform new uniform int[count];
    for (uniform size_t i = 0; i < count; i++)
    {
      visRenderer->sliceVisibility[i] = visible[i];
      visRenderer->sliceClips[i] = clips[i];
      visRenderer->slicenorms[i] = make_vec3f(planes[i].x, planes[i].y, planes[i].z);
      visRenderer->sliceds[i] = planes[i].w;
    }
  }
  else
  {
    visRenderer->slicenorms = NULL;
    visRenderer->sliceds = NULL;
    visRenderer->sliceClips = NULL;
    visRenderer->sliceVisibility = NULL;
  }
}

//! This function intersects the volume and geometries.
bool VisRenderer_AO_intersect(uniform Renderer *uniform pointer, varying ScreenSample &screenSample)
{
#if GDA3
	print ("AO INTERSECT\n");
#endif
  VisRenderer *uniform renderer = (VisRenderer *uniform) pointer;
	varying Ray &ray = screenSample.ray;

  //! Bounding box of the volume.
  const uniform box3f boundingBox = renderer->model->volumes[0]->boundingBox;

  //! Ray epsilon.
  const uniform float epsilon = 1e-4 * distance(boundingBox.lower, boundingBox.upper);

  //! Compute the intersection interval over the ray and volume bounds.
  VisRenderer_AO_intersectBox(boundingBox, ray);

	// If the exit point is in front of the entry point (say, what?)
	// then quit.   This is a ray termination.
  if (ray.t0 > ray.t1)
	{
		print("X 0\n");
    return true;
	}

  const float tMax = ray.t1;

  //! Offset ray by a fraction of the nominal ray step.
  const uniform float step = renderer->model->volumes[0]->samplingStep / renderer->model->volumes[0]->samplingRate;
  ray.t += 0.01 * step;

  //! Copy of the ray for volume isosurface intersection. The original ray is used for volume intersection.
  Ray isosurfaceRay = ray;
	isosurfaceRay.t1 = renderer->AOradius;
#if GDA3
  VisRenderer_intersectIsosurface(renderer, renderer->model->volumes[0], isosurfaceRay);
	if (isosurfaceRay.t < renderer->AOradius)
	{
#if GDA3
		print("i %\n", isosurfaceRay.t);
#endif
		return true;
	}
#endif

#if 0
  //! Copy of the ray for geometry intersection.
  Ray geometryRay = isosurfaceRay;
  geometryRay.t = tMax; //! end of valid ray interval for traceRay().
  VisRenderer_intersectGeometry(renderer, geometryRay);
	if (geometryRay.t < renderer->AOradius)
	{
		print("g");
		return true;
	}
#endif

  //! Copy of the ray for slice intersection.
  Ray sliceRay = isosurfaceRay;
  sliceRay.t = ray.t0;
  VisRenderer_intersectVisibleSlice(renderer, renderer->model->volumes[0], sliceRay);
	if (sliceRay.t < renderer->AOradius)
	{
#if GDA1
		print("s %\n", sliceRay.t);
#endif
    return true;
	}

	return false;
}

export void *uniform VisRenderer_createInstance()
{
  //! The renderer object.
  VisRenderer *uniform renderer = uniform new uniform VisRenderer;

  renderer->slicenorms = NULL;
  renderer->sliceds = NULL;
  renderer->sliceVisibility = NULL;
  renderer->sliceClips = NULL;
  renderer->sliceCount = 0;

  //! Constructor of the parent class.
  Renderer_Constructor(&renderer->inherited, NULL);

  //! Function to perform per-frame state initialization.
  renderer->inherited.beginFrame = VisRenderer_renderFramePreamble;

  //! Function to perform per-frame state completion.
  renderer->inherited.endFrame = VisRenderer_renderFramePostamble;  

	renderer->inherited.renderTile = VisRenderer_renderTile;

	return(renderer);
}

