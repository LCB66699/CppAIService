#include <gtest/gtest.h>
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "router/Router.h"

using namespace http;
using namespace http::router;

// 辅助：方便构造带方法和路径的 HttpRequest
static void setMethodAndPath(HttpRequest& req,
                              HttpRequest::Method method,
                              const std::string& path) {
    // setMethod / setPath 使用 (const char* start, const char* end) 指针接口
    const char* methodStr = nullptr;
    size_t methodLen = 0;
    switch (method) {
        case HttpRequest::kGet:    methodStr = "GET";    methodLen = 3; break;
        case HttpRequest::kPost:   methodStr = "POST";   methodLen = 4; break;
        case HttpRequest::kHead:   methodStr = "HEAD";   methodLen = 4; break;
        case HttpRequest::kPut:    methodStr = "PUT";    methodLen = 3; break;
        case HttpRequest::kDelete: methodStr = "DELETE"; methodLen = 6; break;
        default: break;
    }
    req.setMethod(methodStr, methodStr + methodLen);
    req.setPath(path.c_str(), path.c_str() + path.size());
}

class RouterTest : public ::testing::Test {
protected:
    Router router;
};

// RouteKey 相等性
TEST_F(RouterTest, RouteKeyEquality) {
    Router::RouteKey key1{HttpRequest::kGet, "/api/test"};
    Router::RouteKey key2{HttpRequest::kGet, "/api/test"};
    Router::RouteKey key3{HttpRequest::kPost, "/api/test"};
    Router::RouteKey key4{HttpRequest::kGet, "/api/other"};

    EXPECT_EQ(key1, key2);
    EXPECT_FALSE(key1 == key3);
    EXPECT_FALSE(key1 == key4);
}

// RouteKey 哈希一致性
TEST_F(RouterTest, RouteKeyHashConsistency) {
    Router::RouteKeyHash hasher;
    Router::RouteKey key1{HttpRequest::kGet, "/api/test"};
    Router::RouteKey key2{HttpRequest::kGet, "/api/test"};
    Router::RouteKey key3{HttpRequest::kPost, "/api/test"};

    EXPECT_EQ(hasher(key1), hasher(key2));
    EXPECT_NE(hasher(key1), hasher(key3));
}

// 注册和匹配精确路由 — 回调形式
TEST_F(RouterTest, StaticRouteCallback) {
    bool called = false;
    router.registerCallback(HttpRequest::kGet, "/hello",
        [&called](const HttpRequest&, HttpResponse* resp) {
            called = true;
            resp->setStatusCode(HttpResponse::k200Ok);
        });

    HttpRequest req;
    setMethodAndPath(req, HttpRequest::kGet, "/hello");

    HttpResponse resp;
    bool routed = router.route(req, &resp);

    EXPECT_TRUE(routed);
    EXPECT_TRUE(called);
}

// 方法不匹配时不路由
TEST_F(RouterTest, MethodMismatch) {
    router.registerCallback(HttpRequest::kGet, "/hello",
        [](const HttpRequest&, HttpResponse* resp) {
            resp->setStatusCode(HttpResponse::k200Ok);
        });

    HttpRequest req;
    setMethodAndPath(req, HttpRequest::kPost, "/hello");

    HttpResponse resp;
    bool routed = router.route(req, &resp);

    EXPECT_FALSE(routed);
}

// 正则动态路由匹配
TEST_F(RouterTest, RegexRouteMatching) {
    bool called = false;
    router.addRegexCallback(HttpRequest::kGet, "/user/:id",
        [&called](const HttpRequest& req, HttpResponse* resp) {
            called = true;
            resp->setStatusCode(HttpResponse::k200Ok);
        });

    HttpRequest req;
    setMethodAndPath(req, HttpRequest::kGet, "/user/42");

    HttpResponse resp;
    bool routed = router.route(req, &resp);

    EXPECT_TRUE(routed);
    EXPECT_TRUE(called);
}

// 正则路由不匹配不应调用
TEST_F(RouterTest, RegexRouteNonMatching) {
    router.addRegexCallback(HttpRequest::kGet, "/user/:id",
        [](const HttpRequest&, HttpResponse* resp) {
            resp->setStatusCode(HttpResponse::k200Ok);
        });

    HttpRequest req;
    setMethodAndPath(req, HttpRequest::kGet, "/admin/42");

    HttpResponse resp;
    bool routed = router.route(req, &resp);

    EXPECT_FALSE(routed);
}

// 未注册路由返回 false
TEST_F(RouterTest, NoRouteFound) {
    HttpRequest req;
    setMethodAndPath(req, HttpRequest::kGet, "/nonexistent");

    HttpResponse resp;
    bool routed = router.route(req, &resp);

    EXPECT_FALSE(routed);
}
