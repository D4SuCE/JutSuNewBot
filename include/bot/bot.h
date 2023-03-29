#pragma once

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <vector>
#include "tgbot/tgbot.h"
#include "database/localDatabaseManager.h"

class Bot
{
private:
	TgBot::Bot bot;
	LocalDatabaseManager db;
	bool addNewAnimeFlag;

public:
	Bot(const std::string& token);
	~Bot();

private:
	void checkNews();
	void exportAnime();
	void checkCMD(bool& quitFlag);

private:
	std::string showCommands();
	const std::vector<TgBot::BotCommand::Ptr> setMenuCommands();
	void initializeKeyboard(TgBot::InlineKeyboardMarkup::Ptr& keyboard, const std::vector<std::vector<std::pair<std::string, std::string>>>& buttons);

public:
	void run();
};