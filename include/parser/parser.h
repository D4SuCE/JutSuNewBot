#pragma once

#define CURL_STATICLIB

#include <string>
#include <curl/curl.h>

class Parser
{
private:
	CURL* curl;
	struct curl_slist* headers;
	std::string htmlBuffer;

public:
	Parser();
	~Parser();

private:
	static size_t writeCallBack(void* contents, size_t size, size_t nmemb, void* userp);

public:
	std::string parse(const std::string& url);
	int getStatusCode(const std::string& url) const;
};