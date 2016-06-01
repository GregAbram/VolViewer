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

#include "sg/common/Texture2D.h"

namespace ospray {
  namespace sg {

    //! \brief load texture from given file. 
    /*! \detailed if file does not exist, or cannot be loaded for
      some reason, return NULL. Multiple loads from the same file
      will return the *same* texture object */
    Ref<Texture2D> Texture2D::load(const FileName &fileName)
    { 
      static std::map<std::string,Texture2D*> textureCache;
      if (textureCache.find(fileName.str()) != textureCache.end()) 
        return textureCache[fileName.str()];

      Texture2D *tex = NULL;
      const std::string ext = fileName.ext();
      if (ext == "ppm") {
        try {
          int rc, peekchar;

          // open file
          FILE *file = fopen(fileName.str().c_str(),"r");
          const int LINESZ=10000;
          char lineBuf[LINESZ+1]; 

          if (!file) 
            throw std::runtime_error("#osp:miniSG: could not open texture file '"+fileName.str()+"'.");
          
          // read format specifier:
          int format=0;
          fscanf(file,"P%i\n",&format);
          if (format != 6) 
            throw std::runtime_error("#osp:miniSG: can currently load only binary P6 subformats for PPM texture files. "
                                     "Please report this bug at ospray.github.io.");

          // skip all comment lines
          peekchar = getc(file);
          while (peekchar == '#') {
            fgets(lineBuf,LINESZ,file);
            peekchar = getc(file);
          } ungetc(peekchar,file);
        
          // read width and height from first non-comment line
          int width=-1,height=-1;
          rc = fscanf(file,"%i %i\n",&width,&height);
          if (rc != 2) 
            throw std::runtime_error("#osp:miniSG: could not parse width and height in P6 PPM file '"+fileName.str()+"'. "
                                     "Please report this bug at ospray.github.io, and include named file to reproduce the error.");
        
          // skip all comment lines
          peekchar = getc(file);
          while (peekchar == '#') {
            fgets(lineBuf,LINESZ,file);
            peekchar = getc(file);
          } ungetc(peekchar,file);
        
          // read maxval
          int maxVal=-1;
          rc = fscanf(file,"%i",&maxVal);
            peekchar = getc(file);

          if (rc != 1) 
            throw std::runtime_error("#osp:miniSG: could not parse maxval in P6 PPM file '"+fileName.str()+"'. "
                                     "Please report this bug at ospray.github.io, and include named file to reproduce the error.");
          if (maxVal != 255)
            throw std::runtime_error("#osp:miniSG: could not parse P6 PPM file '"+fileName.str()+"': currently supporting only maxVal=255 formats."
                                     "Please report this bug at ospray.github.io, and include named file to reproduce the error.");
        
          tex = new Texture2D;
          tex->size      = vec2i(width,height);
          tex->texelType = OSP_UCHAR3;
          tex->texel     = new unsigned char[width*height*3];
          fread(tex->texel,width*height*3,1,file);
        } catch(std::runtime_error e) {
          std::cerr << e.what() << std::endl;
        }
      } 
      textureCache[fileName.str()] = tex;
      return tex;
    }
    
    void Texture2D::render(RenderContext &ctx)
    {
      if (ospTexture) return;
      
      ospTexture = ospNewTexture2D(size.x,
                                   size.y,
                                   texelType,
                                   texel,
                                   0);
      ospCommit(ospTexture);
      
      if(!ospTexture) std::cerr << "Warning: Could not create Texture2D\n";
    }

  } // ::ospray::sg
} // ::ospray
