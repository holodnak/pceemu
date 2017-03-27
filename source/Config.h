#pragma once

#include <string>
#include <list>

class CConfigVar
{
protected:
public:
	CConfigVar() {

	}
	~CConfigVar() {

	}
};

class CConfig
{
protected:
	bool FindConfig;

public:
	CConfig();
	virtual ~CConfig();

	std::string GetString() {

	}
};

