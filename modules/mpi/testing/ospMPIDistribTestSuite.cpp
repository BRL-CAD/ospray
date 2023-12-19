// Copyright 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <mpi.h>
#include "distributed_test_fixture.h"
#include "rkcommon/utility/getEnvVar.h"

OSPRayEnvironment *ospEnv;

namespace MPIDistribOSPRayTestScenes {
TEST_P(MPIFromOsprayTesting, test_scenes)
{
  PerformRenderTest();
}

#ifdef OSPRAY_ENABLE_VOLUMES
INSTANTIATE_TEST_SUITE_P(MPIDistribTestScenesVolumes,
    MPIFromOsprayTesting,
    ::testing::Combine(
        ::testing::Values(
            "gravity_spheres_isosurface", "vdb_volume", "particle_volume"),
        ::testing::Values(1)));
#endif

INSTANTIATE_TEST_SUITE_P(MPIDistribTestScenesGeometry,
    MPIFromOsprayTesting,
    ::testing::Combine(::testing::Values("planes", "boxes", "random_spheres"),
        ::testing::Values(1)));
} // namespace MPIDistribOSPRayTestScenes

int main(int argc, char **argv)
{
  int mpiThreadCapability = 0;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpiThreadCapability);
  if (mpiThreadCapability != MPI_THREAD_MULTIPLE
      && mpiThreadCapability != MPI_THREAD_SERIALIZED) {
    fprintf(stderr,
        "OSPRay requires the MPI runtime to support thread "
        "multiple or thread serialized.\n");
    return 1;
  }

  int result = 0;
  // load the MPI module, and select the MPI distributed device. Here we
  // do not call ospInit, as we want to explicitly pick the distributed
  // device
  auto OSPRAY_MPI_DISTRIBUTED_GPU =
      rkcommon::utility::getEnvVar<int>("OSPRAY_MPI_DISTRIBUTED_GPU").value_or(0);
  if (OSPRAY_MPI_DISTRIBUTED_GPU) {
    ospLoadModule("mpi_distributed_gpu");
  } else {
    ospLoadModule("mpi_distributed_cpu");
  }
  {
    cpp::Device device("mpiDistributed");

    device.setErrorCallback(
        [](void *, OSPError error, const char *errorDetails) {
          std::cerr << "OSPRay error: " << errorDetails << std::endl;
          std::exit(EXIT_FAILURE);
        });

    device.setStatusCallback([](void *, const char *msg) { std::cout << msg; });

    // First commit with INFO log Level to output device info string
    device.setParam("warnAsError", true);
    device.setParam("logLevel", OSP_LOG_INFO);
    device.commit();

    // Then use WARNING log level for tests execution
    device.setParam("logLevel", OSP_LOG_WARNING);
    device.commit();
    device.setCurrent();

    ::testing::InitGoogleTest(&argc, argv);
    ospEnv = new OSPRayEnvironment(argc, argv);
    AddGlobalTestEnvironment(ospEnv);
    result = RUN_ALL_TESTS();
  }
  ospShutdown();
  MPI_Finalize();
  return result;
}
