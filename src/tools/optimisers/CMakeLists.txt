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
		multi_res_cli
		multi_res_cli.cpp
)

target_link_libraries(
		multi_res_cli
		MultiResolutionOptimiser
)

target_compile_features(
		multi_res_cli
		PUBLIC
		cxx_std_11
)