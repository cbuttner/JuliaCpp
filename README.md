# JuliaCpp
A simple C++11 header-only library for calling Julia functions.

## Features
- Call Julia functions with:
 - Primitive types and strings
 - Homogeneous, nested 1D arrays (C-arrays, std::array and std::vector)

## Usage
```c++
using namespace juliacpp;

initJulia("/usr/lib"); // or just initJulia();
JuliaModule module("path/to/file.jl", "ModuleName");

// No return value
module.call("println", "Hello world!");

// Single return value
double result = module.call("add", 2.4, 5.9);
// Specifying the template parameter explicitly can be useful or even necessary in some cases
double result = module.call<double>("add", 2.4, 5.9);

// Multiple return values
int a;
std::string b;
juliacpp::tie(a, b) = module.call("function", (uint8_t)2);

shutdownJulia();
```
Note that we need to use `juliacpp::tie` instead of `std::tie` for returned tuples.
Also, when using `juliacpp::tie`, template parameters are never required.
### Arrays
```c++
const int array[] { 1, 2, 3 };
const std::vector<int> array { 1, 2, 3 };
const std::array<int, 3> array { 1, 2, 3 };

std::vector<int> reversed = module.call("reverse", array);
// Or specify explicitly (with decltype for convenience)
reversed = module.call<decltype<reversed>>("reverse", array);

// Nested arrays
std::vector<int[3]> a1;
std::array<std::vector<bool>> a2;
int a3[][3] {{ 1, 2, 3 }, { 4, 5, 6 }};

juliacpp::tie(a1, a2) = module.call("function", a3);

```
If an array is passed as const, a new Julia array will be allocated and filled with data.
If you pass a non-const array, JuliaCpp will try to box the array without allocating new memory. This will make the array shared:
```c++
std::array<int, 3> array { 1, 2, 3 };
module.call("reverse!", array);
assert(array == std::array<int, 3> { 3, 2, 1 });
```

### Return values by reference
You can also pass references which will used for the returned values.
This allows you to use already allocated arrays and C-style arrays as return types.
```c++
bool a;
std::vector<float> b; b.resize(64);
float c[128];
// No template parameters necessary here
module.call("function", OUT_BYREF(a, b, c), arg1, arg2);
```
To unambiguously indicate which arguments should hold the references, we use `OUT_BYREF(...)`.

You can also use `ArrayPointer<T>` to wrap a pointer and an array size.
```c++
double* pointer;
const size_t len = 128;
ArrayPointer<double> array(pointer, len);

module.call("reverse", OUT_BYREF(array), array);
```
Note that `ArrayPointer<T>` is also supported as a direct return value, but the `_data` pointer has to be freed (with `delete[]`) manually.

### Error handling
JuliaCpp takes care of type checking returned values and handling Julia exceptions when calling a function or loading a module. In these instances, JuliaCpp will throw an exception of type `JuliaCppException`.
For Julia exceptions, a useful error message can't be extracted right now, but in addition to throwing a `JuliaCppException`, the exception object will be printed to the *stderr* output by using `jl_static_show`.

### JuliaModule
The JuliaModule class can be used in the following ways:
```c++
JuliaModule module("path/to/file.jl", "ModuleName"); // will load the file and the module
JuliaModule module("path/to/file.jl"); // will load the file but no module
JuliaModule module(jl_current_module); // will load no file but a specific jl_module_t*

module.reload(); // Reload the file and the module (if specified)
```
