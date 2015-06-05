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

// ospray
#include "ospray/camera/PerspectiveCamera.h"
#include "ospray/common/Data.h"
#include "ospray/common/Ray.h"
#include "VisRenderer.h"
#include "ospray/volume/Volume.h"
// ispc exports
#include "VisRenderer_ispc.h"

namespace ospray {

  void VisRenderer::commit() {

    //! Create the equivalent ISPC VisRenderer object.
    if (ispcEquivalent == NULL) ispcEquivalent = ispc::VisRenderer_createInstance();

    //! Get the camera.
    camera = (Camera *) getParamObject("camera", NULL);  
		if (!camera) return;

    //! Get the model.
    model = (Model *) getParamObject("model", NULL);  
		if (!model) return;

    //! Get the dynamic model.
    dynamicModel = (Model *) getParamObject("dynamic_model", NULL);  
		if (!model) return;

    //! Set the camera.
    ispc::VisRenderer_setCamera(ispcEquivalent, camera->getIE());

    //! Set the model.
    ispc::VisRenderer_setModel(ispcEquivalent, model->getIE());

    //! Set the dynamic model.
    ispc::VisRenderer_setDynamicModel(ispcEquivalent, dynamicModel->getIE());

    //! Set the lights if any.
		void *lightData = getParamData("lights", NULL);
		if (lightData)
			ispc::VisRenderer_setLights(ispcEquivalent, getLightsFromData(getParamData("lights", NULL)));
		else
			ispc::VisRenderer_setLights(ispcEquivalent, NULL);

    Data *t = getParamData("slice planes", NULL);
		if (t)
		{
      Data *planes = t;
      Data *svis = getParamData("slice visibility", NULL);
      Data *sclip = getParamData("slice clips", NULL);
      ispc::VisRenderer_setSlices(ispcEquivalent, planes->numItems, 
																(ispc::vec4f *)planes->data, (int *)sclip->data, 
																(int *)svis->data);
    }
    else
      ispc::VisRenderer_setSlices(ispcEquivalent, 0, NULL, NULL, NULL);

		t = getParamData("AO number", NULL);
		if (t)
			ispc::VisRenderer_set_AO_number(ispcEquivalent, *(int *)t->data);
		else
			ispc::VisRenderer_set_AO_number(ispcEquivalent, 0);


		t = getParamData("AO radius", NULL);
		if (t)
			ispc::VisRenderer_set_AO_number(ispcEquivalent, *(float *)t->data);
		else
			ispc::VisRenderer_set_AO_number(ispcEquivalent, 1.0);

    //! Initialize state in the parent class, must be called after the ISPC object is created.
    Renderer::commit();

  }

  void **VisRenderer::getLightsFromData(const Data *buffer) {

    //! Lights are optional.
    size_t lightCount = (buffer != NULL) ? buffer->numItems : 0;

    //! The light array is a NULL terminated list of pointers.
    void **lights = new void *[lightCount + 1];

    //! Copy pointers to the ISPC Light objects.
    for (size_t i=0 ; i < lightCount ; i++) lights[i] = ((Light **) buffer->data)[i]->getIE();

    //! Mark the end of the array.
    lights[lightCount] = NULL;  return(lights);

  }

} // ::ospray

