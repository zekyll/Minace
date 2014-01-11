#pragma once

#include "../src/Process.h"
#include "../ttest/ttest.h"
#include <string>

namespace cm {

class PstreamTest : public ttest::TestBase
{
private:

#ifdef __unix__

	TTEST_CASE("Reading and writing works when running /bin/cat")
	{
		Process p("/bin/cat",{"--show-ends"});
		p.in() << "abc def\nghi" << std::endl;
		std::string s;
		getline(p.out(), s);
		TTEST_EQUAL(s, "abc def$");
		p.wait();
		getline(p.out(), s);
		TTEST_EQUAL(s, "ghi$");
	}

	TTEST_CASE("Throws if executable file not found.")
	{
		try {
			Process p("/file-not-found");
			throw ttest::TestException("Exception not thrown by constructor.");
		} catch (std::system_error& e) {
		}
	}

#endif

};

}