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

#pragma once

#include "MPICommon.h"
#include "ospray/api/Handle.h"

namespace ospray {
  namespace mpi {
    using api::Handle;

    /*! \brief abstraction for a binary command stream */
    /*! abstracts the concept of the mpi device writing commands and
      parameters, and the respective worker reading and unpacking
      those parameters, into a wrapper class. allows for implementing
      the actual communication via different methods (MPI, sockets,
      COI) as well as tweaking the implementation in a way that will
      apply equally to all functions */
    struct CommandStream {
      void newCommand(int tag) {
        int rc = MPI_Bcast(&tag,1,MPI_INT,MPI_ROOT,mpi::worker.comm); 
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(const void *data, const size_t size)
      {
        Assert(data);
        int rc = MPI_Bcast((void*)data,size,MPI_BYTE,MPI_ROOT,mpi::worker.comm);
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(const void *data, const size_t size, int32 rank, const MPI_Comm &comm)
      {
        int rc = MPI_Send((void*)data,size,MPI_BYTE,rank,0,comm);
        Assert(rc == MPI_SUCCESS);
      }
      inline void send(int32 i)
      { 
        int rc = MPI_Bcast(&i,1,MPI_INT,MPI_ROOT,mpi::worker.comm);
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(size_t i)
      { 
        int rc = MPI_Bcast(&i,1,MPI_LONG,MPI_ROOT,mpi::worker.comm);
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(const vec2f &v)
      {
        int rc = MPI_Bcast((void*)&v,2,MPI_FLOAT,MPI_ROOT,mpi::worker.comm);
        Assert(rc == MPI_SUCCESS);
      }
      inline void send(const vec2i &v)
      { 
        int rc = MPI_Bcast((void*)&v,2,MPI_INT,MPI_ROOT,mpi::worker.comm);
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(const vec3f &v)
      { 
        int rc = MPI_Bcast((void*)&v,3,MPI_FLOAT,MPI_ROOT,mpi::worker.comm);
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(const vec3i &v)
      { 
        int rc = MPI_Bcast((void*)&v,3,MPI_INT,MPI_ROOT,mpi::worker.comm);
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(uint32 i)
      { 
        int rc = MPI_Bcast(&i,1,MPI_INT,MPI_ROOT,mpi::worker.comm);       
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(const Handle &h)
      { 
        int rc = MPI_Bcast((void*)&h,2,MPI_INT,MPI_ROOT,mpi::worker.comm);
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(float f)
      { 
        int rc = MPI_Bcast(&f,1,MPI_FLOAT,MPI_ROOT,mpi::worker.comm); 
        Assert(rc == MPI_SUCCESS); 
      }
      inline void send(const char *s)
      { 
        int len = strlen(s);
        send(len);
        int rc = MPI_Bcast((void*)s,len,MPI_CHAR,MPI_ROOT,mpi::worker.comm); 
        Assert(rc == MPI_SUCCESS); 
      }

      inline int get_int32() 
      { 
        int v; 
        int rc = MPI_Bcast(&v,1,MPI_INT,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline size_t get_size_t() 
      { 
        int64 v; 
        int rc = MPI_Bcast(&v,1,MPI_LONG,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline void get_data(size_t size, void *pointer) 
      { 
        // int64 size; 
        // int rc = MPI_Bcast(&size,1,MPI_LONG,0,mpi::app.comm); 
        // Assert(rc == MPI_SUCCESS); 
        int rc = MPI_Bcast(pointer,size,MPI_BYTE,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
      }
      inline void get_data(size_t size, void *pointer, const int32 &rank, const MPI_Comm &comm)
      {
        MPI_Status status;
        int rc = MPI_Recv(pointer,size,MPI_BYTE,rank,0,comm,&status);
        Assert(rc == MPI_SUCCESS);
      }
      inline Handle get_handle() 
      { 
        Handle v; 
        int rc = MPI_Bcast(&v,2,MPI_INT,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline vec2i get_vec2i() 
      { 
        vec2i v; 
        int rc = MPI_Bcast(&v,2,MPI_INT,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline vec2f get_vec2f() 
      { 
        vec2f v; 
        int rc = MPI_Bcast(&v,2,MPI_FLOAT,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline vec3f get_vec3f() 
      { 
        vec3f v; 
        int rc = MPI_Bcast(&v,3,MPI_FLOAT,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline vec3i get_vec3i() 
      { 
        vec3i v; 
        int rc = MPI_Bcast(&v,3,MPI_INT,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline float get_float() 
      { 
        float v; 
        int rc = MPI_Bcast(&v,1,MPI_FLOAT,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline int get_int() 
      { 
        int v; 
        int rc = MPI_Bcast(&v,1,MPI_INT,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return v; 
      }
      inline void free(const char *s) 
      { Assert(s); ::free((void*)s); }
      inline const char *get_charPtr() 
      { 
        int len = get_int32();
        char buf[len+1]; buf[len] = 0;
        int rc = MPI_Bcast(buf,len,MPI_CHAR,0,mpi::app.comm); 
        Assert(rc == MPI_SUCCESS); 
        return strdup(buf); 
      }
      void flush() {};
    };

  } // ::ospray::mpi
} // ::ospray
