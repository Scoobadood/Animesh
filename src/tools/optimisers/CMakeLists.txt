add_executable(
		posy_cli
		posy_cli.cpp
)

target_link_libraries(
		posy_cli
		PoSy
		Properties
		Surfel
		Cpd::Library-C++
		spdlog::spdlog
)

target_compile_features(
		posy_cli
		PUBLIC
		cxx_std_11
)

# Building rosy_cli
add_executable(
		rosy_cli
		rosy_cli.cpp
)

target_link_libraries(
		rosy_cli
		RoSy
		spdlog::spdlog
)

target_compile_features(
		rosy_cli
		PUBLIC
		cxx_std_11
)

add_executable(
		opt_cli
		opt_cli.cpp
		FieldOptimiser.h FieldOptimiser.cpp
)

target_link_libraries(
		opt_cli
		Surfel
		Geom
		RoSy
		PoSy
		spdlog::spdlog
)

target_compile_features(
		opt_cli
		PUBLIC
		cxx_std_11
)