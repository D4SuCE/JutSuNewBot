#pragma once

#include <vector>
#include <unordered_map>

using ID = uint64_t;
using AnimeMap = std::unordered_map<std::string, Data>;
using AnimeIterator = AnimeMap::iterator;
using UsersMap = std::unordered_map<uint64_t, AnimeMap>;
using UsersMapIterator = UsersMap::iterator;
using KeyboardButtons = std::vector<std::vector<std::pair<std::string, std::string>>>;