# JuliaCpp
A simple C++11 header-only library for calling Julia functions.

## Features
- Primitive types and strings
- Homogeneous, nested 1D arrays (C-arrays, std::array, std::vector)

# Usage
```c++
using namespace juliacpp;
JuliaModule module("path/to/file.jl", "ModuleName");

// No return value
module.call<void>("println", "Hello world!");

// Single return value
double result = module.call<double>("add", 2.4, 5.9);

// Multiple return values
int a;
std::string b;
std::tie(a, b) = module.call<int, std::string>("function", (uint8_t)2);
```
## Arrays
```c++
const int array[] { 1, 2, 3 };
const std::vector<int> array { 1, 2, 3 };
const std::array<int, 3> array { 1, 2, 3 };

std::vector<int> reversed;
reversed = module.call<std::vector<int>>("reverse", array);
// Or use decltype for convenience
reversed = module.call<decltype<reversed>>("reverse", array);

// Nested arrays
std::vector<int[3]> a1;
std::array<std::vector<bool>> a2;
int a3[][3] {{ 1, 2, 3 }, { 4, 5, 6 }};

std::tie(a1, a2) = module.call<decltype(a1), decltype(a2)>("function", a3);

```
If an array is passed as const, a new Julia array will be allocated and filled with data.
If you pass a non-const array, JuliaCpp will try to box the array without allocating new memory. This will make the array shared:
```c++
std::array<int, 3> array { 1, 2, 3 };
module.call<void>("reverse!", array);
assert(array == std::array<int, 3> { 3, 2, 1 });
```

## Return values by reference
You can also pass references which will used for the returned values.
This allows you to use already allocated arrays and C-style arrays as return types.
```c++
bool a;
std::vector<float> b; b.resize(64);
float c[128];
// No template parameters necessary here
module.call("function", OUT_BYREF(a, b, c), arg1, arg2);
```
