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

#pragma once

/*! \file OsprayTriangles.h 
 * \brief translation class for Galaxy Triangles to OSPRay Geometry
 * \ingroup render
 */

#include "Application.h"
#include "Triangles.h"
#include "OsprayObject.h"

namespace gxy
{

OBJECT_POINTER_TYPES(OsprayTriangles)

//! translation class for Galaxy Triangles to OSPRay Geometry
/*! \ingroup render 
 * \sa OsprayObject
 */
class OsprayTriangles : public OsprayObject
{
  GALAXY_OBJECT(OsprayTriangles)

public:
  static OsprayTrianglesP NewP(TrianglesP p) { return OsprayTriangles::Cast(std::shared_ptr<OsprayTriangles>(new OsprayTriangles(p))); }

private:
  OsprayTriangles(TrianglesP);

private:
  TrianglesP triangles;
};

}

