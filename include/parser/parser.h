#pragma once

#define CURL_STATICLIB

#include <string>
#include <curl/curl.h>

class Parser
{
private:
	static size_t writeCallBack(void* contents, size_t size, size_t nmemb, void* userp);
	static void initHeaders(struct curl_slist* headers);

public:
	static std::string parse(const std::string& url);
	static int64_t getStatusCode(const std::string& url);
};