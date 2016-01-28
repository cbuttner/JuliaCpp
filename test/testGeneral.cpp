#include "juliacpp.hpp"
#include "catch.hpp"

TEST_CASE("Literals/rvalues")
{
	using namespace juliacpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	REQUIRE(module.call<bool>("roundtrip", false) == false);
	REQUIRE(module.call<bool>("roundtrip", true) == true);
	REQUIRE(module.call<int8_t>("roundtrip", (int8_t)-42) == -42);
	REQUIRE(module.call<int16_t>("roundtrip", (int16_t)-42) == -42);
	REQUIRE(module.call<int32_t>("roundtrip", (int32_t)-42) == -42);
	REQUIRE(module.call<int64_t>("roundtrip", (int64_t)-42) == -42);
	REQUIRE(module.call<uint8_t>("roundtrip", (uint8_t)42) == 42);
	REQUIRE(module.call<uint16_t>("roundtrip", (uint16_t)42) == 42);
	REQUIRE(module.call<uint32_t>("roundtrip", (uint32_t)42) == 42);
	REQUIRE(module.call<uint64_t>("roundtrip", (uint64_t)42) == 42);
	REQUIRE(module.call<float>("roundtrip", 1.234f) == 1.234f);
	REQUIRE(module.call<double>("roundtrip", 1.234) == 1.234);

	REQUIRE(module.call<std::string>("roundtrip", "tΣster") == "tΣster");
	REQUIRE(module.call<std::string>("roundtrip", std::string("tΣster")) == "tΣster");

	SECTION("Array literals")
	{
		REQUIRE((module.call<std::array<uint8_t, 3>>("roundtrip", std::array<uint8_t, 3> {1,2,3}) == std::array<uint8_t, 3> {1,2,3}));
		REQUIRE(module.call<std::vector<int>>("roundtrip", std::vector<int> {1,2,3,4}) == (std::vector<int> {1,2,3,4}));
		REQUIRE(module.call<std::vector<std::string>>("roundtrip", std::vector<const char*> {"a", "bc"}) == (std::vector<std::string> {"a", "bc"}));
	}

}

TEST_CASE("Multiple return")
{
	using namespace juliacpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	{
		int a;
		std::string b;
		std::vector<double> c;
		std::array<std::array<std::vector<int64_t>, 2>, 2> d;

		juliacpp::tie(a, b, c, d) = module.call("getMultiReturn");

		const std::vector<double> expected_c { 233.23, 2323.424221231, -2.232 };
		const std::array<std::array<std::vector<int64_t>, 2>, 2> expected_d { { {{{2},{1,4,-9}}},{{{},{2,4}}} } };

		REQUIRE(a == 24);
		REQUIRE(b == "tester");
		REQUIRE(c == expected_c);
		REQUIRE(d == expected_d);
	}

	{
		int a, b;
		juliacpp::tie(a, b) = module.call("roundtrip2", (int)1, (int)2);
		REQUIRE(a == 1);
		REQUIRE(b == 2);
	}
}

TEST_CASE("No file")
{
	using namespace juliacpp;
	JuliaModule module(jl_current_module);

	REQUIRE(module.call<double>("sqrt", 4.0) == Approx(2.0));
}

TEST_CASE("Call with reference")
{
	using namespace juliacpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	{
		int a;
		noAlloc(a) = module.call("roundtrip", 42);
		REQUIRE(a == 42);
	}

	{
		int a;
		double b;

		tieNoAlloc(a, b) = module.call("roundtrip2", 42, 123.321);
		REQUIRE(a == 42);
		REQUIRE(b == 123.321);
	}

	{
		const std::array<int64_t,3> in { 2, 4, 99999999999999 };
		std::array<int64_t,3> out { 0, 0, 0 };

		noAlloc(out) = module.call("roundtrip", in);
		REQUIRE(in == out);
	}

	{
		const int in[] { 2, 4, 3 };
		int out[] { 0, 0, 0 };

		noAlloc(out) = module.call("roundtrip", in);
		REQUIRE(std::equal(std::begin(in), std::end(in), std::begin(out)));
	}

	{
		const std::vector<int64_t> in1 { 2, 4, 99999999999999 };
		std::vector<int64_t> out1;
		out1.resize(in1.size());

		const std::array<bool,2> in2 { true, false };
		std::array<bool,2> out2 { false, false };

		tieNoAlloc(out1, out2) = module.call("roundtrip2", in1, in2);
		REQUIRE(in1 == out1);
		REQUIRE(in2 == out2);
	}

	{
		const std::array<std::array<int64_t,3>,2> in {{ { 2, 5, 13 }, { 1, 4, 9 } }};
		std::array<std::array<int64_t,3>,2> out {{ { 2, 5, 13 }, { 1, 4, 9 } }};

		noAlloc(out) = module.call("modifyNestedArray", in);
		REQUIRE_FALSE(in == out);
	}
}

TEST_CASE("ArrayPointer with reference")
{
	using namespace juliacpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	const std::string in1[] { "a", "bc", "def" };
	std::string out1[] { "", "", "" };

	double in2[] { 1.234, 2.345, 3.456 };
	const double in2copy[] { 1.234, 2.345, 3.456 };
	const double in2reverse[] { 3.456, 2.345, 1.234 };
	ArrayPointer<double> array(in2);

	SECTION("Roundtrip")
	{
		tieNoAlloc(out1, array) = module.call("roundtrip2", in1, array);
		REQUIRE(std::equal(std::begin(in1), std::end(in1), std::begin(out1)));
		REQUIRE(array == in2copy);
	}

	SECTION("Reverse")
	{
		noAlloc(array) = module.call("reverse", array);
		REQUIRE(array == in2reverse);
	}
}
