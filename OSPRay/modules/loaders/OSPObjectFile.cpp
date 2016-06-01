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

#include <libgen.h>
#include <string>
#include <string.h>
#include "modules/loaders/OSPObjectFile.h"
#include "modules/loaders/VolumeFile.h"

void OSPObjectFile::importAttributeFloat(const tinyxml2::XMLNode *node, OSPObject parent) {

  //! The attribute value is encoded in a string.
  const char *text = node->ToElement()->GetText();  float value = 0.0f;  char guard[8];

  //! Get the attribute value.
  exitOnCondition(sscanf(text, "%f %7s", &value, guard) != 1, "malformed XML element '" + std::string(node->ToElement()->Name()) + "'");

  //! Set the attribute on the parent object.
  ospSet1f(parent, node->ToElement()->Name(), value);

}

void OSPObjectFile::importAttributeFloat2(const tinyxml2::XMLNode *node, OSPObject parent) {

  //! The attribute value is encoded in a string.
  const char *text = node->ToElement()->GetText();  osp::vec2f value(0.0f);  char guard[8];

  //! Get the attribute value.
  exitOnCondition(sscanf(text, "%f %f %7s", &value.x, &value.y, guard) != 2, "malformed XML element '" + std::string(node->ToElement()->Name()) + "'");

  //! Set the attribute on the parent object.
  ospSetVec2f(parent, node->ToElement()->Name(), value);

}

void OSPObjectFile::importAttributeFloat3(const tinyxml2::XMLNode *node, OSPObject parent) {

  //! The attribute value is encoded in a string.
  const char *text = node->ToElement()->GetText();  osp::vec3f value(0.0f);  char guard[8];

  //! Get the attribute value.
  exitOnCondition(sscanf(text, "%f %f %f %7s", &value.x, &value.y, &value.z, guard) != 3, "malformed XML element '" + std::string(node->ToElement()->Name()) + "'");

  //! Set the attribute on the parent object.
  ospSetVec3f(parent, node->ToElement()->Name(), value);

}

void OSPObjectFile::importAttributeInteger(const tinyxml2::XMLNode *node, OSPObject parent) {

  //! The attribute value is encoded in a string.
  const char *text = node->ToElement()->GetText();  int value = 0;  char guard[8];

  //! Get the attribute value.
  exitOnCondition(sscanf(text, "%d %7s", &value, guard) != 1, "malformed XML element '" + std::string(node->ToElement()->Name()) + "'");

  //! Set the attribute on the parent object.
  ospSet1i(parent, node->ToElement()->Name(), value);

}

void OSPObjectFile::importAttributeInteger3(const tinyxml2::XMLNode *node, OSPObject parent) {

  //! The attribute value is encoded in a string.
  const char *text = node->ToElement()->GetText();  osp::vec3i value(0);  char guard[8];

  //! Get the attribute value.
  exitOnCondition(sscanf(text, "%d %d %d %7s", &value.x, &value.y, &value.z, guard) != 3, "malformed XML element '" + std::string(node->ToElement()->Name()) + "'");

  //! Set the attribute on the parent object.
  ospSetVec3i(parent, node->ToElement()->Name(), value);

}

void OSPObjectFile::importAttributeString(const tinyxml2::XMLNode *node, OSPObject parent) {

  //! Get the attribute value.
  const char *value = node->ToElement()->GetText();

  //! Error check.
  exitOnCondition(strlen(value) == 0, "malformed XML element '" + std::string(node->ToElement()->Name()) + "'");

  //! Set the attribute on the parent object.
  ospSetString(parent, node->ToElement()->Name(), value);

}

OSPLight OSPObjectFile::importLight(const tinyxml2::XMLNode *root) {

  //! Create the OSPRay object.
  OSPLight light = ospNewLight(NULL, root->ToElement()->Attribute("type"));

  //! Iterate over object attributes.
  for (const tinyxml2::XMLNode *node = root->FirstChild() ; node ; node = node->NextSibling()) {

    //! Light color.
    if (!strcmp(node->ToElement()->Name(), "color")) { importAttributeFloat3(node, light);  continue; }

    //! Light direction.
    if (!strcmp(node->ToElement()->Name(), "direction")) { importAttributeFloat3(node, light);  continue; }

    //! Light half angle for spot lights.
    if (!strcmp(node->ToElement()->Name(), "halfAngle")) { importAttributeFloat(node, light);  continue; }

    //! Light position.
    if (!strcmp(node->ToElement()->Name(), "position")) { importAttributeFloat3(node, light);  continue; }

    //! Light illumination distance cutoff.
    if (!strcmp(node->ToElement()->Name(), "range")) { importAttributeFloat(node, light);  continue; }

    //! Error check.
    exitOnCondition(true, "unrecognized XML element type '" + std::string(node->ToElement()->Name()) + "'");

  }

  //! The populated light object.
  return(light);

}

OSPObject OSPObjectFile::importObject(const tinyxml2::XMLNode *node) {

  //! OSPRay light object.
  if (!strcmp(node->ToElement()->Name(), "light")) return((OSPObject) importLight(node));

  //! OSPRay triangle mesh object.
  if (!strcmp(node->ToElement()->Name(), "triangleMesh")) return((OSPObject) importTriangleMesh(node));

  //! OSPRay volume object.
  if (!strcmp(node->ToElement()->Name(), "volume")) return((OSPObject) importVolume(node));

  //! No other object types are currently supported.
  exitOnCondition(true, "unrecognized XML element type '" + std::string(node->ToElement()->Name()) + "'");  return(NULL);

}

OSPObject *OSPObjectFile::importObjects() {

  //! The XML document container.
  tinyxml2::XMLDocument xml(true, tinyxml2::COLLAPSE_WHITESPACE);

  //! Read the XML object file.
  exitOnCondition(xml.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS, "unable to read object file '" + filename + "'");

  //! A list of OSPRay objects and their attributes contained in the file.
  std::vector<OSPObject> objects;

  //! Iterate over the object entries, skip the XML declaration and comments.
  for (const tinyxml2::XMLNode *node = xml.FirstChild() ; node ; node = node->NextSibling()) if (node->ToElement()) objects.push_back(importObject(node));

  //! Copy the objects into a list.
  OSPObject *pointer = new OSPObject[objects.size() + 1];  memcpy(pointer, &objects[0], objects.size() * sizeof(OSPObject));

  //! Mark the end of the list.
  pointer[objects.size()] = NULL;  return(pointer);

}

OSPVolume OSPObjectFile::importVolume(const tinyxml2::XMLNode *root) {

  //! Create the OSPRay object.
  OSPVolume volume = ospNewVolume("block_bricked_volume");

  //! Temporary storage for the file name attribute if specified.
  const char *volumeFilename = NULL;

  //! Iterate over object attributes.
  for (const tinyxml2::XMLNode *node = root->FirstChild() ; node ; node = node->NextSibling()) {

    //! Volume size in voxels per dimension.
    if (!strcmp(node->ToElement()->Name(), "dimensions")) { importAttributeInteger3(node, volume);  continue; }

    //! File containing a volume specification and / or voxel data.
    if (!strcmp(node->ToElement()->Name(), "filename")) { volumeFilename = node->ToElement()->GetText();  continue; }

    //! Gamma correction coefficient and exponent.
    if (!strcmp(node->ToElement()->Name(), "gammaCorrection")) { importAttributeFloat2(node, volume);  continue; }

    //! Grid origin in world coordinates.
    if (!strcmp(node->ToElement()->Name(), "gridOrigin")) { importAttributeFloat3(node, volume);  continue; }

    //! Grid spacing in each dimension in world coordinates.
    if (!strcmp(node->ToElement()->Name(), "gridSpacing")) { importAttributeFloat3(node, volume);  continue; }

    //! Sampling rate for ray casting based renderers.
    if (!strcmp(node->ToElement()->Name(), "samplingRate")) { importAttributeFloat(node, volume);  continue; }

    //! Subvolume offset from origin within the full volume.
    if (!strcmp(node->ToElement()->Name(), "subvolumeOffsets")) { importAttributeInteger3(node, volume);  continue; }

    //! Subvolume dimensions within the full volume.
    if (!strcmp(node->ToElement()->Name(), "subvolumeDimensions")) { importAttributeInteger3(node, volume);  continue; }

    //! Subvolume steps in each dimension; can be used to subsample the volume.
    if (!strcmp(node->ToElement()->Name(), "subvolumeSteps")) { importAttributeInteger3(node, volume);  continue; }

    //! Voxel value range.
    if (!strcmp(node->ToElement()->Name(), "voxelRange")) { importAttributeFloat2(node, volume);  continue; }

    //! Voxel type string.
    if (!strcmp(node->ToElement()->Name(), "voxelType")) { importAttributeString(node, volume);  continue; }

    //! Error check.
    exitOnCondition(true, "unrecognized XML element type '" + std::string(node->ToElement()->Name()) + "'");

  }

  //! Load the contents of the volume file if specified.
  if (volumeFilename != NULL) {

    //! Some implementations of 'dirname()' are destructive.
    char *duplicateFilename = strdup(filename.c_str());

    //! The volume file path is absolute.
    if (volumeFilename[0] == '/') return(VolumeFile::importVolume(volumeFilename, volume));

    //! The volume file path is relative to the object file path.
    if (volumeFilename[0] != '/') return(VolumeFile::importVolume((std::string(dirname(duplicateFilename)) + "/" + volumeFilename).c_str(), volume));

    //! Free the temporary character array.
    if (duplicateFilename != NULL) free(duplicateFilename);

  }

  //! The populated volume object.
  return(volume);

}

