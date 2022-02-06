# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/field_visualiser_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/field_visualiser_autogen.dir/ParseCache.txt"
  "field_visualiser_autogen"
  )
endif()
