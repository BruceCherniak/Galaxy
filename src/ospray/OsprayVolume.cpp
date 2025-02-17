// ========================================================================== //
// Copyright (c) 2014-2019 The University of Texas at Austin.                 //
// All rights reserved.                                                       //
//                                                                            //
// Licensed under the Apache License, Version 2.0 (the "License");            //
// you may not use this file except in compliance with the License.           //
// A copy of the License is included with this software in the file LICENSE.  //
// If your copy does not contain the License, you may obtain a copy of the    //
// License at:                                                                //
//                                                                            //
//     https://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                            //
// Unless required by applicable law or agreed to in writing, software        //
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT  //
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.           //
// See the License for the specific language governing permissions and        //
// limitations under the License.                                             //
//                                                                            //
// ========================================================================== //

#include "OsprayVolume.h"

using namespace gxy;

OsprayVolume::OsprayVolume(VolumeP v)
{
  volume = v;

  OSPVolume ospv = ospNewVolume("shared_structured_volume");
  
  osp::vec3i counts;
  volume->get_ghosted_local_counts(counts.x, counts.y, counts.z);

  osp::vec3f origin, spacing;
  volume->get_ghosted_local_origin(origin.x, origin.y, origin.z);
  volume->get_deltas(spacing.x, spacing.y, spacing.z);
  
  OSPData data = ospNewData(counts.x*counts.y*counts.z, 
    volume->isFloat() ? OSP_FLOAT : OSP_UCHAR, (void *)volume->get_samples(), OSP_DATA_SHARED_BUFFER);
  ospCommit(data);
  
  ospSetObject(ospv, "voxelData", data);
  ospSetVec3i(ospv, "dimensions", counts);
  ospSetVec3f(ospv, "gridOrigin", origin);
  ospSetVec3f(ospv, "gridSpacing", spacing);
  ospSetString(ospv, "voxelType", volume->isFloat() ? "float" : "uchar");
  ospSetf(ospv, "samplingRate", 1.0);
  
  ospSetObject(ospv, "transferFunction", ospNewTransferFunction("piecewise_linear"));

  ospCommit(ospv);
  
  theOSPRayObject = ospv;
}
