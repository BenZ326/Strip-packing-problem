find_package(GTest QUIET)

if(GTest_FOUND)
	add_executable(solver_utils_test
		gtest_solver_utils_test.cpp
	)

	target_link_libraries(solver_utils_test
		PRIVATE
		strip_packing_core
		GTest::gtest
		GTest::gtest_main
	)

	target_include_directories(solver_utils_test
		PRIVATE
		${CMAKE_SOURCE_DIR}/src
	)

	include(GoogleTest)
	gtest_discover_tests(solver_utils_test)
else()
	message(WARNING "GTest not found. Skipping solver_utils_test target.")
endif()

add_test(
	NAME spp_cli_completed
	COMMAND $<TARGET_FILE:spp> --time_buget 1 --problem_path ${CMAKE_SOURCE_DIR}/test/2sp/HT01.TXT
)
set_tests_properties(spp_cli_completed PROPERTIES
	PASS_REGULAR_EXPRESSION "state: completed"
)

add_test(
	NAME spp_cli_ht01_height_regression
	COMMAND $<TARGET_FILE:spp> --time_buget 60 --problem_path ${CMAKE_SOURCE_DIR}/test/2sp/HT01.TXT
)
set_tests_properties(spp_cli_ht01_height_regression PROPERTIES
	PASS_REGULAR_EXPRESSION "state: completed[\r\n]+height: 20"
)

add_test(
	NAME spp_cli_beng01_height_regression
	COMMAND $<TARGET_FILE:spp> --time_buget 60 --problem_path ${CMAKE_SOURCE_DIR}/test/2sp/BENG01.TXT
)
set_tests_properties(spp_cli_beng01_height_regression PROPERTIES
	PASS_REGULAR_EXPRESSION "state: completed[\r\n]+height: 30"
)

add_test(
	NAME spp_cli_timeout_zero_budget
	COMMAND $<TARGET_FILE:spp> --time_buget 0 --problem_path ${CMAKE_SOURCE_DIR}/test/2sp/NGCUT12.TXT
)
set_tests_properties(spp_cli_timeout_zero_budget PROPERTIES
	PASS_REGULAR_EXPRESSION "state: timeout"
)

add_test(
	NAME spp_cli_time_budget_alias
	COMMAND $<TARGET_FILE:spp> --time_budget 1 --problem_path ${CMAKE_SOURCE_DIR}/test/2sp/HT01.TXT
)
set_tests_properties(spp_cli_time_budget_alias PROPERTIES
	PASS_REGULAR_EXPRESSION "state: completed"
)

add_test(
	NAME spp_cli_unknown_argument
	COMMAND $<TARGET_FILE:spp> --bad_flag 1
)
set_tests_properties(spp_cli_unknown_argument PROPERTIES
	WILL_FAIL TRUE
)
