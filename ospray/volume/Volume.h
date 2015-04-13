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

#include "ospray/common/Managed.h"

//! \brief Define a function to create an instance of the InternalClass
//!  associated with ExternalName.
//!
//! \internal The function generated by this macro is used to create an
//!  instance of a concrete subtype of an abstract base class.  This
//!  macro is needed since the subclass type may not be known to OSPRay
//!  at build time.  Rather, the subclass can be defined in an external
//!  module and registered with OSPRay using this macro.
//!
#define OSP_REGISTER_VOLUME(InternalClass, ExternalName)        \
  extern "C" Volume *ospray_create_volume_##ExternalName()      \
  { return(new InternalClass()); }

namespace ospray {

  //! \brief A Volume is an abstraction for the concrete object which
  //!  performs the volume sampling.
  //!
  //!  The actual memory layout, dimensionality, and source of samples
  //!  are unknown to this class.  Subclasses may implement structured
  //!  volumes, unstructured volumes, radial basis functions, etc.  A
  //!  type string specifies a particular concrete implementation to
  //!  createInstance().  This type string must be registered either in
  //!  OSPRay proper, or in a loaded module using OSP_REGISTER_VOLUME.
  //!
  class Volume : public ManagedObject {
  public:

    //! Constructor.
    Volume() {};

    //! Destructor.
    virtual ~Volume() {};

    //! Allocate storage and populate the volume.
    virtual void commit() = 0;

    //! Create a volume container of the given type.
    static Volume *createInstance(std::string type);


    //! Copy voxels into the volume at the given index (non-zero return value indicates success).
    virtual int setRegion(const void *source, const vec3i &index, const vec3i &count) = 0;

    //! A string description of this class.
    virtual std::string toString() const { return("ospray::Volume"); }

  protected:


    //! Print an error message.
    inline void emitMessage(const std::string &kind, const std::string &message) const
    { std::cerr << "  " + toString() + "  " + kind + ": " + message + "." << std::endl; }

    //! Error checking.
    inline void exitOnCondition(bool condition, const std::string &message) const
    { if (!condition) return;  emitMessage("ERROR", message);  exit(1); }

    //! Warning condition.
    inline void warnOnCondition(bool condition, const std::string &message) const
    { if (!condition) return;  emitMessage("WARNING", message); }

  };

} // ::ospray

