#include "juliacpp.hpp"
#include "catch.hpp"

TEST_CASE("Errors")
{
	using namespace jlcpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	REQUIRE_THROWS_AS(module.call<void>("errorFunction"), JuliaCppException);
	REQUIRE_THROWS_AS(module.call<void>("getArray", 24.24), JuliaCppException);
	REQUIRE_THROWS_AS(module.call<float>("roundtrip", (double)123.4123), JuliaCppException);
	REQUIRE_THROWS_AS(module.call<std::string>("roundtrip", true), JuliaCppException);
	REQUIRE_THROWS_AS((module.call<std::string, bool>("roundtrip2", true, "not a bool")), JuliaCppException);

	bool a, b, c;
	REQUIRE_THROWS_AS((jlcpp::tie(a, b) = module.call("roundtrip", true)), JuliaCppException);
	REQUIRE_THROWS_AS((module.call<bool>("roundtrip2", true, false)), JuliaCppException);
	REQUIRE_THROWS_AS((jlcpp::tie(a, b, c) = module.call("roundtrip2", true, false)), JuliaCppException);

	REQUIRE_THROWS_AS(module.call<std::vector<uint64_t>>("roundtrip", "not a vector"), JuliaCppException);
	REQUIRE_THROWS_AS(module.call<std::string>("getArrayOfArrays"), JuliaCppException);
	REQUIRE_THROWS_AS(module.call<std::vector<int>>("getArrayOfArrays"), JuliaCppException);

	{
		int array1[3];
		int64_t array2[2];
		REQUIRE_THROWS_AS(noAlloc(array1) = module.call("getArray"), JuliaCppException);
		REQUIRE_THROWS_AS(noAlloc(array2) = module.call("getArray"), JuliaCppException);
	}

	{
		std::vector<std::vector<double>> arrayOfArrays;
		REQUIRE_THROWS_AS(arrayOfArrays = module.call<decltype(arrayOfArrays)>("getArrayOfArrays2"), JuliaCppException);
	}

	{
		std::vector<std::vector<double>> arrayOfArrays;
		REQUIRE_THROWS_AS(noAlloc(arrayOfArrays) = module.call("getArrayOfArrays2"), JuliaCppException);
		arrayOfArrays.resize(2);
		arrayOfArrays[0].resize(2);
		arrayOfArrays[1].resize(3);
		REQUIRE_THROWS_AS(noAlloc(arrayOfArrays) = module.call("getArrayOfArrays2"), JuliaCppException);
	}

	// function does not accept keyword arguments
	REQUIRE_THROWS_AS(module.call("roundtrip", 42, KeywordArgs("a", 0)), JuliaCppException);

	REQUIRE_THROWS_AS(module.call<void>("NOTEXISTING"), JuliaCppException);
	REQUIRE_THROWS_AS(JuliaModule errorModule("NOTEXISTING.jl"), JuliaCppException);
	REQUIRE_THROWS_AS(JuliaModule errorModule("NOTEXISTING.jl", "NOTEXISTING_MODULE"), JuliaCppException);
	REQUIRE_THROWS_AS(JuliaModule errorModule("../test/test.jl", "NOTEXISTING_MODULE"), JuliaCppException);
}
