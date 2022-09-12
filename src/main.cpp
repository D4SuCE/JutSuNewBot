#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include "server.h"
#include "token.h"
#include <chrono>
#include <string>
#include "../include/tgbot/tgbot.h"
#include <iostream>
#include <thread>
#include <vector>

void initializeKeyboard(TgBot::InlineKeyboardMarkup::Ptr& keyboard, const std::vector<std::vector<std::pair<std::string, std::string>>>& buttons)
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

std::vector<TgBot::BotCommand::Ptr> setMenuCommands(const std::vector<std::pair<std::string, std::string>>& commands)
{
	std::vector<TgBot::BotCommand::Ptr> bot_commands;
	for (auto it = commands.begin(); it != commands.end(); it++)
	{
		TgBot::BotCommand::Ptr command_i(new TgBot::BotCommand);
		command_i->command = it->first;
		command_i->description = it->second;
		bot_commands.push_back(command_i);
	}
	return bot_commands;
}

std::string help()
{
	std::string message = "Команды для работы с ботом:\n\n";
	message += "/myanimelist - вывести ваш список аниме\n";
	message += "/addnewanime - добавить новое аниме в список\n";
	message += "/removeanime - удалить аниме из списка\n";
	message += "/buttons - кнопочки\n";
	message += "/help - данное сообщение";
	return message;
}

int main()
{
	Server server;

	bool addNewAnime_flag = false;

	TgBot::Bot bot(token::token);

	bot.getApi().setMyCommands(setMenuCommands({
		{"myanimelist", "Ваш список аниме"},
		{"addnewanime", "Добавить новое аниме"},
		{"removeanime", "Удалить аниме"},
		{"buttons", "Показать кнопочки"},
		{"help", "Ознакомительное сообщение"}
		}));

	bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message)
	{
		bot.getApi().sendMessage(message->chat->id, help());
	});

	bot.getEvents().onCommand("help", [&bot](TgBot::Message::Ptr message)
	{
		bot.getApi().sendMessage(message->chat->id, help());
	});

	bot.getEvents().onCommand("buttons", [&bot](TgBot::Message::Ptr message)
	{
		TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
		initializeKeyboard(keyboard, {
			{{"Мои аниме", "myanimelist"}},
			{{"Добавить новое аниме", "addnewanime"}, {"Удалить аниме", "removeanime"}}
			});
		bot.getApi().sendMessage(message->chat->id, "Кнопочки:", false, 0, keyboard);
	});

	bot.getEvents().onCommand("myanimelist", [&bot, &server](TgBot::Message::Ptr message)
	{
		std::string list = server.getAnimeList(message->from->id);
		bot.getApi().sendMessage(message->chat->id, (list.empty() ? "Ваш список пуст :(" : "Ваш список аниме:\n\n" + list), true, 0, nullptr, "HTML");
	});

	bot.getEvents().onCommand("addnewanime", [&bot, &addNewAnime_flag](TgBot::Message::Ptr message)
	{
		addNewAnime_flag = true;
		bot.getApi().sendMessage(message->chat->id, "Введите ссылку на аниме");
	});

	bot.getEvents().onCommand("removeanime", [&bot, &server](TgBot::Message::Ptr message)
	{
		TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
		Keyboard_buttons user_anime = server.getUserAnime(message->from->id);
		if (user_anime.empty())
		{
			bot.getApi().sendMessage(message->chat->id, "Ваш список пуст");
		}
		else
		{
			initializeKeyboard(keyboard, user_anime);
			bot.getApi().sendMessage(message->chat->id, "Выберите аниме", false, 0, keyboard);
		}
	});

	bot.getEvents().onCallbackQuery([&](TgBot::CallbackQuery::Ptr query)
	{
		if (StringTools::startsWith(query->data, "myanimelist"))
		{
			std::string list = server.getAnimeList(query->message->chat->id);
			bot.getApi().sendMessage(query->message->chat->id, (list.empty() ? "Ваш список пуст :(" : "Ваш список аниме:\n\n" + list), true, 0, nullptr, "HTML");
		}
		else if (StringTools::startsWith(query->data, "addnewanime"))
		{
			addNewAnime_flag = true;
			bot.getApi().sendMessage(query->message->chat->id, "Введите ссылку на аниме");
		}
		else if (StringTools::startsWith(query->data, "removeanime"))
		{
			TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
			Keyboard_buttons user_anime = server.getUserAnime(query->from->id);
			if (user_anime.empty())
			{
				bot.getApi().sendMessage(query->message->chat->id, "Ваш список пуст");
			}
			else
			{
				initializeKeyboard(keyboard, user_anime);
				bot.getApi().sendMessage(query->message->chat->id, "Выберите аниме", false, 0, keyboard);
			}
		}
		else if (StringTools::startsWith(query->data, "help"))
		{
			bot.getApi().sendMessage(query->message->chat->id, help());
		}
		else
		{
			Anime_map anime_map = server.getAnimeMap(query->from->id);
			if (anime_map.find(query->data) != anime_map.end())
			{
				bot.getApi().sendMessage(query->message->chat->id, (server.removeAnime(query->from->id, query->data)));
			}
			else if (query->data == "deleteallanime")
			{
				bot.getApi().sendMessage(query->message->chat->id, (server.removeAllAnime(query->from->id)));
			}
		}
	});

	bot.getEvents().onAnyMessage([&](TgBot::Message::Ptr message)
	{
		if ((addNewAnime_flag) && (StringTools::startsWith(message->text, "https://jut.su/")))
		{
			bot.getApi().sendMessage(message->chat->id, server.addAnime(message->from->id, message->text));
			addNewAnime_flag = false;
			return;
		}
		else
		{
			addNewAnime_flag = false;
		}
	});

	try
	{
		std::cout << "Bot username: " << bot.getApi().getMe()->username << std::endl;
		std::cout << "Bot id: " << bot.getApi().getMe()->id << std::endl;
		TgBot::TgLongPoll longPoll(bot);

		bool quit = false;

		std::thread th_cmd([&bot, &quit]()
		{
			std::string cmd;
			while (std::cin >> cmd)
			{
				if (cmd == "stop")
				{
					quit = true;
					std::cout << "Stopping..." << std::endl;
				}
				else if (cmd == "count")
				{
					std::cout << "Count users: " << bot.getApi().getChatMemberCount(1008823422) << std::endl;
				}
			}
		});

		th_cmd.detach();

		std::thread th_checkNews([&bot, &server]()
		{
			while (true)
			{
				Users_map users = server.checkNews();

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
		});

		th_checkNews.detach();

		std::thread th_export([&server]()
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::minutes(60));
				server.exportAnime();
			}
		});

		th_export.detach();

		while (!quit)
		{
			std::cout << "Long Poll started" << std::endl;
			longPoll.start();
		}
	}
	catch (TgBot::TgException& e)
	{
		std::cout << "[ Error ] " << e.what() << std::endl;
	}

	return 0;
}
