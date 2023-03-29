#include "parser/parser.h"
#include "details/details.h"

Parser::Parser()
	: curl(curl_easy_init()), headers(NULL)
{
	headers = curl_slist_append(headers, details::authority);
	headers = curl_slist_append(headers, details::accept);
	headers = curl_slist_append(headers, details::accept_language);
	headers = curl_slist_append(headers, details::cache_control);
	headers = curl_slist_append(headers, details::cookie);
	headers = curl_slist_append(headers, details::sec_ch_ua);
	headers = curl_slist_append(headers, details::sec_ch_ua_mobile);
	headers = curl_slist_append(headers, details::sec_ch_ua_platform);
	headers = curl_slist_append(headers, details::sec_fetch_dest);
	headers = curl_slist_append(headers, details::sec_fetch_mode);
	headers = curl_slist_append(headers, details::sec_fetch_site);
	headers = curl_slist_append(headers, details::sec_fetch_user);
	headers = curl_slist_append(headers, details::upgrade_insecure_requests);
	headers = curl_slist_append(headers, details::user_agent);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallBack);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlBuffer);
}

Parser::~Parser()
{
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
}

std::string Parser::parse(const std::string& url)
{
	htmlBuffer = "";
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_perform(curl);
	return htmlBuffer;
}

int Parser::getStatusCode(const std::string& url) const
{
	long httpCode = 0;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	return httpCode;
}

size_t Parser::writeCallBack(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
