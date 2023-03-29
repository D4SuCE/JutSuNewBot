#pragma once

#include "data/data.h"
#include "types/types.h"
#include "iDatadaseImporterExporter.h"

class LocalDatabase : public IDatadaseImporterExporter
{
private:
	UsersMap users;

public:
	LocalDatabase();
	~LocalDatabase();

public:
	UsersMapIterator end();	
	UsersMapIterator begin();
	void importAnime() override;
	void exportAnime() override;
	AnimeMap& operator[] (const ID& id);
	void insert(ID id, const AnimeMap& animeMap);
};