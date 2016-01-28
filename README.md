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

#### Indicate that arrays should not be allocated
To use already allocated arrays and C-style arrays as return types, JuliaCpp provides `tieNoAlloc` and `noAlloc`:
```c++
bool a;
std::vector<float> b; b.resize(64);
float c[128];
tieNoAlloc(a, b, c) = module.call("function", arg1, arg2);

// Special (but not required) syntax for single return values:
noAlloc(b) = module.call("function", arg1, arg2);
```

You can also use `ArrayPointer<T>` to wrap a pointer and an array size.
`ArrayPointer<T>` is useful if you need to have data copied into previously allocated memory which is not wrapped by one of the standard array/vector containers.
```c++
double* pointer;
const size_t len = 128;
ArrayPointer<double> array(pointer, len);

noAlloc(array) = module.call("reverse", array);
```
Note that `ArrayPointer<T>` is also supported as a direct (not a `noAlloc`) return value, but the `_data` pointer has to be freed (with `delete[]`) manually.

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

### Manual mode
You can use `jl_value_t*` directly and still make use of some of JuliaCpp's features:
```c++
jl_value_t* value = module.call("roundtrip", (int)2).getJuliaValue();
int a = unboxJuliaValue<int>(value);

value = module.call("roundtrip2", (int)2, "tester").getJuliaValue();
std::string b;
juliacpp::tie(a, b) = unboxJuliaValue(value);
```
