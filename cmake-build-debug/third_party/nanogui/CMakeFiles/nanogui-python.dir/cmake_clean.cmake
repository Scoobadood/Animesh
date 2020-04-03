file(REMOVE_RECURSE
  "python/nanogui.cpython-37m-darwin.so"
  "python/nanogui.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C CXX)
  include(CMakeFiles/nanogui-python.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
