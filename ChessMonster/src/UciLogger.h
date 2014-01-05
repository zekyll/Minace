#pragma once

#include <string>

namespace cm {

/* Observes UCI traffic. */
class UciLogger
{
public:

	virtual void onOutput(const std::string& msg)
	{
	}

	virtual void onInput(const std::string& msg)
	{
	}

	virtual ~UciLogger()
	{
	};
};

}
