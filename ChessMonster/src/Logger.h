#pragma once

#include <string>

namespace cm {

/**
 * Abstract logger interface.
 */
class Logger
{
public:
	virtual void logMessage(const std::string& msg) = 0;
	virtual ~Logger() = default;

protected:
	Logger() = default;
};

}
