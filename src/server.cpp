#include "server.h"
#include "details.h"
#include "iconvlite.h"
#include <experimental/filesystem>
#include <mutex>
#include <sstream>
#include <fstream>
#include <string>
#include <regex>

Server::Server()
	: curl_(curl_easy_init()), headers_(NULL)
{
	headers_ = curl_slist_append(headers_, details::authority);
	headers_ = curl_slist_append(headers_, details::accept);
	headers_ = curl_slist_append(headers_, details::accept_language);
	headers_ = curl_slist_append(headers_, details::cache_control);
	headers_ = curl_slist_append(headers_, details::cookie);
	headers_ = curl_slist_append(headers_, details::sec_ch_ua);
	headers_ = curl_slist_append(headers_, details::sec_ch_ua_mobile);
	headers_ = curl_slist_append(headers_, details::sec_ch_ua_platform);
	headers_ = curl_slist_append(headers_, details::sec_fetch_dest);
	headers_ = curl_slist_append(headers_, details::sec_fetch_mode);
	headers_ = curl_slist_append(headers_, details::sec_fetch_site);
	headers_ = curl_slist_append(headers_, details::sec_fetch_user);
	headers_ = curl_slist_append(headers_, details::upgrade_insecure_requests);
	headers_ = curl_slist_append(headers_, details::user_agent);
	curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
	curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallBack);
	curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &html_buffer_);
	importAnime();
}

Server::~Server()
{
	curl_easy_cleanup(curl_);
	curl_slist_free_all(headers_);
	exportAnime();
}

void Server::importAnime()
{
	std::fstream fs;
	fs.open("database/users.txt", std::fstream::in);
	
	std::string line;
	std::vector<std::string> files;
	while (std::getline(fs, line))
	{
		files.push_back(line);
	}
	
	fs.close();
	
	for (auto& file : files)
	{
		fs.open(file, std::fstream::in);
		while (std::getline(fs, line))
		{
			std::stringstream ss(line);
			std::string word;
			std::string anime_arr[4];
			for (int i = 0; ss >> word; i++)
			{
				anime_arr[i] = word;
			}
			std::string name = anime_arr[1];
			std::replace(name.begin(), name.end(), '_', ' ');
			std::string s_id = file.substr(20);
			s_id = s_id.substr(0, s_id.find(".txt"));
			uint64_t id;
			std::istringstream isid(s_id);
			isid >> id;
			Anime_map anime;
			anime.insert(std::make_pair(anime_arr[0], Data{ name , std::stoi(anime_arr[2]), std::stoi(anime_arr[3])}));
			users_[id].push_back(anime);
		}
		fs.close();
	}
}

void Server::exportAnime()
{
	std::lock_guard<std::mutex> lock(mtx_export_);
	std::fstream fs;
	std::experimental::filesystem::create_directory("database");
	fs.open("database/users.txt", std::fstream::out);
	for (auto& user : users_)
	{
		fs << "database/anime_list_" << user.first << ".txt" << std::endl;
	}
	fs.close();
	for (auto& user : users_)
	{
		fs.open("database/anime_list_" + std::to_string(user.first) + ".txt", std::fstream::out);
		for (auto& anime_map : user.second)
		{
			for (auto& anime : anime_map)
			{
				std::string name = anime.second.name;
				std::replace(name.begin(), name.end(), ' ', '_');
				fs << anime.first << " " << name << " " << anime.second.season << " " << anime.second.episode << std::endl;
			}
		}
		fs.close();
	}
}

size_t Server::WriteCallBack(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

std::string Server::getAnimeName(const std::string& url)
{
	parseHtml(url);
	std::string name = html_buffer_.substr(html_buffer_.find("<title>") + 7);
	name = name.substr(0, name.find("</title>"));
	std::string utf_name = cp2utf(name);
	utf_name = utf_name.substr(17);
	utf_name = utf_name.substr(0, utf_name.find(" все серии "));
	return utf_name;
}

std::string Server::getAnimeUrlName(const std::string& url)
{
	std::string name = url.substr(url.find("su/") + 3);
	return name.substr(0, name.find("/"));
}

void Server::parseHtml(const std::string& url)
{
	html_buffer_ = "";
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	curl_easy_perform(curl_);
}

Data Server::getData(const std::string& url)
{
	std::string full_url = getLastEpisodeUrl(url);
	std::string name = getAnimeName(url);
	int season = 1;
	int episode = 1;
	if (full_url.find("season-") != std::string::npos)
	{
		std::string season_url = full_url;
		std::string episode_url = full_url;
		season_url = season_url.substr(season_url.find("season-") + 7);
		season_url = season_url.substr(0, season_url.find("/"));
		season = std::stoi(&season_url[0]);
		episode_url = episode_url.substr(episode_url.find("episode-") + 8);
		episode_url = episode_url.substr(0, episode_url.find("/"));
		episode = std::stoi(&episode_url[0]);
	}
	else
	{
		std::string episode_url = full_url;
		episode_url = episode_url.substr(episode_url.find("episode-") + 8);
		episode_url = episode_url.substr(0, episode_url.find("/"));
		episode = std::stoi(&episode_url[0]);
	}
	return { name, season, episode };
}

std::string Server::getLastEpisodeUrl(const std::string& url)
{
	parseHtml(url);
	std::string url_name = getAnimeUrlName(url);
	const std::regex regex_full("/" + url_name + "/season-+\\d+/episode-+\\d+.html");
	const std::regex regex_small("/" + url_name + "/episode-+\\d+.html");
	std::vector<std::string> episodes;
	if (std::regex_search(html_buffer_, regex_full))
	{
		episodes = { std::sregex_token_iterator(html_buffer_.begin(), html_buffer_.end(), regex_full, 0), std::sregex_token_iterator{} };
	}
	else
	{
		episodes = { std::sregex_token_iterator(html_buffer_.begin(), html_buffer_.end(), regex_small, 0), std::sregex_token_iterator{} };
	}
	return episodes.back();
}

int Server::getStatusCode(const std::string& url)
{
	long http_code = 0;
	curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
	curl_easy_perform(curl_);
	curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
	return http_code;
}

std::string Server::addAnime(uint64_t id, const std::string& url)
{
	int status_code = getStatusCode(url);
	if (status_code == 404)
	{
		return "Аниме не найдено";
	}
	else if (status_code == 200)
	{
		std::string url_name = getAnimeUrlName(url);
		if (std::find_if(users_[id].begin(), users_[id].end(), [&url_name](Anime_map& anime_map)
		{
			return anime_map.find(url_name) != anime_map.end();
		}) != users_[id].end())
		{
			return (url_name + " уже есть в списке");
		}
		else
		{
			Anime_map anime;
			anime.insert(std::make_pair(url_name, getData(url)));
			users_[id].push_back(anime);
			return "Аниме добавлено";
		}
	}
	else
	{
		return "Что-то пошло не так :(";
	}
}

std::string Server::removeAnime(uint64_t id, const std::string& url)
{
	std::string url_name = getAnimeUrlName(url);
	if (std::find_if(users_[id].begin(), users_[id].end(), [&url_name](Anime_map& anime_map)
	{
			return anime_map.find(url_name) != anime_map.end();
	}) != users_[id].end())
	{
		users_[id].erase(std::remove_if(users_[id].begin(), users_[id].end(), [&url_name](Anime_map& anime_map)
		{
			return anime_map.find(url_name) != anime_map.end();
		}));
		return "Аниме удалено";
	}
	else
	{
		return (url_name + " нет в вашем списке");
	}
}

std::string Server::getAnimeList(uint64_t id)
{
	std::string list = "";
	for (auto& anime_map : users_[id])
	{
		for (auto& anime : anime_map)
		{
			list += "Название: <a href=\"https://jut.su/" + anime.first + "\">" + anime.second.name + "</a>" + "\n";
			list += "Сезон: " + std::to_string(anime.second.season) + " Серия: " + std::to_string(anime.second.episode) + "\n\n";

		}
	}
	return list;
}

Users_map Server::checkNews()
{
	Anime_map anime_map;
	Users_map users_map;
	for (auto& user : users_)
	{
		for (auto& anime_list : user.second)
		{
			for (auto& anime : anime_list)
			{
				if (anime_map.find(anime.first) == anime_map.end())
				{
					std::string domen = "https://jut.su/";
					Data data = getData(domen + anime.first);
				
					if (anime.second.episode != data.episode)
					{
						anime_map.insert(std::make_pair(anime.first, data));
					}
				}
			}
		}
	}

	for (auto& user : users_)
	{
		for (auto& anime_list : user.second)
		{
			for (auto& anime : anime_map)
			{
				if (anime_list.find(anime.first) != anime_list.end())
				{
					Anime_map reserve_anime;
					reserve_anime.insert(std::make_pair(anime.first, Data { anime.second.name, anime.second.season, anime.second.episode }));
					users_map[user.first].push_back(reserve_anime);
					anime_list[anime.first].season = anime.second.season;
					anime_list[anime.first].episode = anime.second.episode;
				}
			}
		}
	}

	return users_map;
}

