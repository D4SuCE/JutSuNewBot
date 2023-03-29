#include <fstream>
#include <sstream>
#include <filesystem>
#include "database/localDatabase.h"

LocalDatabase::LocalDatabase()
{
	importAnime();
}

LocalDatabase::~LocalDatabase()
{
	exportAnime();
}

UsersMapIterator LocalDatabase::end()
{
	return users.end();
}

UsersMapIterator LocalDatabase::begin()
{
	return users.begin();
}

void LocalDatabase::importAnime()
{
	std::ifstream fs("database/users.txt");
	std::string line;
	std::vector<std::string> files;
	while (std::getline(fs, line))
	{
		files.push_back(line);
	}

	fs.close();

	for (auto& file : files)
	{
		fs.open(file);
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
			std::string strId = file.substr(20);
			strId = strId.substr(0, strId.find(".txt"));
			uint64_t id;
			std::istringstream isId(strId);
			isId >> id;
			users[id].insert(std::make_pair(anime_arr[0], Data{ name , std::stoi(anime_arr[2]), std::stoi(anime_arr[3]) }));
		}
		fs.close();
	}
}

void LocalDatabase::exportAnime()
{
	std::filesystem::create_directory("database");
	std::ofstream fs("database/users.txt");
	for (auto& user : users)
	{
		fs << "database/anime_list_" << user.first << ".txt" << std::endl;
	}
	fs.close();
	for (auto& user : users)
	{
		fs.open("database/anime_list_" + std::to_string(user.first) + ".txt");
		for (auto& anime : user.second)
		{
			std::string name = anime.second.name;
			std::replace(name.begin(), name.end(), ' ', '_');
			fs << anime.first << " " << name << " " << anime.second.season << " " << anime.second.episode << std::endl;
		}
		fs.close();
	}
}

AnimeMap& LocalDatabase::operator[](const ID& id)
{
	return users[id];
}

void LocalDatabase::insert(ID id, const AnimeMap& animeMap)
{
	users.insert(std::make_pair(id, animeMap));
}