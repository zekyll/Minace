#pragma once

#include "Logger.h"
#include <string>
#include <iostream>

namespace mnc {

/**
 * Simple logger that outputs to standard output.
 */
class StdOutLogger : public Logger
{
public:

	virtual void logMessage(const std::string& msg) override
	{
		std::cout << msg << std::endl;
	}

};

}
