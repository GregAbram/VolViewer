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

#include "MPICommon.h"
#include "ospray/api/Device.h"
#include "CommandStream.h"
#include "ospray/common/Managed.h"

/*! \file mpidevice.h Implements the "mpi" device for mpi rendering */

namespace ospray {
  namespace api {

    struct MPIDevice : public Device {
      typedef ospray::mpi::CommandStream CommandStream;

      CommandStream cmd;

      enum {
        CMD_NEW_RENDERER=0,
        CMD_FRAMEBUFFER_CREATE,
        CMD_RENDER_FRAME,
        CMD_FRAMEBUFFER_CLEAR,
        CMD_FRAMEBUFFER_MAP,
        CMD_FRAMEBUFFER_UNMAP,
        CMD_NEW_MODEL,
        CMD_NEW_GEOMETRY,
        CMD_NEW_MATERIAL,
        CMD_NEW_LIGHT,
        CMD_NEW_TRIANGLEMESH,
        CMD_NEW_CAMERA,
        CMD_NEW_VOLUME,
        CMD_NEW_TRANSFERFUNCTION,
        CMD_NEW_DATA,
        CMD_NEW_TEXTURE2D,
        CMD_ADD_GEOMETRY,
        CMD_REMOVE_GEOMETRY,
        CMD_ADD_VOLUME,
        CMD_COMMIT,
        CMD_LOAD_MODULE,
        CMD_RELEASE,
        CMD_GET_TYPE,
        CMD_GET_VALUE,
        CMD_SET_MATERIAL,
        CMD_SET_REGION,
        CMD_SET_OBJECT,
        CMD_SET_STRING,
        CMD_SET_INT,
        CMD_SET_FLOAT,
        CMD_SET_VEC2F,
        CMD_SET_VEC3F,
        CMD_SET_VEC3I,
        CMD_USER
      } CommandTag;

      /*! constructor */
      MPIDevice(// AppMode appMode, OSPMode ospMode,
                int *_ac=NULL, const char **_av=NULL);

      /*! create a new frame buffer */
      virtual OSPFrameBuffer
      frameBufferCreate(const vec2i &size, 
                        const OSPFrameBufferFormat mode,
                        const uint32 channels);

      /*! create a new transfer function object (out of list of 
        registered transfer function types) */
      virtual OSPTransferFunction newTransferFunction(const char *type);

      /*! remove an existing geometry from a model */
      virtual void removeGeometry(OSPModel _model, OSPGeometry _geometry);

      /*! have given renderer create a new Light */
      virtual OSPLight newLight(OSPRenderer _renderer, const char *type);

      /*! map frame buffer */
      virtual const void *frameBufferMap(OSPFrameBuffer fb, 
                                         OSPFrameBufferChannel channel);

      /*! unmap previously mapped frame buffer */
      virtual void frameBufferUnmap(const void *mapped,
                                    OSPFrameBuffer fb);

      /*! clear the specified channel(s) of the frame buffer specified in 'whichChannels'
        
        if whichChannel&OSP_FB_COLOR!=0, clear the color buffer to
        '0,0,0,0'.  

        if whichChannel&OSP_FB_DEPTH!=0, clear the depth buffer to
        +inf.  

        if whichChannel&OSP_FB_ACCUM!=0, clear the accum buffer to 0,0,0,0,
        and reset accumID.
      */
      virtual void frameBufferClear(OSPFrameBuffer _fb,
                                    const uint32 fbChannelFlags); 

      /*! create a new model */
      virtual OSPModel newModel();

      // /*! finalize a newly specified model */
      // virtual void finalizeModel(OSPModel _model);

      /*! commit the given object's outstanding changes */
      virtual void commit(OSPObject object);

      /*! add a new geometry to a model */
      virtual void addGeometry(OSPModel _model, OSPGeometry _geometry);

      /*! add a new volume to a model */
      virtual void addVolume(OSPModel _model, OSPVolume _volume);

      /*! create a new data buffer */
      virtual OSPData newData(size_t nitems, OSPDataType format, void *init, int flags);

      /*! Copy data into the given volume. */
      virtual int setRegion(OSPVolume object, const void *source, 
                            const vec3i &index, const vec3i &count);

      /*! assign (named) string parameter to an object */
      virtual void setString(OSPObject object, const char *bufName, const char *s);

      /*! assign (named) data item as a parameter to an object */
      virtual void setObject(OSPObject target, const char *bufName, OSPObject value);

      /*! assign (named) float parameter to an object */
      virtual void setFloat(OSPObject object, const char *bufName, const float f);

      /*! assign (named) vec2f parameter to an object */
      virtual void setVec2f(OSPObject object, const char *bufName, const vec2f &v);

      /*! assign (named) vec3f parameter to an object */
      virtual void setVec3f(OSPObject object, const char *bufName, const vec3f &v);

      /*! assign (named) int parameter to an object */
      virtual void setInt(OSPObject object, const char *bufName, const int f);

      /*! assign (named) vec3i parameter to an object */
      virtual void setVec3i(OSPObject object, const char *bufName, const vec3i &v);

      /*! add untyped void pointer to object - this will *ONLY* work in local rendering!  */
      virtual void setVoidPtr(OSPObject object, const char *bufName, void *v);

      /*! Get the handle of the named data array associated with an object. */
      virtual int getData(OSPObject object, const char *name, OSPData *value);

      /*! Get a copy of the data in an array (the application is responsible for freeing this pointer). */
      virtual int getDataValues(OSPData object, void **pointer, size_t *count, OSPDataType *type);

      /*! Get the named scalar floating point value associated with an object. */
      virtual int getf(OSPObject object, const char *name, float *value);

      /*! Get the named scalar integer associated with an object. */
      virtual int geti(OSPObject object, const char *name, int *value);

      /*! Get the material associated with a geometry object. */
      virtual int getMaterial(OSPGeometry geometry, OSPMaterial *value);

      /*! Get the named object associated with an object. */
      virtual int getObject(OSPObject object, const char *name, OSPObject *value);

      /*! Retrieve a NULL-terminated list of the parameter names associated with an object. */
      virtual int getParameters(OSPObject object, char ***value);

      /*! Retrieve the total length of the names (with terminators) of the parameters associated with an object. */
      int getParametersSize(OSPObject object, int *value);

      /*! Get a pointer to a copy of the named character string associated with an object. */
      virtual int getString(OSPObject object, const char *name, char **value);

      /*! Get the type of the named parameter or the given object (if 'name' is NULL). */
      virtual int getType(OSPObject object, const char *name, OSPDataType *value);

      /*! Get the named 2-vector floating point value associated with an object. */
      virtual int getVec2f(OSPObject object, const char *name, vec2f *value);

      /*! Get the named 3-vector floating point value associated with an object. */
      virtual int getVec3f(OSPObject object, const char *name, vec3f *value);

      /*! Get the named 3-vector integer value associated with an object. */
      virtual int getVec3i(OSPObject object, const char *name, vec3i *value);

      /*! create a new triangle mesh geometry */
      virtual OSPTriangleMesh newTriangleMesh();

      /*! create a new renderer object (out of list of registered renderers) */
      virtual OSPRenderer newRenderer(const char *type);

      /*! create a new geometry object (out of list of registered geometrys) */
      virtual OSPGeometry newGeometry(const char *type);

      /*! have given renderer create a new material */
      virtual OSPMaterial newMaterial(OSPRenderer _renderer, const char *type);

      /*! create a new camera object (out of list of registered cameras) */
      virtual OSPCamera newCamera(const char *type);

      /*! create a new volume object (out of list of registered volumes) */
      virtual OSPVolume newVolume(const char *type);

      /*! call a renderer to render a frame buffer */
      virtual void renderFrame(OSPFrameBuffer _sc, 
                               OSPRenderer _renderer, 
                               const uint32 fbChannelFlags);

      /*! load module */
      virtual int loadModule(const char *name);

      //! release (i.e., reduce refcount of) given object
      /*! note that all objects in ospray are refcounted, so one cannot
        explicitly "delete" any object. instead, each object is created
        with a refcount of 1, and this refcount will be
        increased/decreased every time another object refers to this
        object resp releases its hold on it; if the refcount is 0 the
        object will automatically get deleted. For example, you can
        create a new material, assign it to a geometry, and immediately
        after this assignation release its refcount; the material will
        stay 'alive' as long as the given geometry requires it. */
      virtual void release(OSPObject _obj);

      //! assign given material to given geometry
      virtual void setMaterial(OSPGeometry _geom, OSPMaterial _mat);

      /*! create a new Texture2D object */
      virtual OSPTexture2D newTexture2D(int width, int height, 
                                        OSPDataType type, void *data, int flags);
    };

  } // ::ospray::api
} // ::ospray


