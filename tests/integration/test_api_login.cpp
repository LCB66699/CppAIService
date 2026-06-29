#include <gtest/gtest.h>
#include <curl/curl.h>
#include <string>
#include <cstdlib>

// 获取 API 根 URL
static std::string baseUrl() {
    const char* env = std::getenv("API_BASE_URL");
    return env ? env : "http://localhost:8117";
}

// libcurl 写回调
static size_t writeCb(void* contents, size_t size, size_t nmemb, std::string* out) {
    size_t total = size * nmemb;
    out->append(static_cast<char*>(contents), total);
    return total;
}

// 发送 POST JSON 请求
static std::string postJson(const std::string& url, const std::string& json) {
    std::string response;
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        ADD_FAILURE() << "curl error: " << curl_easy_strerror(res);
    }
    return response;
}

// 集成测试：登录端点可达性
// 需要 docker-compose up 启动完整服务栈
TEST(LoginApi, EndpointResponds) {
    std::string url = baseUrl() + "/login";
    std::string resp = postJson(url, R"({"username":"test","password":"123456"})");

    // 服务可达即可通过 (不要求登录成功)
    EXPECT_FALSE(resp.empty()) << "Server did not respond — is docker-compose up?";
    EXPECT_NE(resp.find("success"), std::string::npos)
        << "Response should be valid JSON with success field";
}

// 集成测试：入口页面可达
TEST(EntryApi, EntryPageResponds) {
    std::string url = baseUrl() + "/entry";
    std::string response;

    CURL* curl = curl_easy_init();
    ASSERT_NE(curl, nullptr);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    EXPECT_EQ(res, CURLE_OK) << "GET /entry failed: " << curl_easy_strerror(res);
    EXPECT_FALSE(response.empty()) << "Entry page should return HTML";
}
