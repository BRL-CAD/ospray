## Copyright 2021 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

set(COMPONENT_NAME gtest)

set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE})
if (INSTALL_IN_SEPARATE_DIRECTORIES)
  set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE}/${COMPONENT_NAME})
endif()

# Google Test follows the "Live Head" philosophy and therefore recommends
# using the latest commit to 'master' 
ExternalProject_Add(${COMPONENT_NAME}
  URL "https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip"
  URL_HASH "SHA256=1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4"

#   # Skip updating on subsequent builds (faster)
  UPDATE_COMMAND ""

  CMAKE_ARGS
    -Dgtest_force_shared_crt=ON
    -DCMAKE_INSTALL_PREFIX:PATH=${COMPONENT_PATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${DEPENDENCIES_BUILD_TYPE}
    -DBUILD_GMOCK=OFF
)

list(APPEND CMAKE_PREFIX_PATH ${COMPONENT_PATH})
string(REPLACE ";" "|" CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
