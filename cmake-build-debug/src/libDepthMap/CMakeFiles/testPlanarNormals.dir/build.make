# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.16.5/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.16.5/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/dave/Animesh

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/dave/Animesh/cmake-build-debug

# Include any dependencies generated for this target.
include src/libDepthMap/CMakeFiles/testPlanarNormals.dir/depend.make

# Include the progress variables for this target.
include src/libDepthMap/CMakeFiles/testPlanarNormals.dir/progress.make

# Include the compile flags for this target's objects.
include src/libDepthMap/CMakeFiles/testPlanarNormals.dir/flags.make

src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/main.cpp.o: src/libDepthMap/CMakeFiles/testPlanarNormals.dir/flags.make
src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/main.cpp.o: ../src/libDepthMap/tests/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/main.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libDepthMap && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testPlanarNormals.dir/tests/main.cpp.o -c /Users/dave/Animesh/src/libDepthMap/tests/main.cpp

src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testPlanarNormals.dir/tests/main.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libDepthMap && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libDepthMap/tests/main.cpp > CMakeFiles/testPlanarNormals.dir/tests/main.cpp.i

src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testPlanarNormals.dir/tests/main.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libDepthMap && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libDepthMap/tests/main.cpp -o CMakeFiles/testPlanarNormals.dir/tests/main.cpp.s

src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.o: src/libDepthMap/CMakeFiles/testPlanarNormals.dir/flags.make
src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.o: ../src/libDepthMap/tests/TestPlanarNormals.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libDepthMap && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.o -c /Users/dave/Animesh/src/libDepthMap/tests/TestPlanarNormals.cpp

src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libDepthMap && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libDepthMap/tests/TestPlanarNormals.cpp > CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.i

src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libDepthMap && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libDepthMap/tests/TestPlanarNormals.cpp -o CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.s

# Object files for target testPlanarNormals
testPlanarNormals_OBJECTS = \
"CMakeFiles/testPlanarNormals.dir/tests/main.cpp.o" \
"CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.o"

# External object files for target testPlanarNormals
testPlanarNormals_EXTERNAL_OBJECTS =

../bin/testPlanarNormals: src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/main.cpp.o
../bin/testPlanarNormals: src/libDepthMap/CMakeFiles/testPlanarNormals.dir/tests/TestPlanarNormals.cpp.o
../bin/testPlanarNormals: src/libDepthMap/CMakeFiles/testPlanarNormals.dir/build.make
../bin/testPlanarNormals: ../lib/libDepthMap.a
../bin/testPlanarNormals: /usr/local/lib/libpcl_apps.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_outofcore.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_people.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_simulation.dylib
../bin/testPlanarNormals: /usr/local/lib/libboost_system-mt.dylib
../bin/testPlanarNormals: /usr/local/lib/libboost_filesystem-mt.dylib
../bin/testPlanarNormals: /usr/local/lib/libboost_thread-mt.dylib
../bin/testPlanarNormals: /usr/local/lib/libboost_date_time-mt.dylib
../bin/testPlanarNormals: /usr/local/lib/libboost_iostreams-mt.dylib
../bin/testPlanarNormals: /usr/local/lib/libboost_chrono-mt.dylib
../bin/testPlanarNormals: /usr/local/lib/libboost_atomic-mt.dylib
../bin/testPlanarNormals: /usr/local/lib/libboost_regex-mt.dylib
../bin/testPlanarNormals: /usr/local/lib/libqhull_p.dylib
../bin/testPlanarNormals: /usr/lib/libz.dylib
../bin/testPlanarNormals: /usr/lib/libexpat.dylib
../bin/testPlanarNormals: /usr/local/opt/python/Frameworks/Python.framework/Versions/3.7/lib/libpython3.7.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkWrappingTools-8.2.a
../bin/testPlanarNormals: /usr/local/lib/libjpeg.dylib
../bin/testPlanarNormals: /usr/local/lib/libpng.dylib
../bin/testPlanarNormals: /usr/local/lib/libtiff.dylib
../bin/testPlanarNormals: /usr/local/lib/libhdf5.dylib
../bin/testPlanarNormals: /usr/local/lib/libsz.dylib
../bin/testPlanarNormals: /usr/lib/libdl.dylib
../bin/testPlanarNormals: /usr/lib/libm.dylib
../bin/testPlanarNormals: /usr/local/lib/libhdf5_hl.dylib
../bin/testPlanarNormals: /usr/local/lib/libnetcdf.dylib
../bin/testPlanarNormals: /usr/lib/libxml2.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_keypoints.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_tracking.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_recognition.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_registration.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_stereo.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_segmentation.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_ml.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_features.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_filters.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_sample_consensus.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_visualization.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_io.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_surface.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_search.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_kdtree.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_octree.dylib
../bin/testPlanarNormals: /usr/local/lib/libpcl_common.dylib
../bin/testPlanarNormals: /usr/lib/libexpat.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkDomainsChemistryOpenGL2-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkDomainsChemistry-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersFlowPaths-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersGeneric-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersHyperTree-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersParallelImaging-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersPoints-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersProgrammable-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkPythonInterpreter-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/opt/python/Frameworks/Python.framework/Versions/3.7/lib/libpython3.7.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkWrappingTools-8.2.a
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersPython-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersSMP-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersSelection-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersTopology-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersVerdict-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkverdict-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkGUISupportQtSQL-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOSQL-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtksqlite-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/opt/qt/lib/QtSql.framework/QtSql
../bin/testPlanarNormals: /usr/local/lib/libjpeg.dylib
../bin/testPlanarNormals: /usr/local/lib/libtiff.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkGeovisCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkproj-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/lib/libhdf5.dylib
../bin/testPlanarNormals: /usr/local/lib/libsz.dylib
../bin/testPlanarNormals: /usr/lib/libdl.dylib
../bin/testPlanarNormals: /usr/lib/libm.dylib
../bin/testPlanarNormals: /usr/local/lib/libhdf5_hl.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOAMR-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersAMR-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOAsynchronous-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOCityGML-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkpugixml-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOEnSight-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOExodus-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOExportOpenGL2-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOExportPDF-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOExport-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingGL2PSOpenGL2-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkgl2ps-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/lib/libpng.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtklibharu-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOImport-8.2.1.dylib
../bin/testPlanarNormals: /usr/lib/libxml2.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOInfovis-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOLSDyna-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOMINC-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOMovie-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtktheora-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkogg-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOPLY-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOParallel-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersParallel-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkexodusII-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/lib/libnetcdf.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOGeometry-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIONetCDF-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkjsoncpp-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOParallelXML-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkParallelCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOLegacy-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOSegY-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOTecplotTable-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOVeraOut-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOVideo-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingMorphological-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingStatistics-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingStencil-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkInfovisBoostGraphAlgorithms-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkInteractionImage-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkPythonContext2D-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkWrappingPython37Core-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingContextOpenGL2-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingFreeTypeFontConfig-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingImage-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingLOD-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingQt-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersTexture-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingVolumeOpenGL2-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingMath-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkViewsContext2D-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkViewsQt-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkGUISupportQt-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingOpenGL2-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkglew-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkViewsInfovis-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkChartsCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingContext2D-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersImaging-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkInfovisLayout-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkInfovisCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkViewsCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkInteractionWidgets-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersHybrid-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingGeneral-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingSources-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersModeling-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkInteractionStyle-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersExtraction-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersStatistics-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingFourier-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingHybrid-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOImage-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkDICOMParser-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkmetaio-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingAnnotation-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingColor-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingVolume-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkImagingCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOXML-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOXMLParser-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkIOCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkdoubleconversion-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtklz4-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtklzma-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingLabel-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingFreeType-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkRenderingCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonColor-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersGeometry-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersSources-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersGeneral-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonComputationalGeometry-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkFiltersCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonExecutionModel-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonDataModel-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonMisc-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonSystem-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonTransforms-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonMath-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkCommonCore-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtksys-8.2.1.dylib
../bin/testPlanarNormals: /usr/local/Cellar/vtk/8.2.0_8/lib/libvtkfreetype-8.2.1.dylib
../bin/testPlanarNormals: /usr/lib/libz.dylib
../bin/testPlanarNormals: /usr/local/opt/qt/lib/QtWidgets.framework/QtWidgets
../bin/testPlanarNormals: /usr/local/opt/qt/lib/QtGui.framework/QtGui
../bin/testPlanarNormals: /usr/local/opt/qt/lib/QtCore.framework/QtCore
../bin/testPlanarNormals: ../lib/libCamera.a
../bin/testPlanarNormals: ../lib/libFileUtils.a
../bin/testPlanarNormals: ../lib/libGeom.a
../bin/testPlanarNormals: src/libDepthMap/CMakeFiles/testPlanarNormals.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../../bin/testPlanarNormals"
	cd /Users/dave/Animesh/cmake-build-debug/src/libDepthMap && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testPlanarNormals.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/libDepthMap/CMakeFiles/testPlanarNormals.dir/build: ../bin/testPlanarNormals

.PHONY : src/libDepthMap/CMakeFiles/testPlanarNormals.dir/build

src/libDepthMap/CMakeFiles/testPlanarNormals.dir/clean:
	cd /Users/dave/Animesh/cmake-build-debug/src/libDepthMap && $(CMAKE_COMMAND) -P CMakeFiles/testPlanarNormals.dir/cmake_clean.cmake
.PHONY : src/libDepthMap/CMakeFiles/testPlanarNormals.dir/clean

src/libDepthMap/CMakeFiles/testPlanarNormals.dir/depend:
	cd /Users/dave/Animesh/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dave/Animesh /Users/dave/Animesh/src/libDepthMap /Users/dave/Animesh/cmake-build-debug /Users/dave/Animesh/cmake-build-debug/src/libDepthMap /Users/dave/Animesh/cmake-build-debug/src/libDepthMap/CMakeFiles/testPlanarNormals.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/libDepthMap/CMakeFiles/testPlanarNormals.dir/depend

