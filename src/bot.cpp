#include <thread>
#include <chrono>
#include "bot/bot.h"

Bot::Bot(const std::string& token)
	: bot(token), db(LocalDatabase()), addNewAnimeFlag(false)
{
}

Bot::~Bot()
{
}

void Bot::checkCMD(bool& quitFlag)
{
	std::string cmd;
	while (std::cin >> cmd)
	{
		if (cmd == "stop")
		{
			quitFlag = true;
			std::cout << "Stopping..." << std::endl;
		}
	}
}

void Bot::checkNews()
{
	while (true)
	{
		UsersMap users = db.checkNews();

		for (auto& user : users)
		{
			std::string list;
			for (auto& anime : user.second)
			{
				list += "Название: <a href=\"https://jut.su/" + anime.first + "\">" + anime.second.name + "</a>" + "\n";
				list += "Сезон: " + std::to_string(anime.second.season) + " Серия: " + std::to_string(anime.second.episode) + "\n";
				list += "https://jut.su/" + anime.first + "/season-" +
					std::to_string(anime.second.season) + "/episode-" +
					std::to_string(anime.second.episode) + ".html\n\n";
			}
			bot.getApi().sendMessage(user.first, "Новые серии:\n\n" + list, true, 0, nullptr, "HTML");
		}
		std::this_thread::sleep_for(std::chrono::minutes(30));
	}
}

void Bot::exportAnime()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::minutes(30));
		db.exportAnime();
	}
}

std::string Bot::showCommands()
{
	std::string message = "Команды для работы с ботом:\n\n";
	message += "/myanimelist - вывести ваш список аниме\n";
	message += "/addnewanime - добавить новое аниме в список\n";
	message += "/removeanime - удалить аниме из списка\n";
	message += "/buttons - кнопочки\n";
	message += "/help - данное сообщение";
	return message;
}

const std::vector<TgBot::BotCommand::Ptr> Bot::setMenuCommands()
{
	std::vector<std::pair<std::string, std::string>> commands = {
		{"myanimelist", "Ваш список аниме"},
		{"addnewanime", "Добавить новое аниме"},
		{"removeanime", "Удалить аниме"},
		{"buttons", "Показать кнопочки"},
		{"help", "Ознакомительное сообщение"}
	};

	std::vector<TgBot::BotCommand::Ptr> botCommands;

	for (auto it = commands.begin(); it != commands.end(); it++)
	{
		TgBot::BotCommand::Ptr iCommand(new TgBot::BotCommand);
		iCommand->command = it->first;
		iCommand->description = it->second;
		botCommands.push_back(iCommand);
	}
	return botCommands;
}

void Bot::initializeKeyboard(TgBot::InlineKeyboardMarkup::Ptr& keyboard, const std::vector<std::vector<std::pair<std::string, std::string>>>& buttons)
{
	for (int i = 0; i < buttons.size(); i++)
	{
		std::vector<TgBot::InlineKeyboardButton::Ptr> row;
		for (auto it = buttons[i].begin(); it != buttons[i].end(); it++)
		{
			TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton);
			button->text = it->first;
			button->callbackData = it->second;
			row.push_back(button);
		}
		keyboard->inlineKeyboard.push_back(row);
	}
}

void Bot::run()
{
	bot.getApi().setMyCommands(setMenuCommands());

	bot.getEvents().onCommand("start", [this](TgBot::Message::Ptr message)
	{
		bot.getApi().sendMessage(message->chat->id, showCommands());
	});

	bot.getEvents().onCommand("help", [this](TgBot::Message::Ptr message)
	{
		bot.getApi().sendMessage(message->chat->id, showCommands());
	});

	bot.getEvents().onCommand("buttons", [this](TgBot::Message::Ptr message)
	{
		TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
		initializeKeyboard(keyboard, {
			{{"Мои аниме", "myanimelist"}},
			{{"Добавить новое аниме", "addnewanime"}, {"Удалить аниме", "removeanime"}}
			});
		bot.getApi().sendMessage(message->chat->id, "Кнопочки:", false, 0, keyboard);
	});

	bot.getEvents().onCommand("myanimelist", [this](TgBot::Message::Ptr message)
	{
		std::string list = db.getAnimeList(message->from->id);
		bot.getApi().sendMessage(message->chat->id, (list.empty() ? "Ваш список пуст :(" : "Ваш список аниме:\n\n" + list), true, 0, nullptr, "HTML");
	});

	bot.getEvents().onCommand("addnewanime", [this](TgBot::Message::Ptr message)
	{
		addNewAnimeFlag = true;
		bot.getApi().sendMessage(message->chat->id, "Введите ссылку на аниме");
	});

	bot.getEvents().onCommand("removeanime", [this](TgBot::Message::Ptr message)
	{
		TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
		KeyboardButtons userAnime = db.getUserAnime(message->from->id);
		if (!userAnime.empty())
		{
			initializeKeyboard(keyboard, userAnime);
			bot.getApi().sendMessage(message->chat->id, "Выберите аниме", false, 0, keyboard);
		}
		else
		{
			bot.getApi().sendMessage(message->chat->id, "Ваш список пуст");
		}
	});

	bot.getEvents().onCallbackQuery([this](TgBot::CallbackQuery::Ptr query)
	{
		if (StringTools::startsWith(query->data, "myanimelist"))
		{
			std::string list = db.getAnimeList(query->message->chat->id);
			bot.getApi().sendMessage(query->message->chat->id, (list.empty() ? "Ваш список пуст :(" : "Ваш список аниме:\n\n" + list), true, 0, nullptr, "HTML");
		}
		else if (StringTools::startsWith(query->data, "addnewanime"))
		{
			addNewAnimeFlag = true;
			bot.getApi().sendMessage(query->message->chat->id, "Введите ссылку на аниме");
		}
		else if (StringTools::startsWith(query->data, "removeanime"))
		{
			TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
			KeyboardButtons userAnime = db.getUserAnime(query->from->id);
			if (!userAnime.empty())
			{
				initializeKeyboard(keyboard, userAnime);
				bot.getApi().sendMessage(query->message->chat->id, "Выберите аниме", false, 0, keyboard);
			}
			else
			{
				bot.getApi().sendMessage(query->message->chat->id, "Ваш список пуст");
			}
		}
		else if (StringTools::startsWith(query->data, "help"))
		{
			bot.getApi().sendMessage(query->message->chat->id, showCommands());
		}
		else
		{
			AnimeMap animeMap = db.getAnimeMap(query->from->id);
			if (animeMap.find(query->data) != animeMap.end())
			{
				bot.getApi().sendMessage(query->message->chat->id, (db.removeAnime(query->from->id, query->data)));
			}
			else if (query->data == "deleteallanime")
			{
				bot.getApi().sendMessage(query->message->chat->id, (db.removeAllAnime(query->from->id)));
			}
		}
	});

	bot.getEvents().onAnyMessage([this](TgBot::Message::Ptr message)
	{
		if ((addNewAnimeFlag) && (StringTools::startsWith(message->text, "https://jut.su/")))
		{
			bot.getApi().sendMessage(message->chat->id, db.addAnime(message->from->id, message->text));
		}
		addNewAnimeFlag = false;
	});

	try
	{
		TgBot::TgLongPoll longPoll(bot);

		bool quitFlag = false;

		std::thread th_cmd(&Bot::checkCMD, this, std::ref(quitFlag));
		th_cmd.detach();

		std::thread th_checkNews(&Bot::checkNews, this);
		th_checkNews.detach();

		std::thread th_export(&Bot::exportAnime, this);
		th_export.detach();

		while (!quitFlag)
		{
			longPoll.start();
		}
	}
	catch (TgBot::TgException& e)
	{
		db.exportAnime();
		std::cout << "[ Error ] " << e.what() << std::endl;
	}
}
