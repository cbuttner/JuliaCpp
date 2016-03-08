# JuliaCpp

A simple C++11 header-only library for calling Julia functions.


## Features

- Primitive types and strings
- Homogeneous, nested 1D arrays (C-arrays, std::array and std::vector)
- Multiple return values (tuples)
- Keyword arguments
- Error handling


## Building

If your Julia root directory is not in `/usr` (e.g. because you compile from
git) then you'll need to tell cmake where to find the headers and libs.  This
is done by

```
mkdir build && cd build
cmake .. -Djulia-root-dir=/path/to/julia.git/julia-COMMIT
```

or wherever julia has placed its built `usr` directory.


## Usage

JuliaCpp is a wrapper for the Julia C API. See
http://docs.julialang.org/en/stable/manual/embedding/ for instructions on how
to link against Julia.

```c++
using namespace jlcpp;

initJulia("/usr/lib"); // or just initJulia();
JuliaModule module("path/to/file.jl", "ModuleName");

// No return value
module.call("println", "Hello world!");

// Single return value
double result = module.call("add", 2.4, 5.9);
// Specifying the template parameter explicitly can be useful or even
// necessary in some cases
double result = module.call<double>("add", 2.4, 5.9);

// Multiple return values
int a;
std::string b;
jlcpp::tie(a, b) = module.call("function", (uint8_t)2);

shutdownJulia();
```
Note that we need to use `jlcpp::tie` instead of `std::tie` for returned
tuples.  Also, when using `jlcpp::tie`, template parameters are never
required.


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

jlcpp::tie(a1, a2) = module.call("function", a3);

```

If an array is passed as const, a new Julia array will be allocated and filled
with data.  If you pass a non-const array, JuliaCpp will try to box the array
without allocating new memory. This will make the array shared:

```c++
std::array<int, 3> array { 1, 2, 3 };
module.call("reverse!", array);
assert(array == std::array<int, 3> { 3, 2, 1 });
```

#### Indicate that arrays should not be allocated
To use already allocated arrays and C-style arrays as return types, JuliaCpp
provides `tieNoAlloc` and `noAlloc`:

```c++
bool a;
std::vector<float> b; b.resize(64);
float c[128];
tieNoAlloc(a, b, c) = module.call("function", arg1, arg2);

// Special (but not required) syntax for single return values:
noAlloc(b) = module.call("function", arg1, arg2);
```

You can also use `ArrayPointer<T>` to wrap a pointer and an array size.
`ArrayPointer<T>` is useful if you need to have data copied into previously
allocated memory which is not wrapped by one of the standard array/vector
containers.

```c++
double* pointer;
const size_t len = 128;
ArrayPointer<double> array(pointer, len);

noAlloc(array) = module.call("reverse", array);
```

Note that `ArrayPointer<T>` is also supported as a direct (not a `noAlloc`)
return value, but the `_data` pointer has to be freed (with `delete[]`)
manually.


### Keyword arguments

An optional `KeywordArgs` can be added to the arguments. It doesn't matter at
which position the `KeywordArgs` is passed since it is filtered out
separately. Passing more than one `KeywordArgs` will cause a compile time
error.

```c++
module.call("function", arg1, arg2, KeywordArgs("kw1", kw1value)("kw2", 
            kw2value));
// or module.call("function", KeywordArgs("kw1", kw1value)("kw2", kw2value), 
//                arg1, arg2);
```

Keyword arguments are appended to `KeywordArgs` by using the bracket operator
for each key-value pair.

```c++
const size_t size = 4;
std::array<double, size> rand = module.call("rand", size);

double x[size] { 1, 2, 3, 4 };
module.call("plot", KeywordArgs("x", x)("y", rand));
```


### Error handling

JuliaCpp takes care of type checking returned values and handling Julia
exceptions when calling a function or loading a module. In these instances,
JuliaCpp will throw an exception of type `JuliaCppException`.  For Julia
exceptions, a useful error message can't be extracted right now, but in
addition to throwing a `JuliaCppException`, the exception object will be
printed to the *stderr* output by using `jl_static_show`.


### JuliaModule

The JuliaModule class can be used in the following ways:

```c++
JuliaModule module("path/to/file.jl", "ModuleName"); // will load the file and the module
JuliaModule module("path/to/file.jl"); // will load the file but no module
JuliaModule module(jl_current_module); // will load no file but a specific jl_module_t*

module.reload(); // Reload the file and the module (if specified)
```


### Manual mode

You can use `jl_value_t*` directly and still make use of some of JuliaCpp's
features:

```c++
jl_value_t* value = module.call("roundtrip", (int)2).getJuliaValue();
int a = unboxJuliaValue<int>(value);

value = module.call("roundtrip2", (int)2, "tester").getJuliaValue();
std::string b;
juliacpp::tie(a, b) = unboxJuliaValue(value);
```


## Roadmap

Features that would be nice to have in the future:
- Multi-dimensional arrays
- Using Julia-internal allocated array data directly without copying the
  memory (however this would require preventing the Julia GC from freeing the
  memory)

Pull requests are always welcome.
