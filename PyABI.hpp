#ifdef PyABI_INCLUDE

#include <Python.h>

#include "PyABI_header.hpp"

#include <map>
#include <array>
#include <stack>

#include "safeint.hpp"

#include "ttmath.h"

typedef std::u32string String_UTF32;
typedef long double Decimal80BIT;
typedef ttmath::Big<8,8> Decimal_Huge;
typedef ttmath::Int<256> Integer_Huge;
typedef SafeInt<long long int> Integer64BIT;

struct Arguments {

	Arguments(PyObject* module, PyObject* args, PyObject* kwargs, PyObject* defaultargs) {


	};

	std::map<String_UTF32, String_UTF32> kwargs_Strings;
	std::map<String_UTF32, Integer64BIT> kwargs_Integer64s;
	std::map<String_UTF32, Decimal80BIT> kwargs_Decimal80s;
	std::map<String_UTF32, Integer_Huge> kwargs_HugeIntegers;
	std::map<String_UTF32, Decimal_Huge> kwargs_HugeDecimals;

};

struct Results {

	std::stack<String_UTF32> returna_String;
	std::stack<Integer64BIT> returna_Integer64;
	std::stack<Decimal80BIT> returna_Decimal80;
	std::stack<Integer_Huge> returna_HugeInteger;
	std::stack<Decimal_Huge> returna_HugeDecimal;

	std::map<size_t, String_UTF32> results_Strings;
	std::map<size_t, Integer64BIT> results_Integer64s;
	std::map<size_t, Decimal80BIT> results_Decimal80s;
	std::map<size_t, Integer_Huge> results_HugeInteger;
	std::map<size_t, Decimal_Huge> results_HugeDecimal;

	std::map<std::u32string, String_UTF32> kwresults_Strings;
	std::map<std::u32string, Integer64BIT> kwresukts_Integer64s;
	std::map<std::u32string, Decimal80BIT> kwresukts_Decimal80s;
	std::map<std::u32string, Decimal_Huge> kwresukts_HugeDecimals;
	std::map<std::u32string, Integer_Huge> kwresukts_HugeIntegers;

};

class Singleton final {

public:

	Singleton() {

	};

	~Singleton() {

	};

	static void hello_world() {
		object.object_hello_world();
	};
	void object_hello_world() {

	};

	static bool hello() {
		object.object_hello();
	};
	void object_hello() {

	};

private:

	static Singleton object;

	std::mutex PyABI_mutex;

	ThreadPool PyABI_pool{ PyABI_threads };

};

#endif

