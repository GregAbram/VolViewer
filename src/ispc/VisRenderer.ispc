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

void VisRenderer_renderSample(Renderer *uniform pointer, varying ScreenSample &sample, 
				varying int &n_continuation_rays, varying ScreenSample *continuation_rays);

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

vec3f VisRenderer_computeTotalLambertialIntensity(VisRenderer *uniform renderer, vec3f point, vec3f normal)
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
			cosNL = dot(normalize(lightDirection), normalize(normal));			
			if (cosNL < 0) cosNL = 0.0;
		}
		totalRadiance = totalRadiance + cosNL * lightRadiance;
	}
	return totalRadiance;
}

void VisRenderer_renderTile(uniform Renderer *uniform self, uniform Tile &tile)
{
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

	for (uint32 i=programIndex;i<TILE_SIZE*TILE_SIZE;i+=programCount) {

		const uint32 pixel = z_order.xs[i] + (z_order.ys[i] * TILE_SIZE);
		assert(pixel < TILE_SIZE*TILE_SIZE);

		screenSample.sampleID.x        = tile.region.lower.x + z_order.xs[i];
		screenSample.sampleID.y        = tile.region.lower.y + z_order.ys[i];

		if ((screenSample.sampleID.x >= fb->size.x) | (screenSample.sampleID.y >= fb->size.y)) 
			continue;

		cameraSample.screen.x = (screenSample.sampleID.x + pixel_du) * fb->rcpSize.x;
		cameraSample.screen.y = (screenSample.sampleID.y + pixel_dv) * fb->rcpSize.y;

		camera->initRay(camera, screenSample.ray, cameraSample);

		varying ScreenSample continuation_rays[32]; varying int n_continuation_rays;
		VisRenderer_renderSample(self, screenSample, n_continuation_rays, continuation_rays);

		setRGBAZ(tile,pixel,screenSample.rgb,screenSample.alpha,screenSample.z);
  }
}

// Test the origin point against the clipping planes to see if we start the ray in 
// a clipped region.  If so, we figure out the t where the ray resumes.

inline void VisRenderer_initializeClip(VisRenderer *uniform renderer, varying Ray &ray, varying int *hit)
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
	  hit[i] = (t[i] < ray.t);
}

inline void VisRenderer_computeSliceSample(VisRenderer *uniform renderer,
                                                      Volume *uniform volume,
                                                      varying Ray &ray,
                                                      varying vec4f &color,
																											varying int *hit)
{
	// Find the closest slice

	float t,  nearest_t = infinity;
  varying int sliceHit = -1;
  for (int i = 0; i < renderer->sliceCount; i++)
  {
		// Ignore any slices we've already hit
		if (hit[i]) 
				continue;

    float d = dot(ray.dir, renderer->slicenorms[i]);
    if (d != 0.0)
		{
      float t = -(dot(ray.org, renderer->slicenorms[i]) + renderer->sliceds[i]) / d;
			if (t >= ray.t && t <= ray.t1)
			{
				if (t < nearest_t)
				{
					sliceHit = i;
					nearest_t = t;
				}
			}
		}
  }

	// If we didn't hit a slice, OK

	if (sliceHit == -1)
	{
		ray.t = infinity;
		return;
	}

	hit[sliceHit] = true;

	ray.t = nearest_t;

	// If this slice is visible...

	if (renderer->sliceVisibility[sliceHit] == 1)
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
			const float opacity = volume->transferFunction->getOpacityForValue(volume->transferFunction, sample);

			vec3f totalRadiance = VisRenderer_computeTotalLambertialIntensity(renderer, hit, renderer->slicenorms[sliceHit]);
			
			const float ambient = 0.1f;
			const vec3f litColor = sampleColor * (ambient + ((1.f - ambient)* totalRadiance));

			color = make_vec4f(litColor.x, litColor.y, litColor.z, 1.0);
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
		vec3f totalRadiance = VisRenderer_computeTotalLambertialIntensity(renderer, coordinates, gradient);
			
    const float ambient = 0.1f;
		sampleColor = sampleColor * (ambient + ((1.f - ambient)* totalRadiance));
  }

  //! Look up the opacity associated with the volume sample.
  const float sampleOpacity = volume->transferFunction->getOpacityForValue(volume->transferFunction, sample);

  //! Set the color contribution for this sample only (do not accumulate).
  color = clamp(sampleOpacity / volume->samplingRate) * make_vec4f(sampleColor.x, sampleColor.y, sampleColor.z, 1.0f);
}

inline void VisRenderer_computeIsosurfaceSample(VisRenderer *uniform renderer,
                                                          Volume *uniform volume,
                                                          varying Ray &ray,
                                                          varying vec4f &color)
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
			const vec3f gradient = normalize(volume->computeGradient(volume, coordinates));

			vec3f totalRadiance = VisRenderer_computeTotalLambertialIntensity(renderer, coordinates, gradient);

      const float ambient = 0.1f;
      sampleColor = sampleColor * (ambient + ((1.f-ambient) * totalRadiance));

      //! Assume fully opaque isosurfaces for now.
      const float sampleOpacity = 1.f;

      //! Set the color contribution for this sample only (do not accumulate).
      color = sampleOpacity * make_vec4f(sampleColor.x, sampleColor.y, sampleColor.z, 1.0f);

			// color = make_vec4f(1.0, 0.0, 0.0, 1.0);

      ray.t = tHit;

      //! Only one hit at a time.
      return;
    }

    t0 = t;
    sample0 = sample;
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
  const float ambient = 0.5f;

  const float cosNL = abs(dot(normalize(lightDirection), normalize(dg.Ns)));

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

//! This function intersects the volume and geometries.
inline void VisRenderer_intersect(uniform VisRenderer *uniform renderer,
                                            varying Ray &ray,
                                            const varying float &rayOffset,
                                            varying vec4f &color,
																						varying int &n_continuation_rays, 
																						varying ScreenSample *continuation_rays)
{
	// Assume no more than 32(?) slices
	varying int slicesHit[32];

  //! Assume just one volume.

  //! Bounding box of the volume.
  const uniform box3f boundingBox = renderer->model->volumes[0]->boundingBox;

  //! Ray epsilon.
  const uniform float epsilon = 1e-4 * distance(boundingBox.lower, boundingBox.upper);

  //! Compute the intersection interval over the ray and volume bounds.
  VisRenderer_intersectBox(boundingBox, ray);

	// If the exit point is in front of the entry point (say, what?) or the ray hits a clipping before the 
	// volume itself, quit.
  if (ray.t0 > ray.t1)
    return;

  ray.t = ray.t0;

	// Lop off any part of the ray.t -> ray.t1 interval thats
	// clipped away.

	VisRenderer_initializeClip(renderer, ray, slicesHit);

  //! Maximum extent for the volume bounds.
  const float tMax = ray.t1;

  //! Offset ray by a fraction of the nominal ray step.
  const uniform float step = renderer->model->volumes[0]->samplingStep / renderer->model->volumes[0]->samplingRate;
  ray.t += rayOffset * step;

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

  //! Initial trace through the volume and geometries.
  VisRenderer_computeVolumeSample(renderer, renderer->model->volumes[0], ray, volumeColor);
  VisRenderer_computeIsosurfaceSample(renderer, renderer->model->volumes[0], isosurfaceRay, isosurfaceColor);
  VisRenderer_computeGeometrySample(renderer, geometryRay, geometryColor);
  VisRenderer_computeSliceSample(renderer, renderer->model->volumes[0], sliceRay, sliceColor, slicesHit);

  //! Trace the ray through the volume and geometries.
  float firstHit;

	int k = 0;

  while ((firstHit = min(min(min(ray.t, sliceRay.t), isosurfaceRay.t), geometryRay.t)) <= tMax && 
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

      //! Isosurface contribution.
      color = color + (1.0f - color.w) * isosurfaceColor;

      //! Reset isosurface ray.
      isosurfaceRay.t = isosurfaceRay.t + epsilon;
      isosurfaceRay.primID = -1;
      isosurfaceRay.geomID = -1;
      isosurfaceRay.instID = -1;

      //! Trace next isosurface ray.
      VisRenderer_computeIsosurfaceSample(renderer, renderer->model->volumes[0], isosurfaceRay, isosurfaceColor);
    }
    else if (firstHit == geometryRay.t) {

      //! Geometry contribution.
      color = color + (1.0f - color.w) * geometryColor;

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

      //! Geometry contribution.

      color = color + (1.0f - color.w) * sliceColor;

			//! Reset slice ray.
			sliceRay.t = sliceRay.t + epsilon;
			sliceRay.primID = -1;
			sliceRay.geomID -1;
			sliceRay.instID = -1;

			//! Trace next slice ray.
			VisRenderer_computeSliceSample(renderer, renderer->model->volumes[0], sliceRay, sliceColor, slicesHit);
		}
  }
}

void VisRenderer_renderSample(Renderer *uniform pointer, varying ScreenSample &sample, 
												varying int &n_continuation_rays, varying ScreenSample *continuation_rays)
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
  VisRenderer_intersect(renderer, sample.ray, rayOffset, color, n_continuation_rays, continuation_rays);

  //! Gamma correction.
  color = renderer->model->volumes[0]->gammaCorrection.x * pow(color, renderer->model->volumes[0]->gammaCorrection.y);

  //! Attenuate the foreground and background colors by the opacity.
  color = color.w * color + (1.0f - color.w) * background;

  //! Store the result in the sample.
  sample.rgb.x = color.x;  sample.rgb.y = color.y;  sample.rgb.z = color.z;  sample.alpha = color.w;
}

export void VisRenderer_setCamera(void *uniform pointer, 
                                            void *uniform camera) 
{
  //! Cast to the actual Renderer subtype.
  VisRenderer *uniform renderer = (VisRenderer *uniform) pointer;

  //! Set the camera view to render.
  renderer->inherited.camera = (Camera *uniform) camera;
}

export void VisRenderer_setLights(void *uniform pointer, 
                                            void **uniform lights) 
{
  //! Cast to the actual Renderer subtype.
  VisRenderer *uniform renderer = (VisRenderer *uniform) pointer;

  //! Set the light sources.
  renderer->lights = (Light **uniform) lights;

	uniform int i;
	for (i = 0; renderer->lights[i] != NULL; i++);
	renderer->numLights = i;
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

