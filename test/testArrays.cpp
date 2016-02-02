#include "juliacpp.hpp"
#include "catch.hpp"

TEST_CASE("Arrays")
{
	using namespace jlcpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	{
		const std::array<int64_t, 3> expected {23, 45, 67};
		std::array<int64_t, 3> array = module.call("getArray");

		REQUIRE(array == expected);
		array = module.call<decltype(array)>("getArray"); // explicit template parameter
		REQUIRE(array == expected);
	}

	{
		const std::vector<int64_t> expected {23, 45, 67};
		std::vector<int64_t> array = module.call("getArray");

		REQUIRE(array == expected);
		array = module.call<decltype(array)>("getArray"); // explicit template parameter
		REQUIRE(array == expected);
	}

	{
		const int64_t expected[3] {23, 45, 67};
		ArrayPointer<int64_t> array = module.call("getArray");

		REQUIRE(array == expected);
		delete[] array._data;

		array = module.call<decltype(array)>("getArray"); // explicit template parameter
		REQUIRE(array == expected);
		delete[] array._data;
	}
}

TEST_CASE("Nested arrays")
{
	using namespace jlcpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	{
		std::array<std::array<int64_t, 3>, 2> arrayOfArrays = module.call("getArrayOfArrays");
		const decltype(arrayOfArrays) expected {{{5,2,9}, {1,2,4}}};

		REQUIRE(arrayOfArrays == expected);

		arrayOfArrays = module.call<decltype(arrayOfArrays)>("getArrayOfArrays"); // explicit template parameter
		REQUIRE(arrayOfArrays == expected);
	}

	{
		std::array<std::vector<int64_t>, 2> arrayOfArrays = module.call("getArrayOfArrays2");
		const decltype(arrayOfArrays) expected {{{2,2}, {1,2,4}}};

		REQUIRE(arrayOfArrays == expected);

		arrayOfArrays = module.call<decltype(arrayOfArrays)>("getArrayOfArrays2"); // explicit template parameter
		REQUIRE(arrayOfArrays == expected);
	}
}

TEST_CASE("Array constness")
{
	using namespace jlcpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	{
		const double array[] { 34.123, 231.21, 12.23, 14.2 };
		const double arrayCopy[] = { array[0], array[1], array[2], array[3] };

		module.call("modifyArray", array);
		REQUIRE(std::equal(std::begin(array), std::end(array), std::begin(arrayCopy)));
	}

	{
		std::array<double, 4> array { 34.123, 231.21, 12.23, 14.2 };
		const std::array<double, 4> arrayCopy(array);

		module.call("modifyArray", array);
		REQUIRE_FALSE(std::equal(std::begin(array), std::end(array), std::begin(arrayCopy)));
	}

	{
		std::array<const double, 4> array { 34.123, 231.21, 12.23, 14.2 };
		const std::array<const double, 4> arrayCopy(array);

		module.call("modifyArray", array);
		REQUIRE(std::equal(std::begin(array), std::end(array), std::begin(arrayCopy)));
	}

	{
		const std::array<double, 4> array { 34.123, 231.21, 12.23, 14.2 };
		const std::array<double, 4> arrayCopy(array);

		module.call("modifyArray", array);
		REQUIRE(std::equal(std::begin(array), std::end(array), std::begin(arrayCopy)));
	}

	{
		std::vector<double> array { 34.123, 231.21, 12.23, 14.2 };
		const std::vector<double> arrayCopy(array);

		module.call("modifyArray", array);
		REQUIRE_FALSE(std::equal(std::begin(array), std::end(array), std::begin(arrayCopy)));
	}

	{
		const std::vector<double> array { 34.123, 231.21, 12.23, 14.2 };
		const std::vector<double> arrayCopy(array);

		module.call("modifyArray", array);
		REQUIRE(std::equal(std::begin(array), std::end(array), std::begin(arrayCopy)));
	}
}

TEST_CASE("Array constness for nested arrays")
{
	using namespace jlcpp;
	JuliaModule module("../test/test.jl", "JuliaCppTests");

	{
		std::array<const std::array<int, 3>, 2> arrayOfArrays{{{21, 234, 5}, {-23, 0, 55}}};
		const std::array<const std::array<int, 3>, 2> arrayOfArraysCopy(arrayOfArrays);

		module.call("modifyNestedArray", arrayOfArrays);
		REQUIRE(arrayOfArrays == arrayOfArraysCopy);
	}
	{
		const std::array<std::array<int, 3>, 2> arrayOfArrays{{{21, 234, 5}, {-23, 0, 55}}};
		const std::array<std::array<int, 3>, 2> arrayOfArraysCopy(arrayOfArrays);

		module.call("modifyNestedArray", arrayOfArrays);
		REQUIRE(arrayOfArrays == arrayOfArraysCopy);
	}
	{
		std::array<std::array<int, 3>, 2> arrayOfArrays{{{21, 234, 5}, {-23, 0, 55}}};
		const std::array<std::array<int, 3>, 2> arrayOfArraysCopy(arrayOfArrays);

		module.call("modifyNestedArray", arrayOfArrays);
		REQUIRE_FALSE(arrayOfArrays == arrayOfArraysCopy);
	}
	{
		std::array<std::array<int, 3>, 2> arrayOfArrays{{{21, 234, 5}, {-23, 0, 55}}};
		const std::array<std::array<int, 3>, 2> arrayOfArraysCopy(arrayOfArrays);

		module.call("modifyNestedArray", arrayOfArrays);
		REQUIRE_FALSE(arrayOfArrays == arrayOfArraysCopy);
	}
	{
		std::array<int, 3> arrayOfArrays[] {{21, 234, 5}, {-23, 0, 55}};
		const std::array<int, 3> arrayOfArraysCopy[] {{21, 234, 5}, {-23, 0, 55}};

		module.call("modifyNestedArray", arrayOfArrays);
		REQUIRE_FALSE(std::equal(std::begin(arrayOfArrays), std::end(arrayOfArrays), std::begin(arrayOfArraysCopy)));
	}
	{
		std::array<int[3], 2> arrayOfArrays {{{21, 234, 5}, {-23, 0, 55}}};
		const std::array<int[3], 2> arrayOfArraysCopy {{{21, 234, 5}, {-23, 0, 55}}};

		module.call("modifyNestedArray", arrayOfArrays);
		for (int i = 0; i < 2; i++)
			REQUIRE_FALSE(std::equal(std::begin(arrayOfArrays[i]), std::end(arrayOfArrays[i]), std::begin(arrayOfArraysCopy[i])));
	}
	{
		const std::array<int[3], 2> arrayOfArrays {{{21, 234, 5}, {-23, 0, 55}}};
		const std::array<int[3], 2> arrayOfArraysCopy {{{21, 234, 5}, {-23, 0, 55}}};

		module.call("modifyNestedArray", arrayOfArrays);
		for (int i = 0; i < 2; i++)
			REQUIRE(std::equal(std::begin(arrayOfArrays[i]), std::end(arrayOfArrays[i]), std::begin(arrayOfArraysCopy[i])));
	}
	{
		std::array<const int[3], 2> arrayOfArrays {{{21, 234, 5}, {-23, 0, 55}}};
		const std::array<const int[3], 2> arrayOfArraysCopy {{{21, 234, 5}, {-23, 0, 55}}};

		module.call("modifyNestedArray", arrayOfArrays);
		for (int i = 0; i < 2; i++)
			REQUIRE(std::equal(std::begin(arrayOfArrays[i]), std::end(arrayOfArrays[i]), std::begin(arrayOfArraysCopy[i])));
	}
	{
		int arrayOfArrays[2][3] {{21, 234, 5}, {-23, 0, 55}};
		const int arrayOfArraysCopy[2][3] {{21, 234, 5}, {-23, 0, 55}};

		module.call("modifyNestedArray", arrayOfArrays);
		for (int i = 0; i < 2; i++)
			REQUIRE_FALSE(std::equal(std::begin(arrayOfArrays[i]), std::end(arrayOfArrays[i]), std::begin(arrayOfArraysCopy[i])));
	}
	{
		const int arrayOfArrays[2][3] {{21, 234, 5}, {-23, 0, 55}};
		const int arrayOfArraysCopy[2][3] {{21, 234, 5}, {-23, 0, 55}};

		module.call("modifyNestedArray", arrayOfArrays);
		for (int i = 0; i < 2; i++)
			REQUIRE(std::equal(std::begin(arrayOfArrays[i]), std::end(arrayOfArrays[i]), std::begin(arrayOfArraysCopy[i])));
	}
}
