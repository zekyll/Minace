#pragma once

#include <string>
#include <cstring>

namespace cm {

template<typename ...TArgs>
std::string strFormat(size_t len, const char* fmt, TArgs ...args)
{
	std::unique_ptr<char[] > buf(new char[len]);
	std::snprintf(buf.get(), len, fmt, args...);
	return buf.get();
}

}
