#include <regex>
#include "parser/parser.h"
#include "iconvlite/iconvlite.h"
#include "database/localDatabaseManager.h"

LocalDatabaseManager::LocalDatabaseManager(const LocalDatabase& db)
	: db(db)
{
}

LocalDatabaseManager::~LocalDatabaseManager()
{
}

Data LocalDatabaseManager::getLastEpisodeInfo(const std::string& url)
{
	std::string fullUrl = getLastEpisodeUrl(url);
	std::string name = getAnimeName(url);
	int season = 1;
	int episode = 1;
	if (fullUrl.find("season-") != std::string::npos)
	{
		std::string seasonUrl = fullUrl;
		std::string episodeUrl = fullUrl;
		seasonUrl = seasonUrl.substr(seasonUrl.find("season-") + 7);
		seasonUrl = seasonUrl.substr(0, seasonUrl.find("/"));
		season = std::stoi(&seasonUrl[0]);
		episodeUrl = episodeUrl.substr(episodeUrl.find("episode-") + 8);
		episodeUrl = episodeUrl.substr(0, episodeUrl.find("/"));
		episode = std::stoi(&episodeUrl[0]);
	}
	else
	{
		std::string episodeUrl = fullUrl;
		episodeUrl = episodeUrl.substr(episodeUrl.find("episode-") + 8);
		episodeUrl = episodeUrl.substr(0, episodeUrl.find("/"));
		episode = std::stoi(&episodeUrl[0]);
	}
	return { name, season, episode };
}

std::string LocalDatabaseManager::getAnimeName(const std::string& url)
{
	std::string html = Parser::parse(url);
	std::string name = html.substr(html.find("<title>") + 7);
	name = name.substr(0, name.find("</title>"));
	std::string utfName = cp2utf(name);
	utfName = utfName.substr(17);
	utfName = utfName.substr(0, utfName.find(" все серии "));
	return utfName;
}

std::string LocalDatabaseManager::getAnimeUrlName(const std::string& url)
{
	std::string name = url.substr(url.find("su/") + 3);
	return name.substr(0, name.find("/"));
}

std::string LocalDatabaseManager::getLastEpisodeUrl(const std::string& url)
{
	std::string html = Parser::parse(url);
	std::string urlName = getAnimeUrlName(url);
	const std::regex regexFull("/" + urlName + "/season-+\\d+/episode-+\\d+.html");
	const std::regex regexSmall("/" + urlName + "/episode-+\\d+.html");
	std::vector<std::string> episodes;
	if (std::regex_search(html, regexFull))
	{
		episodes = { std::sregex_token_iterator(html.begin(), html.end(), regexFull, 0), std::sregex_token_iterator{} };
	}
	else
	{
		episodes = { std::sregex_token_iterator(html.begin(), html.end(), regexSmall, 0), std::sregex_token_iterator{} };
	}
	return episodes.back();
}

void LocalDatabaseManager::exportAnime()
{
	db.exportAnime();
}

UsersMap LocalDatabaseManager::checkNews()
{
	AnimeMap animeMap;
	UsersMap usersMap;

	for (auto& user : db)
	{
		for (auto& anime : user.second)
		{
			if (animeMap.find(anime.first) == animeMap.end())
			{
				const std::string domen = "https://jut.su/";
				Data data = getLastEpisodeInfo(domen + anime.first);

				if (anime.second.episode != data.episode)
				{
					animeMap.insert(std::make_pair(anime.first, data));
				}
			}
		}
	}

	for (auto& user : db)
	{
		for (auto& anime : animeMap)
		{
			if (user.second.find(anime.first) != user.second.end())
			{
				usersMap[user.first].insert(std::make_pair(anime.first, Data{ anime.second.name, anime.second.season, anime.second.episode }));
				user.second[anime.first].season = anime.second.season;
				user.second[anime.first].episode = anime.second.episode;
			}
		}
	}

	return usersMap;
}

AnimeMap LocalDatabaseManager::getAnimeMap(uint64_t id)
{
	return db[id];
}

std::string LocalDatabaseManager::getAnimeList(uint64_t id)
{
	std::string list = "";
	for (auto& anime : db[id])
	{
		list += "Название: <a href=\"https://jut.su/" + anime.first + "\">" + anime.second.name + "</a>" + "\n";
		list += "Сезон: " + std::to_string(anime.second.season) + " Серия: " + std::to_string(anime.second.episode) + "\n\n";
	}
	return list;
}

std::string LocalDatabaseManager::removeAllAnime(uint64_t id)
{
	db[id].clear();
	return "Все аниме удалены";
}

KeyboardButtons LocalDatabaseManager::getUserAnime(uint64_t id)
{
	if (db[id].empty())
	{
		return KeyboardButtons();
	}
	KeyboardButtons userAnime;
	std::vector<std::pair<std::string, std::string>> animeRow;
	int counter = 0;
	for (auto& anime : db[id])
	{
		if (counter < 3)
		{
			animeRow.push_back({ anime.second.name, anime.first });
			counter++;
		}
		else
		{
			userAnime.push_back(animeRow);
			animeRow.clear();
			counter = 0;
			animeRow.push_back({ anime.second.name, anime.first });
			counter++;
		}
	}
	userAnime.push_back(animeRow);
	userAnime.push_back({ { "Удалить все", "deleteallanime" } });
	return userAnime;
}

std::string LocalDatabaseManager::addAnime(uint64_t id, const std::string& url)
{
	long status_code = Parser::getStatusCode(url);
	if (status_code == 404)
	{
		return "Аниме не найдено";
	}
	else if (status_code == 200)
	{
		std::string urlName = getAnimeUrlName(url);
		if (db[id].find(urlName) == db[id].end())
		{
			db[id].insert(std::make_pair(urlName, getLastEpisodeInfo(url)));
			return "Аниме добавлено";
		}
		else
		{
			return (getAnimeName(url) + " уже есть в списке");
		}
	}
	else
	{
		return "Что-то пошло не так :(";
	}
}

std::string LocalDatabaseManager::removeAnime(uint64_t id, const std::string& name)
{
	if (db[id].erase(name))
	{
		return "Аниме удалено";
	}
	else
	{
		return "Что-то пошло не так :(";
	}
}