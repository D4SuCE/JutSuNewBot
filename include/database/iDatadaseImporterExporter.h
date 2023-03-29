#pragma once

class IDatadaseImporterExporter
{
public:
	virtual void importAnime() = 0;
	virtual void exportAnime() = 0;
	virtual ~IDatadaseImporterExporter() = default;
};