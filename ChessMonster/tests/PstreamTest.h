#pragma once

#include "../src/Pstream.h"
#include "../ttest/ttest.h"
#include <string>

namespace cm {

class PstreamTest : public ttest::TestBase
{
private:

#ifdef __unix__

	TTEST_CASE("Reading and writing works when running /bin/cat")
	{
		Pstream ps("/bin/cat",{"--show-ends"});
		ps << "abc def\nghi" << std::endl;
		std::string s;
		getline(ps, s);
		TTEST_EQUAL(s, "abc def$");
		ps.wait();
		getline(ps, s);
		TTEST_EQUAL(s, "ghi$");
	}

	TTEST_CASE("Throws if executable file not found.")
	{
		try {
			Pstream ps("/file-not-found");
			throw ttest::TestException("Exception not thrown by constructor.");
		} catch (std::system_error& e) {
		}
	}

#endif

};

}