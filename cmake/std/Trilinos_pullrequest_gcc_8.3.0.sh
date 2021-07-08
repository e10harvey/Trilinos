#!/bin/bash
cd GenConfig
git checkout issue27
python3 -m gen_config --force rhel7_Trilinos-pullrequest-gcc-8.3.0_release-debug_no-asan_no-complex_no-fpic_framework --cmake-fragment=rhel7_Trilinos-pullrequest-gcc-8.3.0_release-debug_no-asan_no-complex_no-fpic.cmake
cd ../Trilinos/build
rm -rf *
cd ../../GenConfig/deps/LoadEnv/
git checkout gen_config_issue27
cd -
bash -c "source ../../GenConfig/deps/LoadEnv/load-env.sh --ci-mode Trilinos-pullrequest-gcc-8.3.0; cmake -DTrilinos_CONFIGURE_OPTIONS_FILE:STRING=$PWD/../../GenConfig/rhel7_Trilinos-pullrequest-gcc-8.3.0_release-debug_no-asan_no-complex_no-fpic.cmake .."
