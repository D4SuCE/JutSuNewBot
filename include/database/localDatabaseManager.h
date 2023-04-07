#pragma once

#include "data/data.h"
#include "localDatabase.h"
#include "databaseManager.h"

class LocalDatabaseManager : public DatabaseManager
{
private:
	LocalDatabase db;

public:
	LocalDatabaseManager(const LocalDatabase& db);
	~LocalDatabaseManager();

private:
	Data getLastEpisodeInfo(const std::string& url) override;
	std::string getAnimeName(const std::string& url) override;
	std::string getAnimeUrlName(const std::string& url) override;
	std::string getLastEpisodeUrl(const std::string& url) override;
public:
	void exportAnime();
	UsersMap checkNews() override;
	AnimeMap getAnimeMap(uint64_t id) override;
	std::string getAnimeList(uint64_t id) override;
	std::string removeAllAnime(uint64_t id) override;
	KeyboardButtons getUserAnime(uint64_t id) override;
	std::string addAnime(uint64_t id, const std::string& url) override;
	std::string removeAnime(uint64_t id, const std::string& name) override;
};