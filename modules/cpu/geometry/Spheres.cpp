// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// ospray
#include "Spheres.h"
#include "common/Data.h"
#ifndef OSPRAY_TARGET_SYCL
// ispc-generated files
#include "geometry/Spheres_ispc.h"
#else
namespace ispc {
void *Spheres_sampleArea_addr();
void *Spheres_getAreas_addr();
} // namespace ispc
#endif

namespace ospray {

static std::map<OSPSphereType, FeatureFlagsGeometry> sphereFeatureFlags = {
    {OSP_SPHERE, FFG_SPHERE},
    {OSP_DISC, FFG_DISC_POINT},
    {OSP_ORIENTED_DISC, FFG_ORIENTED_DISC_POINT}};

Spheres::Spheres(api::ISPCDevice &device)
    : AddStructShared(device.getIspcrtContext(), device, FFG_NONE)
{
#ifndef OSPRAY_TARGET_SYCL
  getSh()->super.postIntersect =
      reinterpret_cast<ispc::Geometry_postIntersectFct>(
          ispc::Spheres_postIntersect_addr());
#endif
  getSh()->super.getAreas = reinterpret_cast<ispc::Geometry_GetAreasFct>(
      ispc::Spheres_getAreas_addr());
  // We also set the sampleArea function ptr so that
  // Geometry::supportAreaLighting will be true, but in SYCL we'll never call it
  // through the function pointer on the device
  getSh()->super.sampleArea = reinterpret_cast<ispc::Geometry_SampleAreaFct>(
      ispc::Spheres_sampleArea_addr());
}

std::string Spheres::toString() const
{
  return "ospray::Spheres";
}

void Spheres::commit()
{
  sphereType = (OSPSphereType)getParam<uint32_t>("type", OSP_SPHERE);
  radius = getParam<float>("radius", 0.01f);
  vertexData = getParamDataT<vec3f>("sphere.position", true);
  radiusData = getParamDataT<float>("sphere.radius");
  texcoordData = getParamDataT<vec2f>("sphere.texcoord");
  normalData = nullptr;
  if (sphereType == OSP_ORIENTED_DISC) {
    normalData = getParamDataT<vec3f>("sphere.normal", true);
  }

  // If the application's data is already interleaved and they just passed us
  // separate views into the interleaved data to make the sphere.position and
  // sphere.radius arrays we can detect this and use the interleaved array
  // directly to avoid copying it. If not, we need to make a new interleaved
  // data array for Embree
  if (radiusData && vertexData
      && reinterpret_cast<uint8_t *>(vertexData->data()) + sizeof(vec3f)
          == reinterpret_cast<uint8_t *>(radiusData->data())
      && vertexData->stride() == radiusData->stride()) {
    auto interleaved = new Data(getISPCDevice(),
        vertexData->data(),
        OSP_VEC4F,
        vertexData->numItems,
        vec3l(sizeof(vec4f), 0, 0));
    sphereData = &interleaved->as<vec4f, 1>();
    interleaved->refDec();
  } else {
    // To maintain OSPRay 2.x compatibility we need to create the interleaved
    // position/radius array that Embree expects from the separate
    // position/radius (or global radius) that the OSPRay geometry takes
    auto interleaved =
        new Data(getISPCDevice(), OSP_VEC4F, vertexData->numItems);
    sphereData = &interleaved->as<vec4f, 1>();
    interleaved->refDec();
    // For now default to always create the interleaved buffer since we
    // don't expose the interleaved data yet
    for (size_t i = 0; i < vertexData->size(); ++i) {
      float ptRadius = radius;
      if (radiusData) {
        ptRadius = (*radiusData)[i];
      }
      interleaved->as<vec4f, 1>()[i] = vec4f((*vertexData)[i], ptRadius);
    }
  }

  createEmbreeGeometry((RTCGeometryType)sphereType);
  featureFlagsGeometry = sphereFeatureFlags[sphereType];
  setEmbreeGeometryBuffer(embreeGeometry, RTC_BUFFER_TYPE_VERTEX, sphereData);
  setEmbreeGeometryBuffer(embreeGeometry, RTC_BUFFER_TYPE_NORMAL, normalData);
  rtcCommitGeometry(embreeGeometry);

  getSh()->sphere = *ispc(sphereData);
  getSh()->texcoord = *ispc(texcoordData);
  getSh()->super.numPrimitives = numPrimitives();
  getSh()->normalData = *ispc(normalData);
  getSh()->sphereType = sphereType;

  postCreationInfo(numPrimitives());
}

size_t Spheres::numPrimitives() const
{
  return sphereData ? sphereData->size() : 0;
}

} // namespace ospray
