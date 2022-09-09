#pragma once

#define CURL_STATICLIB

#include <curl/curl.h>
#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>

struct Data
{
	std::string name;
	int season;
	int episode;
};

typedef std::unordered_map<std::string, Data> Anime_map;
typedef std::unordered_map<uint64_t, std::vector<Anime_map>> Users_map;

class Server
{
private:
	CURL* curl_;
	struct curl_slist* headers_;
	Users_map users_;
	std::string html_buffer_;
	std::mutex mtx_export_;

public:
	Server();
	~Server();

private:
	void importAnime();
	static size_t WriteCallBack(void* contents, size_t size, size_t nmemb, void* userp);
	std::string getAnimeName(const std::string& url);
	std::string getAnimeUrlName(const std::string& url);
	void parseHtml(const std::string& url);
	Data getData(const std::string& url);
	std::string getLastEpisodeUrl(const std::string& url);
	int getStatusCode(const std::string& url);
	
public:
	void exportAnime();
	std::string addAnime(uint64_t id, const std::string& url);
	std::string removeAnime(uint64_t id, const std::string& url);
	std::string getAnimeList(uint64_t id);
	Users_map checkNews();
};
