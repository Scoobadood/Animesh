# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/posy_visualiser_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/posy_visualiser_autogen.dir/ParseCache.txt"
  "posy_visualiser_autogen"
  )
endif()
