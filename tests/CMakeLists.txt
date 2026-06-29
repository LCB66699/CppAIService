# 单元测试 (无外部依赖)
add_executable(test_router
    unit/test_router.cpp
)
target_link_libraries(test_router http_server_lib GTest::GTest GTest::Main pthread)
add_test(NAME Router.Unit COMMAND test_router)

# 集成测试 (需要服务运行)
find_package(CURL REQUIRED)

add_executable(test_api_login
    integration/test_api_login.cpp
)
target_link_libraries(test_api_login http_server_lib GTest::GTest GTest::Main ${CURL_LIBRARIES} pthread)
add_test(NAME ApiLogin.Integration COMMAND test_api_login)
set_tests_properties(ApiLogin.Integration PROPERTIES
    ENVIRONMENT "API_BASE_URL=${API_BASE_URL:-http://localhost:8117}"
)
