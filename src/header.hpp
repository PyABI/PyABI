/***

License: MIT License

Author: Copyright (c) 2020-2020, Scott McCallum (github.com scott91e1)

Ethos: http://utf8everywhere.org

***/

#pragma once

#include <exception>

#include <map>
#include <queue>
#include <array>
#include <stack>
#include <vector>

#include <future>
#include <thread>
#include <cstddef>
#include <cassert>
#include <functional>
#include <condition_variable>

//#include "ttmath/ttmath.h"
//using Integer_Huge = ttmath::Int<256>;

#define INTEGER_ENABLE_LITERALS 1
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer>
using Integer_Huge = sw::unum::integer<1024>;

#define POSIT_ENABLE_LITERALS 1
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
using Decimal = sw::unum::posit<128, 2>;

//#include <universal/decimal/decimal.hpp>
//using Decimal = sw::unum::decimal;

//#include <universal/mpfloat/mpfloat.hpp>
//using Decimal = sw::unum::mpfloat;

//#include <universal/value/value.hpp>
//using Decimal = sw::unum::value<64>;


#include "safeint/safeint.hpp"
using Safe_I32 = SafeInt<int32_t>;
using Safe_I64 = SafeInt<int64_t>;

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/***



***/

struct StringHash {

	template <size_t N, size_t I>
	struct HashHelper
	{
		constexpr static size_t Calculate(const char(&str)[N])
		{
			return (HashHelper<N, I - 1>::Calculate(str) ^ (str[I - 1] & 0xFF)) * StringHash::PRIME;
		}
	};

	template <size_t N>
	struct HashHelper<N, 1>
	{
		constexpr static size_t Calculate(const char(&str)[N])
		{
			return (StringHash::OFFSET ^ (str[0] & 0xFF)) * StringHash::PRIME;
		}
	};

	template<size_t N>
	constexpr static size_t StaticHash(const char(&str)[N])
	{
		return HashHelper<N, N>::Calculate(str);
	}
	static const size_t OFFSET = 0x01234567;
	static const size_t PRIME = 0x89ABCDEF;
};

static size_t StringHash__Dynamic(const char* str) {
	size_t R = (StringHash::OFFSET ^ (*str & 0xFF)) * StringHash::PRIME;
	while (*str++) {
		R = (R ^ (*str & 0xFF)) * StringHash::PRIME;
	}
	return R;
}

static size_t IntegerHash__Dynamic(int64_t value) {
	value = abs(value);
	size_t R = (StringHash::OFFSET ^ (value & 0xFF)) * StringHash::PRIME;
	while (value) {
		value >>= 8;
		R = (R ^ (value & 0xFF)) * StringHash::PRIME;
	}
	return R;
}

/*
static size_t IntegerHash__Dynamic(Integer_Huge value) {
	value = value.Abs();
	size_t R = (StringHash::OFFSET ^ (value.BitAnd(0xff).ToInt())) * StringHash::PRIME;
	while (value > 0) {
		value >>= 8;
		R = (R ^ (value & 0xFF)) * StringHash::PRIME;
	}
	return R;
}
*/

/***

https://code.activestate.com/recipes/577985

An unique_ptr that, instead of deleting, decrements the reference count of a PyObject pointer.

Make sure to only use this when you get a *new* reference (Py_INCREF or getting the result of
any function that says it returns a new reference to a PyObject), NOT for "borrowed" references.

***/

using auto_pyptr_base = std::unique_ptr<PyObject>;
class auto_pyptr : public auto_pyptr_base {
public:
	auto_pyptr(PyObject* obj = nullptr) : auto_pyptr_base(obj) {
	}
	~auto_pyptr() {
		reset();
	}
	void reset(PyObject* obj = nullptr) {
		if (obj != get()) {
			PyObject* old = release(); // Avoid the delete call
			Py_XDECREF(old);
			auto_pyptr_base::reset(obj);
		}
	}
	void inc() {
		PyObject* ptr = get();
		if (ptr)
			Py_INCREF(ptr);
	}
	operator PyObject* () {
		return this->get();
	}
};

struct Dict;
struct List;
struct Tuple;

struct PyABI_Exception : public std::exception {

};

/***



***/

class Object {

public:

	inline PyObject* toPyObject() const {
		return m_object->toPyObject();
	};

	inline bool isNone() const {
		return m_object->TypeHash == StringHash::StaticHash("None");
	}

	inline bool isBool() const {
		return m_object->TypeHash == StringHash::StaticHash("Bool");
	}

	inline bool isString() const {
		return m_object->TypeHash == StringHash::StaticHash("String");
	}

	Object()
		: m_object(new Object_None()) {
		// create a None object by default
	};

	Object(PyObject* object) {
		if (object == Py_None) {
			m_object.reset(new Object_None);
		}
		else if (object == Py_True || object == Py_False) {
			m_object.reset(new Object_Bool(object == Py_True));
		}
		else if (PyLong_Check(object)) {
			int overflow = 0;
			long long value = PyLong_AsLongLongAndOverflow(object, &overflow);
			if (overflow == 0) {
				m_object.reset(new Object_Integer(value));
			}
			else {
				m_object.reset(new Object_Integer_Huge(object));
			}
		}
		else if (PyUnicode_Check(object)) {

		}
		else if (PyTuple_Check(object)) {

		}
		else if (PyList_Check(object)) {

		}
		else if (PyDict_Check(object)) {

		}
		else {

		};

	};

	Object(const Dict& value)
		: m_object(new Object_None()) {

	};

	Object(const List& value)
		: m_object(new Object_None()) {

	};

	Object(const Tuple& value)
		: m_object(new Object_None()) {

	};

	Object(const std::int64_t& value)
		: m_object(new Object_Integer(value)) {

	};

	Object(const std::string& value)
		: m_object(new Object_String(value)) {

	};

	bool operator==(const Object& other) const
	{
		return m_object->equals(other);
	}

	size_t hash() const {
		return m_object->hash();
	}

	struct HashFunction
	{
		size_t operator()(const Object& object) const
		{
			return object.hash();
		}
	};

private:

	class Object_ABC {

	public:

		const char* Type;

		const int32_t TypeHash;

		Object_ABC(const char* type, int32_t typeHash) noexcept
			: Type(type), TypeHash(typeHash) {

		}

		virtual ~Object_ABC() {

		}

		virtual bool equals(const Object& other) const noexcept { return false; }

		virtual int32_t hash() const = 0;

		virtual PyObject* toPyObject() = 0;

		virtual Safe_I32 toInt32() {
			return toInt64();
		};

		virtual Safe_I64 toInt64() {
			const Integer_Huge huge = toIntHuge();
			if (huge > Integer_Huge(std::numeric_limits<long long>::max()) || huge < Integer_Huge(std::numeric_limits<long long>::min())) {
				throw new PyABI_Exception;
			}
			return (int64_t)huge;
		};

		virtual Integer_Huge toIntHuge() {
			throw new PyABI_Exception;
		};

	};

	std::shared_ptr<Object_ABC> m_object;

	class Object_None : public Object_ABC {

	public:

		Object_None() noexcept
			: Object_ABC("None", StringHash::StaticHash("None")) {

		}

		virtual ~Object_None() {

		}

		int32_t hash() const override {
			return 0;
		}

		PyObject* toPyObject() override {
			Py_INCREF(Py_None);
			return Py_None;
		}

	};


	class Object_Bool : public Object_ABC {

	public:

		Object_Bool(bool value = false)
			: Object_ABC("Bool", StringHash::StaticHash("Bool"))
			, m_value(value) {

		}

		int32_t hash() const override {
			return m_value ? 1 : 0;
		}

		PyObject* toPyObject() override {
			if (m_value) {
				Py_INCREF(Py_True);
				return Py_True;
			} else {
				Py_INCREF(Py_False);
				return Py_False;
			}
		}

	private:

		bool m_value;

	};


	class Object_String : public Object_ABC {

	public:

		Object_String(const std::string& value)
			: Object_ABC("String", StringHash::StaticHash("String"))
			, m_value(value) {

		}

		int32_t hash() const override {
			return StringHash__Dynamic(m_value.c_str());
		}

		PyObject* toPyObject() override {
			Py_INCREF(Py_None);
			return Py_None;
		}

	private:

		std::string m_value;

	};


	class Object_Integer : public Object_ABC {

	public:

		Object_Integer(const long long& value = 0)
			: Object_ABC("Integer", StringHash::StaticHash("Integer")), m_value(value) {

			static_assert(sizeof(long long) == sizeof(std::int64_t));

		}

		int32_t hash() const override {
			return IntegerHash__Dynamic(m_value);
		}

		PyObject* toPyObject() override {
			return PyLong_FromLongLong(m_value);
		}

	private:

		long long m_value;

	};

	class Object_Integer_Huge : public Object_ABC {

	public:

		Object_Integer_Huge(const Integer_Huge& value = 0)
			: Object_ABC("Integer", StringHash::StaticHash("Integer"))
			, m_value(value) {

		}

		Object_Integer_Huge(PyObject* object)
			: Object_ABC("Integer", StringHash::StaticHash("Integer")) {

			if (PyLong_Check(object)) {
				int overflow = 0;
				long long value = PyLong_AsLongLongAndOverflow(object, &overflow);
				if (overflow == 0) {
					m_value = value;
					return;
				}
			}

			auto_pyptr __repr__ = PyObject_Repr(object);
			const char* repr_utf8 = PyUnicode_AsUTF8(__repr__);
			if (!repr_utf8) throw new PyABI_Exception;
			m_value.assign(repr_utf8);
		}

		int32_t hash() const override {
			// TODO: fixme
			return IntegerHash__Dynamic((int64_t)m_value);
		}

		PyObject* toPyObject() override {
			if (m_value > std::numeric_limits<long long>::max() || m_value < std::numeric_limits<long long>::min()) {
				std::string value = convert_to_decimal_string(m_value);
				return PyLong_FromString(value.c_str(), nullptr, 10);
			}
			else {
				return PyLong_FromLongLong((int64_t)m_value);
			}
		}

	private:

		Integer_Huge m_value;

	};

	/***

	this switch statement will fail to compile if any of the strings produce the same hash

	***/

	void SANITY_CHECK__all_names_hash_uniquely() {
		switch (int32_t value = 0) {
		case StringHash::StaticHash("None"):
		case StringHash::StaticHash("Bool"):
		case StringHash::StaticHash("List"):
		case StringHash::StaticHash("Dict"):
		case StringHash::StaticHash("Tuple"):
		case StringHash::StaticHash("Bytes"):
		case StringHash::StaticHash("String"):
		case StringHash::StaticHash("Integer"):
		case StringHash::StaticHash("Decimal"):
		case StringHash::StaticHash("Complex"):
		case StringHash::StaticHash("Unknown"):
		case StringHash::StaticHash("Results"):
			break;
		}
	}

public:


};


/***



***/

struct List {

	List() {

	};

	List(PyObject* object) {

	};

	PyObject* toPyList() {

	};

private:

	std::vector<Object> m_objects;

};

/***



***/

struct Tuple {

	Tuple() {

	};

	Tuple(PyObject* object) {

	};

	PyObject* toPyList() {

	};

private:

	std::vector<Object> m_objects;

};

/***



***/

struct Dict {

	Dict() {

	};

	Dict(PyObject* object) {

	};

	PyObject* toPyDict() {

	};

private:

	// std::map<std::string, Object> stringKeys;

	std::unordered_map<Object, Object, Object::HashFunction> m_objects;

};




class Results {

public:

	Results(const size_t call_id)
		: CallID(call_id),
		Success(false), ResultTypeSet(false) {

	};

	const size_t CallID;

	bool Success;

	Object Result;

	Dict kwResults;

	void Return(const std::string& value) {
		ResultTypeSet = true;
		Result = Object(value);
	}

	void Return(Safe_I64& value) {
		ResultTypeSet = true;
		Result = Object(value);
	}

	PyObject* result() {
		return Result.toPyObject();
	};

private:

	bool ResultTypeSet;

};



















/*

https://codereview.stackexchange.com/questions/229560/implementation-of-a-thread-pool-in-c

*/

class ThreadPool final
{
public:

	explicit ThreadPool(std::size_t nthreads = std::thread::hardware_concurrency()) :
		m_enabled(true),
		m_pool(nthreads)
	{
		run();
	}

	~ThreadPool()
	{
		stop();
	}

	ThreadPool(ThreadPool const&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	template<class TaskT>
	auto enqueue(TaskT task) -> std::future<decltype(task())>
	{
		using ReturnT = decltype(task());
		auto promise = std::make_shared<std::promise<ReturnT>>();
		auto result = promise->get_future();

		auto t = [p = std::move(promise), t = std::move(task)]() mutable { execute(*p, t); };

		{
			std::lock_guard<std::mutex> lock(m_mu);
			m_tasks.push(std::move(t));
		}

		m_cv.notify_one();

		return result;
	}

private:

	std::mutex m_mu;
	std::condition_variable m_cv;

	bool m_enabled;
	std::vector<std::thread> m_pool;
	std::queue<std::function<void()>> m_tasks;

	template<class ResultT, class TaskT>
	static void execute(std::promise<ResultT>& p, TaskT& task)
	{
		p.set_value(task()); // syntax doesn't work with void ResultT :(
	}

	template<class TaskT>
	static void execute(std::promise<void>& p, TaskT& task)
	{
		task();
		p.set_value();
	}

	void stop()
	{
		{
			std::lock_guard<std::mutex> lock(m_mu);
			m_enabled = false;
		}

		m_cv.notify_all();

		for (auto& t : m_pool)
			t.join();
	}

	void run()
	{
		auto f = [this]()
		{
			while (true)
			{
				std::unique_lock<std::mutex> lock{ m_mu };
				m_cv.wait(lock, [&]() { return !m_enabled || !m_tasks.empty(); });

				if (!m_enabled)
					break;

				assert(!m_tasks.empty());

				auto task = std::move(m_tasks.front());
				m_tasks.pop();

				lock.unlock();
				task();
			}
		};

		for (auto& t : m_pool)
			t = std::thread(f);
	}
};


#define PY_DEFAULT_ARGUMENT_INIT(name, value, ret) \
    PyObject *name = NULL; \
    static PyObject *default_##name = NULL; \
    if (! default_##name) { \
        default_##name = value; \
        if (! default_##name) { \
            PyErr_SetString(PyExc_RuntimeError, "Can not create default value for " #name); \
            return ret; \
        } \
    }

#define PY_DEFAULT_ARGUMENT_SET(name) if (! name) name = default_##name; \
    Py_INCREF(name)


/***

//
//  MIT License
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//

***/
