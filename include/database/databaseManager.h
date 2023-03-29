#pragma once

#include "data/data.h"
#include "types/types.h"

class DatabaseManager
{
public:
	virtual UsersMap checkNews() = 0;
	virtual AnimeMap getAnimeMap(ID id) = 0;
	virtual std::string getAnimeList(ID id) = 0;
	virtual std::string removeAllAnime(ID id) = 0;
	virtual KeyboardButtons getUserAnime(ID id) = 0;
	virtual Data getLastEpisodeInfo(const std::string& url) = 0;
	virtual std::string getAnimeName(const std::string& url) = 0;
	virtual std::string getAnimeUrlName(const std::string& url) = 0;
	virtual std::string getLastEpisodeUrl(const std::string& url) = 0;	
	virtual std::string addAnime(ID id, const std::string& url) = 0;
	virtual std::string removeAnime(ID id, const std::string& name) = 0;
	virtual ~DatabaseManager() = default;
};