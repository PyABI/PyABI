/***

License: MIT License

Author: Copyright (c) 2020-2020, Scott McCallum (github.com scott91e1)

***/

#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <map>
#include <queue>
#include <array>
#include <stack>
#include <vector>
//#include <unique>

#include <future>
#include <thread>
#include <cstddef>
#include <cassert>
#include <functional>
#include <condition_variable>

#include "../ttmath/ttmath.h"

#include "../safeint/safeint.hpp"

typedef long double Decimal80BIT;
typedef std::u32string String;
typedef ttmath::Int<256> Integer_Huge;
typedef ttmath::Big<8, 8> Decimal_Huge;
typedef SafeInt<long long int> Integer64BIT;

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

struct Unicode {

  String Value;

  Unicode(String value) : Value(value) {

  };

  Unicode(PyObject* object) {

  };

  PyObject* to_unicode() {
    PyObject* result = 0;
    Py_UCS4 maxchar = 0;
    for (String::iterator it = Value.begin(); it != Value.end(); ++it) {
      if (*it > maxchar) {
        maxchar = *it;
      }
    }
    result = PyUnicode_New(Value.length() + 1, maxchar);
    for (String::iterator it = Value.begin(); it != Value.end(); ++it) {
      //PyUnicode_WRITE()
    }
    return result;
  };

};

class List;
class Tuple;
class Dictionay;

class Object {

  public:

		bool isNone() { 
      return m_Object->TypeHash == StringHash::StaticHash("None"); 
    }

  private:

    class Object_ABC {

      public:

        Object_ABC(const char *type, int32_t typeHash) 
          : Type(type), TypeHash(typeHash) {

        }

				const char* Type;

        const int32_t TypeHash;

        virtual PyObject* toPyObject() = 0;

		    virtual void toInteger(int32_t&) {

		    };

		    virtual void toInteger(int64_t&) {

		    };

    };

		std::unique_ptr<Object_ABC> m_Object;

	  class Object_None : public Object_ABC {

	    public:

        Object_None() : Object_ABC("None", StringHash::StaticHash("None")) {

        }

		    PyObject* toPyObject() override {
          Py_INCREF(Py_None);
			    return Py_None;
		    }

	  };

    class Object_Integer : public Object_ABC {

      public:

				Object_Integer(const int64_t& value = 0)
					: Object_ABC("Integer", StringHash::StaticHash("Integer")), m_Value(value) {

				}

        PyObject* toPyObject() override {
          return PyLong_FromLongLong(m_Value);
        }
  
    private:

      int64_t m_Value;

    };


		/***

    sanity check:

    this switch statement will fail to compile if any of the strings produce the same hash

		***/

    void __sanity_check__all_hashes_are_unique__() {
      switch (int32_t value = 0) {
			  case StringHash::StaticHash("None"):
			  case StringHash::StaticHash("List"):
			  case StringHash::StaticHash("Dict"):
				case StringHash::StaticHash("Tuple"):
				case StringHash::StaticHash("String"):
			  case StringHash::StaticHash("Decimal"):
				case StringHash::StaticHash("Integer"):
				case StringHash::StaticHash("PyObject"):
      }
    }

  public:

	  // By default its a Python None object
	  Object()
		  : m_Object(new Object_None()) {

	  };

		Object(const List& value)
			: m_Object(new Object_None()) {

		};

		Object(const Tuple& value)
			: m_Object(new Object_None()) {

		};

		Object(const Dictionay& value)
			: m_Object(new Object_None()) {

		};

	  Object(const int32_t &value)
		  : m_Object(new Object_Integer(value)) {

	  };

    Object(const int64_t &value)
		  : m_Object(new Object_Integer(value)) {

	  };

	  PyObject* toPyObject() {
		  return m_Object->toPyObject();
	  };


  };


struct Dict {

  Dict() {

  };

  Dict(PyObject* object) {

  };

  PyObject* toPyDict() {

  };

private:

  std::map<String, Object> Objects;

};

struct List {

  List() {

  };

  List(PyObject* object) {

  };

  PyObject* toPyList() {

  };

private:

  std::map<size_t, Object> Objects;

};


struct Tuple {

	Tuple() {

	};

	Tuple(PyObject* object) {

	};

	PyObject* toPyList() {

	};

private:

	std::map<size_t, Object> Objects;

};


class ResultBundle {

public:

  ResultBundle(const size_t call_id) 
    : CallID(call_id), 
    Success(false), ResultTypeSet(false) {

  };

  const size_t CallID;

  bool Success;

	Object Result;

	List Results;

  Dict kwResults;

  void Return(String& value) {
		ResultTypeSet = true;
    Result = Object(value.c_str());
  }

  void Return(Integer64BIT& value) {
		ResultTypeSet = true;
    Result = value;
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
