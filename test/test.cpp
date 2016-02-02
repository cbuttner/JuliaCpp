#include "juliacpp.hpp"
#define CATCH_CONFIG_RUNNER

#include "catch.hpp"

int main(int argc, char* argv[])
{
	jlcpp::initJulia(JULIA_INIT_DIR);

	int result = Catch::Session().run(argc, argv);

	jlcpp::shutdownJulia();

	return result;
}
