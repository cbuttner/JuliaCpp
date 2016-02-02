#pragma once

#include <julia.h>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <typeindex>
#include <memory>
#include <cstring> // std::memcpy
#include <iostream>
#include <sstream>
#include <type_traits>

namespace jlcpp
{

class JuliaCppException : public std::exception
{
public:
	
	JuliaCppException(const char* expression, const char* file, int line, const std::string& message)
		: _expression(expression), _file(file), _line(line), _message(message)
	{
		std::ostringstream outputStream;

		if (!message.empty())
			outputStream << "Error: " << message << " ";

		outputStream << "Assertion '" << expression << "'";
		outputStream << " failed in file '" << file << "' line " << line << ".";
		_what = outputStream.str();
	}

	virtual const char* what() const throw()
	{
		return _what.c_str();
	}

private:
	const char* _expression;
	const char* _file;
	int _line;
	std::string _message;
	std::string _what;
};

#define JULIACPP_ASSERT(EXPRESSION, MESSAGE) if(!(EXPRESSION)) { throw JuliaCppException(#EXPRESSION, __FILE__, __LINE__, MESSAGE); }
#define JULIACPP_ASSERT_NOMSG(EXPRESSION) if(!(EXPRESSION)) { throw JuliaCppException(#EXPRESSION, __FILE__, __LINE__, ""); }

//#define ENABLE_LOG

#ifdef ENABLE_LOG
inline void log(const char* f, ...)
{
	va_list args;
	va_start(args, f);
	vprintf(f, args);
	va_end(args);
	printf("\n");
}
#else
inline void log(const char*, ...)
{
}
#endif

template<typename T>
struct ArrayPointer
{
	ArrayPointer() { }

	ArrayPointer(T* data, size_t len)
		: _data(data), _len(len)
	{
	}

	template <std::size_t N>
	ArrayPointer(T(&array)[N])
		: _data(array), _len(N)
	{
	}

	T* _data;
	size_t _len;

	size_t size() const	{ return _len; }
	T* data() { return _data; }
	const T* data() const { return _data; }

	T& operator[](size_t i) { return _data[i]; }
	const T& operator[](size_t i) const { return _data[i]; }

	bool operator==(const ArrayPointer<T>& rhs) const
	{
		return std::equal(_data, _data + _len, rhs._data);
	}

	template<typename TArray>
	bool operator==(const TArray& rhs) const
	{
		return std::equal(_data, _data + _len, std::begin(rhs));
	}
};


namespace Impl
{
	template <std::size_t... Is>
	struct Indices {};

	template <std::size_t N, std::size_t... Is>
	struct IndicesBuilder : IndicesBuilder<N-1, N-1, Is...> {};

	template <std::size_t... Is>
	struct IndicesBuilder<0, Is...>
	{
		using type = Indices<Is...>;
	};

	template <typename T>
	using UnqualifiedType = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

	template <typename T> inline jl_datatype_t* dataTypeOf()
	{
		static_assert(sizeof(T) == -1, "No jl_datatype_t found.");
		return nullptr;
	}

	template <> inline jl_datatype_t* dataTypeOf<bool>() { return jl_bool_type; }
	template <> inline jl_datatype_t* dataTypeOf<char>() { return jl_char_type; }
	template <> inline jl_datatype_t* dataTypeOf<int8_t>() { return jl_int8_type; }
	template <> inline jl_datatype_t* dataTypeOf<int16_t>() { return jl_int16_type; }
	template <> inline jl_datatype_t* dataTypeOf<int32_t>() { return jl_int32_type; }
	template <> inline jl_datatype_t* dataTypeOf<int64_t>() { return jl_int64_type; }
	template <> inline jl_datatype_t* dataTypeOf<uint8_t>() { return jl_uint8_type; }
	template <> inline jl_datatype_t* dataTypeOf<uint16_t>() { return jl_uint16_type; }
	template <> inline jl_datatype_t* dataTypeOf<uint32_t>() { return jl_uint32_type; }
	template <> inline jl_datatype_t* dataTypeOf<uint64_t>() { return jl_uint64_type; }
	template <> inline jl_datatype_t* dataTypeOf<float>() { return jl_float32_type; }
	template <> inline jl_datatype_t* dataTypeOf<double>() { return jl_float64_type; }
	template <> inline jl_datatype_t* dataTypeOf<std::string>() { return jl_utf8_string_type; }
	template <> inline jl_datatype_t* dataTypeOf<const char*>() { return jl_utf8_string_type; }


	template<typename T>
	struct TypeTraits
	{
		static constexpr bool isPtrArray = false;
		static inline jl_datatype_t* dataType() { return dataTypeOf<UnqualifiedType<T>>(); }
	};

	template<>
	struct TypeTraits<std::string>
	{
		static constexpr bool isPtrArray = true;
		static inline jl_datatype_t* dataType() { return dataTypeOf<std::string>(); }
	};

	template<>
	struct TypeTraits<const char*>
	{
		static constexpr bool isPtrArray = true;
		static jl_datatype_t* dataType() { return dataTypeOf<const char*>(); }
	};

	template<typename T, size_t size>
	struct TypeTraits<T[size]>
	{
		static constexpr bool isPtrArray = true;
		static jl_datatype_t* dataType() { return jl_array_type; }
	};

	template<typename T>
	struct TypeTraits<ArrayPointer<T>>
	{
		static constexpr bool isPtrArray = true;
		static jl_datatype_t* dataType() { return jl_array_type; }
	};

	template<typename T, size_t N>
	struct TypeTraits<std::array<T, N>>
	{
		static constexpr bool isPtrArray = true;
		static jl_datatype_t* dataType() { return jl_array_type; }
	};

	template<typename T>
	struct TypeTraits<std::vector<T>>
	{
		static constexpr bool isPtrArray = true;
		static jl_datatype_t* dataType() { return jl_array_type; }
	};


	template <typename T>
	using ValueIfPtrArray = typename std::enable_if<TypeTraits<UnqualifiedType<T>>::isPtrArray, jl_value_t*>::type;
	template <typename T>
	using ValueIfNotPtrArray = typename std::enable_if<!TypeTraits<UnqualifiedType<T>>::isPtrArray, jl_value_t*>::type;


	inline jl_value_t* box(bool val) { return jl_box_bool(val); }
	inline jl_value_t* box(char val) { return jl_box_char((uint32_t)val); }
	inline jl_value_t* box(int8_t val) { return jl_box_int8(val); }
	inline jl_value_t* box(int16_t val) { return jl_box_int16(val); }
	inline jl_value_t* box(int32_t val) { return jl_box_int32(val); }
	inline jl_value_t* box(int64_t val) { return jl_box_int64(val); }
	inline jl_value_t* box(uint8_t val) { return jl_box_uint8(val); }
	inline jl_value_t* box(uint16_t val) { return jl_box_uint16(val); }
	inline jl_value_t* box(uint32_t val) { return jl_box_uint32(val); }
	inline jl_value_t* box(uint64_t val) { return jl_box_uint64(val); }
	inline jl_value_t* box(float val) { return jl_box_float32(val); }
	inline jl_value_t* box(double val) { return jl_box_float64(val); }
	// Encoding for individual strings is handled by Julia. Julia strings are immutable.
	inline jl_value_t* box(const std::string& val) { return jl_cstr_to_string(val.c_str()); }
	inline jl_value_t* box(const char* val) { return jl_cstr_to_string(val); }

	template<typename T> ValueIfNotPtrArray<T> boxArray(T* data, size_t size);
	template<typename T> ValueIfNotPtrArray<T> boxArray(const T* data, size_t size);
	template<typename T> ValueIfPtrArray<T> boxArray(T* data, size_t size);
	template<typename T> ValueIfPtrArray<T> boxArray(const T* data, size_t size);

	template<typename T, size_t N>
	inline jl_value_t* box(T(& val)[N]) { return boxArray<T>(val, N); }

	template<typename T>
	inline jl_value_t* box(ArrayPointer<T>& val) { return boxArray<T>(val._data, val._len); }

	template<typename T, size_t N>
	inline jl_value_t* box(std::array<T, N>& val) { return boxArray<T>(val.data(), val.size()); }

	template<typename T, size_t N>
	inline jl_value_t* box(const std::array<T, N>& val) { return boxArray<T>(val.data(), val.size()); }

	template<typename T>
	inline jl_value_t* box(std::vector<T>& val) { return boxArray<T>(val.data(), val.size()); }

	template<typename T>
	inline jl_value_t* box(const std::vector<T>& val) { return boxArray<T>(val.data(), val.size()); }

	template<typename T>
	ValueIfNotPtrArray<T> boxArray(T* data, size_t size)
	{
		log("boxArray(T*) - jl_ptr_to_array_1d");

		jl_datatype_t* dataType = TypeTraits<UnqualifiedType<T>>::dataType();
		JULIACPP_ASSERT(dataType != nullptr, "Data type not supported.");

		jl_value_t* arrayType = jl_apply_array_type(dataType, 1);
		jl_array_t* array = jl_ptr_to_array_1d(arrayType, data, size, 0);

		return (jl_value_t*)array;
	}

	template<typename T>
	ValueIfNotPtrArray<T> boxArray(const T* data, size_t size)
	{
		log("boxArray(const T*) - jl_alloc_array_1d and memcpy");

		jl_datatype_t* dataType = TypeTraits<UnqualifiedType<T>>::dataType();
		JULIACPP_ASSERT(dataType != nullptr, "Data type not supported.");

		jl_value_t* arrayType = jl_apply_array_type(dataType, 1);
		jl_array_t* array = jl_alloc_array_1d(arrayType, size);

		std::memcpy(jl_array_data(array), data, sizeof(T) * size);

		return (jl_value_t*)array;
	}

	template<typename T>
	ValueIfPtrArray<T> boxArray(T* data, size_t size)
	{
		log("boxArray(T*) - jl_alloc_array_1d and unbox per element");

		jl_datatype_t* dataType = TypeTraits<UnqualifiedType<T>>::dataType();
		JULIACPP_ASSERT(dataType != nullptr, "Data type not supported.");

		jl_value_t* arrayType = jl_apply_array_type(dataType, 1);

		jl_array_t* array = jl_alloc_array_1d(arrayType, size);
		jl_value_t** arrayData = (jl_value_t**)jl_array_data(array);
		for (size_t i = 0; i < size; i++)
		{
			arrayData[i] = box(data[i]);
		}

		return (jl_value_t*)array;
	}

	template<typename T>
	ValueIfPtrArray<T> boxArray(const T* data, size_t size)
	{
		log("boxArray(const T*) - jl_alloc_array_1d and unbox per element");

		jl_datatype_t* dataType = TypeTraits<UnqualifiedType<T>>::dataType();
		JULIACPP_ASSERT(dataType != nullptr, "Data type not supported.");

		jl_value_t* arrayType = jl_apply_array_type(dataType, 1);

		jl_array_t* array = jl_alloc_array_1d(arrayType, size);
		jl_value_t** arrayData = (jl_value_t**)jl_array_data(array);
		for (size_t i = 0; i < size; i++)
		{
			arrayData[i] = box(data[i]);
		}

		return (jl_value_t*)array;
	}


	template<typename T>
	static T unbox(jl_value_t*)
	{
		static_assert(sizeof(T) == -1, "No jl_value_t* unbox for type.");
	}

	template <> inline void unbox<void>(jl_value_t*) { }
	template <> inline bool unbox<bool>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_bool(val)); return (bool)jl_unbox_bool(val); }
	template <> inline int8_t unbox<int8_t>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_int8(val)); return jl_unbox_int8(val); }
	template <> inline int16_t unbox<int16_t>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_int16(val)); return jl_unbox_int16(val); }
	template <> inline int32_t unbox<int32_t>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_int32(val)); return jl_unbox_int32(val); }
	template <> inline int64_t unbox<int64_t>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_int64(val)); return jl_unbox_int64(val); }
	template <> inline uint8_t unbox<uint8_t>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_uint8(val)); return jl_unbox_uint8(val); }
	template <> inline uint16_t unbox<uint16_t>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_uint16(val)); return jl_unbox_uint16(val); }
	template <> inline uint32_t unbox<uint32_t>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_uint32(val)); return jl_unbox_uint32(val); }
	template <> inline uint64_t unbox<uint64_t>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_uint64(val)); return jl_unbox_uint64(val); }
	template <> inline float unbox<float>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_float32(val)); return jl_unbox_float32(val); }
	template <> inline double unbox<double>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_float64(val)); return jl_unbox_float64(val); }
	template <> inline std::string unbox<std::string>(jl_value_t* val) { JULIACPP_ASSERT_NOMSG(jl_is_byte_string(val)); return std::string(jl_string_data(val)); }
	// Disabled for now, can be garbage collected by Julia
	//template <> inline const char* unbox<const char*>(jl_value_t* val) { return jl_string_data(val); }

	namespace Unboxer
	{
		template<typename... TReturns>
		struct ValueUnboxer
		{
			typedef std::tuple<TReturns...> type;

			template <typename T>
			static T unboxAt(jl_value_t* value, size_t index)
			{
				log("unboxing tuple at index %d", (int)index);
				return ValueUnboxer<T>::apply(jl_fieldref(value, index));
			}

			template <typename... T, std::size_t... N>
			static std::tuple<T...> makeTuple(jl_value_t* value, Indices<N...>)
			{
				return std::make_tuple(unboxAt<T>(value, N)...);
			}

			static type apply(jl_value_t* value)
			{
				constexpr auto numReturns = sizeof...(TReturns);
				static_assert(numReturns > 1, "This function call is for tuples.");

				JULIACPP_ASSERT(jl_is_tuple(value), "Returned value is not a tuple.");
				JULIACPP_ASSERT(jl_nfields(value) == numReturns, "Julia did not return the expected number of values.")

				return makeTuple<TReturns...>(value, typename IndicesBuilder<numReturns>::type());
			}
		};

		template <typename TArray, typename TElem>
		static void unboxArray(jl_value_t* val, TArray& array)
		{
			static_assert(!std::is_const<TElem>::value, "Cannot unbox array of const type.");
			JULIACPP_ASSERT(jl_is_array(val), "jl_value_t is not an array.");

			jl_array_t* jlArray = (jl_array_t*)val;

			const auto size = array.size();
			JULIACPP_ASSERT(jlArray->length == size, "Invalid array length.");
			if (jlArray->ptrarray)
			{
				jl_value_t** data = (jl_value_t**)jlArray->data;
				for (size_t i = 0; i < size; i++)
				{
					array[i] = ValueUnboxer<TElem>::apply(data[i]);
				}
			}
			else
			{
				JULIACPP_ASSERT(jl_array_eltype(val) == TypeTraits<TElem>::dataType(), "Unexpected jl_array_t element type.");
				std::memcpy(array.data(), jlArray->data, sizeof(TElem) * size);
			}
		}

		template<>
		struct ValueUnboxer<>
		{
		};

		template<typename T>
		struct ValueUnboxer<T>
		{
			typedef T type;

			static type apply(jl_value_t* value)
			{
				return unbox<T>(value);
			}
		};

		template<typename T, size_t N>
		struct ValueUnboxer<std::array<T, N>>
		{
			typedef std::array<T, N> type;

			static type apply(jl_value_t* val)
			{
				std::array<T, N> array;
				unboxArray<std::array<T, N>, T>(val, array);
				return array;
			}
		};

		template<typename T>
		struct ValueUnboxer<std::vector<T>>
		{
			typedef std::vector<T> type;

			static type apply(jl_value_t* val)
			{
				JULIACPP_ASSERT(jl_is_array(val), "Unboxing std::vector: jl_value_t is not an array.");
				std::vector<T> array;
				array.resize(jl_array_len(val));

				unboxArray<std::vector<T>, T>(val, array);
				return array;
			}
		};

		template<typename T>
		struct ValueUnboxer<ArrayPointer<T>>
		{
			typedef ArrayPointer<T> type;

			static type apply(jl_value_t* val)
			{
				JULIACPP_ASSERT(jl_is_array(val), "Unboxing ArrayPointer: jl_value_t is not an array.");
				const auto len = jl_array_len(val);
				ArrayPointer<T> array(new T[len], len);

				unboxArray<ArrayPointer<T>, T>(val, array);
				return array;
			}
		};
	} // namespace Unboxer

	namespace RefUnboxer
	{
		template <typename TArray, typename T>
		static void unboxArrayByRef(jl_value_t* val, TArray& array);

		template<typename... TReturns>
		struct RefValueUnboxer
		{
			template <typename T>
			static void unboxAt(jl_value_t* value, size_t index, T& outVal)
			{
				log("unboxing tuple at index %d", (int)index);
				RefValueUnboxer<T>::apply(jl_fieldref(value, index), outVal);
			}

			template <typename T>
			static void rec(jl_value_t* value, size_t idx, T& outVal)
			{
				unboxAt(value, idx, outVal);
			}

			template<typename T, typename... TArgs>
			static void rec(jl_value_t* value, size_t idx, T& outVal, TArgs&... outVals)
			{
				rec(value, idx, outVal);
				rec(value, idx+1, outVals...);
			}

			template<std::size_t... N>
			static void unboxTuple(jl_value_t* value, std::tuple<TReturns&...>& tuple, Indices<N...>)
			{
				rec<TReturns...>(value, 0, std::get<N>(tuple)...);
			}

			static void apply(jl_value_t* value, std::tuple<TReturns&...>& tuple)
			{
				constexpr auto numReturns = sizeof...(TReturns);
				static_assert(numReturns > 1, "This function call is for tuples.");

				JULIACPP_ASSERT(jl_is_tuple(value), "Returned value is not a tuple.");
				JULIACPP_ASSERT(jl_nfields(value) == numReturns, "Julia did not return the expected number of values.");

				unboxTuple(value, tuple, typename IndicesBuilder<numReturns>::type());
			}
		};

		template<typename T>
		struct RefValueUnboxer<T>
		{
			static void apply(jl_value_t* value, T& outVal)
			{
				outVal = unbox<T>(value);
			}
		};

		template<typename T, size_t N>
		struct RefValueUnboxer<std::array<T, N>>
		{
			typedef std::array<T, N> type;

			static void apply(jl_value_t* val, type& outArray)
			{
				unboxArrayByRef<std::array<T, N>, T>(val, outArray);
			}
		};

		template<typename T>
		struct RefValueUnboxer<std::vector<T>>
		{
			typedef std::vector<T> type;

			static void apply(jl_value_t* val, type& outArray)
			{
				unboxArrayByRef<std::vector<T>, T>(val, outArray);
			}
		};

		template<typename T>
		struct RefValueUnboxer<ArrayPointer<T>>
		{
			typedef ArrayPointer<T> type;

			static void apply(jl_value_t* val, type& outArray)
			{
				unboxArrayByRef<ArrayPointer<T>, T>(val, outArray);
			}
		};

		template<typename T, size_t N>
		struct RefValueUnboxer<T[N]>
		{
			static void apply(jl_value_t* val, T(& outArray)[N])
			{
				ArrayPointer<T> a(outArray);
				unboxArrayByRef<ArrayPointer<T>, T>(val, a);
			}
		};

		template <typename TArray, typename TElem>
		static void unboxArrayByRef(jl_value_t* val, TArray& array)
		{
			static_assert(!std::is_const<TElem>::value, "Cannot unbox array of const type.");
			JULIACPP_ASSERT(jl_is_array(val), "jl_value_t is not an array.");

			jl_array_t* jlArray = (jl_array_t*)val;

			const auto size = array.size();
			JULIACPP_ASSERT(jlArray->length == size, "Invalid array length.");
			if (jlArray->ptrarray)
			{
				jl_value_t** data = (jl_value_t**)jlArray->data;
				for (size_t i = 0; i < size; i++)
				{
					RefValueUnboxer<TElem>::apply(data[i], array[i]);
				}
			}
			else
			{
				JULIACPP_ASSERT(jl_array_eltype(val) == TypeTraits<TElem>::dataType(), "Unexpected jl_array_t element type.");
				std::memcpy(array.data(), jlArray->data, sizeof(TElem) * size);
			}
		}
	} // namespace RefUnboxer


	template<typename... TReturns>
	inline typename Unboxer::ValueUnboxer<TReturns...>::type unboxValue(jl_value_t* value)
	{
		return Unboxer::ValueUnboxer<TReturns...>::apply(value);
	}

	template<typename T>
	inline void unboxValueByRef(jl_value_t* val, std::tuple<T&>& returns)
	{
		RefUnboxer::RefValueUnboxer<T>::apply(val, std::get<0>(returns));
	}

	template<typename... TReturns>
	inline void unboxValueByRef(jl_value_t* val, std::tuple<TReturns&...>& returns)
	{
		RefUnboxer::RefValueUnboxer<TReturns...>::apply(val, returns);
	}
} // namespace Impl


inline void initJulia()
{
	jl_init(nullptr);
}

inline void initJulia(const std::string& homeDir)
{
	jl_init(homeDir.c_str());
}

inline void shutdownJulia(int status = 0)
{
	jl_atexit_hook(status);
}

struct IntermediateValue
{
	IntermediateValue() = default;
	IntermediateValue(jl_value_t* jlvalue) : _jlvalue(jlvalue) { };

	jl_value_t* _jlvalue;

	template <typename T>
	inline operator T()
	{
		return Impl::Unboxer::ValueUnboxer<T>::apply(_jlvalue);
	}

	inline jl_value_t* getJuliaValue() { return _jlvalue; }
};

template <typename T>
inline T unboxJuliaValue(jl_value_t* value)
{
	return Impl::unboxValue<T>(value);
}

inline IntermediateValue unboxJuliaValue(jl_value_t* value)
{
	return IntermediateValue(value);
}

template <typename... T>
class Tuple
{
public:
	Tuple(T&... args) : _tuple(args...) {}

	void operator=(IntermediateValue&& value)
	{
		_tuple = Impl::unboxValue<typename std::remove_reference<T>::type...>(value._jlvalue);
	}

private:
	std::tuple<T&...> _tuple;
};

template <typename... T>
Tuple<T&...> tie(T&... args)
{
	return Tuple<T&...>(args...);
}

template <typename... T>
struct TupleNoAlloc
{
public:
	TupleNoAlloc(T&... args) : _tuple(args...) {}

	void operator=(IntermediateValue&& value)
	{
		Impl::unboxValueByRef<typename std::remove_reference<T>::type...>(value._jlvalue, _tuple);
	}

private:
	std::tuple<T&...> _tuple;
};

template <typename... T>
TupleNoAlloc<T&...> tieNoAlloc(T&... args)
{
	return TupleNoAlloc<T&...>(args...);
}

template <typename T>
TupleNoAlloc<T&> noAlloc(T& arg)
{
	return TupleNoAlloc<T&>(arg);
}

class JuliaModule
{
public:

	JuliaModule(const std::string& filePath, const std::string& moduleName)
		: _filePath(filePath), _moduleName(moduleName)
	{
		reload();
	}

	JuliaModule(const std::string& filePath)
		: _filePath(filePath), _module(jl_current_module)
	{
		reload();
	}

	JuliaModule(jl_module_t* juliaModule)
		: _module(juliaModule)
	{
	}

	~JuliaModule() { }

	JuliaModule(const JuliaModule&) = default;
	JuliaModule(JuliaModule&&) = default;
	JuliaModule& operator=(const JuliaModule&) = default;
	JuliaModule& operator=(JuliaModule&&) = default;

	void reload()
	{
		if (_filePath.empty())
			return;

		JL_TRY
		{
			loadFile(_filePath);
			if (!_moduleName.empty())
			{
				_module = loadModule(_moduleName);
				JULIACPP_ASSERT(_module != nullptr, "Module '" + _moduleName + "' not found.");
			}
		}
		JL_CATCH
		{
			jl_static_show(JL_STDERR, jl_exception_occurred());
			jl_printf(JL_STDERR, "\n");
			JULIACPP_ASSERT(!jl_exception_occurred(), jl_typeof_str(jl_exception_occurred()));

			jl_exception_clear();
		}
	}

	template<typename... TArgs>
	IntermediateValue call(const std::string& functionName, TArgs&&... args)
	{
		return IntermediateValue { callInternal(functionName, std::forward<TArgs>(args)...) };
	}

	template<typename TReturn, typename... TArgs>
	TReturn call(const std::string& functionName, TArgs&&... args)
	{
		jl_value_t* ret = callInternal(functionName, std::forward<TArgs>(args)...);
		return Impl::unboxValue<TReturn>(ret);
	};

private:

	template<typename... TArgs>
	jl_value_t* callInternal(const std::string& functionName, TArgs&&... args)
	{
		jl_function_t* func = getFunction(functionName);
		JULIACPP_ASSERT(func != nullptr, "Function '" + functionName + "' not found.");

		_argumentList.clear();
		_argumentList.reserve(sizeof...(TArgs));

		pushToArgumentList(std::forward<TArgs>(args)...);

		jl_value_t* ret;
		if (!_argumentList.empty())
		{
			const int32_t nargs = (int32_t)_argumentList.size();
			ret = jl_call(func, _argumentList.data(), nargs);
		}
		else
		{
			ret = jl_call0(func);
		}

		if (jl_exception_occurred())
		{
			jl_static_show(JL_STDERR, jl_exception_occurred());
			jl_printf(JL_STDERR, "\n");

			JULIACPP_ASSERT(!jl_exception_occurred(), jl_typeof_str(jl_exception_occurred()));
		}

		return ret;
	}

	static inline void loadFile(const std::string& file)
	{
		jl_load(file.c_str(), file.size());
	}

	static inline jl_module_t* loadModule(const std::string& module)
	{
		return (jl_module_t*)jl_get_global(jl_current_module, jl_symbol(module.c_str()));
	}

	inline jl_function_t* getFunction(const std::string& functionName)
	{
		return jl_get_function(_module, functionName.c_str());
	}


	void pushToArgumentList() { }

	template<typename T>
	void pushToArgumentList(T&& value)
	{
		jl_value_t* val = Impl::box(value);
		JULIACPP_ASSERT_NOMSG(val != nullptr);
		_argumentList.push_back(val);
	};

	template<typename T, typename... TArgs>
	void pushToArgumentList(T&& value, TArgs&&...  values)
	{
		pushToArgumentList(std::forward<T>(value));
		pushToArgumentList(std::forward<TArgs>(values)...);
	};


private:
	std::string _filePath;
	std::string _moduleName;
	jl_module_t* _module;

	std::vector<jl_value_t*> _argumentList;

};

} // namespace jlcpp
