#include <Python.h>

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

#include "depends\ttmath.h"
#include "PyABI_safeint.hpp"

//#include "PyABI_header_tinyjs.hpp"

typedef long double Decimal80BIT;
typedef std::u32string String_UTF32;
typedef ttmath::Int<256> Integer_Huge;
typedef ttmath::Big<8, 8> Decimal_Huge;
typedef SafeInt<long long int> Integer64BIT;

struct Unicode {

  String_UTF32 Value;

  Unicode(String_UTF32 value) : Value(value) {

  };

  Unicode(PyObject* object) {

  };

  PyObject* to_unicode() {
    PyObject* result = 0;
    Py_UCS4 maxchar = 0;
    for (String_UTF32::iterator it = Value.begin(); it != Value.end(); ++it) {
      if (*it > maxchar) {
        maxchar = *it;
      }
    }
    result = PyUnicode_New(Value.length() + 1, maxchar);
    for (String_UTF32::iterator it = Value.begin(); it != Value.end(); ++it) {
      //PyUnicode_WRITE()
    }
    return result;
  };

};

struct Dictionay {

  Dictionay() {

  };

  Dictionay(PyObject* object) {

  };

  PyObject* to_dict() {

  };

private:

  std::map<String_UTF32, String_UTF32> Strings;
  std::map<String_UTF32, Integer64BIT> Integer64s;
  std::map<String_UTF32, Decimal80BIT> Decimal80s;
  std::map<String_UTF32, Integer_Huge> HugeIntegers;
  std::map<String_UTF32, Decimal_Huge> HugeDecimals;

};

struct List {

  List() {

  };

  List(PyObject* object) {

  };

  PyObject* to_list() {

  };

private:

  std::map<size_t, String_UTF32> Strings;
  std::map<size_t, Integer64BIT> Integer64s;
  std::map<size_t, Decimal80BIT> Decimal80s;
  std::map<size_t, Integer_Huge> HugeInteger;
  std::map<size_t, Decimal_Huge> HugeDecimal;

};

struct Arguments {

  Arguments(PyObject* module, PyObject* args, PyObject* kwargs, PyObject* defaultargs) {


  };

  List args, defaultargs;
  Dictionay kwargs;

};

class ResultBundle {

public:

  ResultBundle(size_t call_id) : CallID(call_id), Success(false), ReturnTypeSet(false) {

  };

  size_t CallID;

  bool Success;

  List results;

  Dictionay kwresults;

  void Return(String_UTF32& value) {
    ReturnTypeSet = true;
    returna_String.push(value);
  }

  void Return(Integer64BIT& value) {
    ReturnTypeSet = true;
    returna_Integer64.push(value);
  }


  PyObject* result() {
    PyObject* ret = 0;
    if (returna_String.size()) {
      Unicode unicode(returna_String.top());
      return unicode.to_unicode();
    }
    else if (returna_Integer64.size()) {

    }
    else if (returna_Decimal80.size()) {

    }
    else if (returna_HugeInteger.size()) {

    }
    else if (returna_HugeDecimal.size()) {

    }
    if (ret == 0) {
      Py_INCREF(Py_None);
      return Py_None;
    }
    return ret;
  };

private:

  bool ReturnTypeSet;

  std::stack<String_UTF32> returna_String;
  std::stack<Integer64BIT> returna_Integer64;
  std::stack<Decimal80BIT> returna_Decimal80;
  std::stack<Integer_Huge> returna_HugeInteger;
  std::stack<Decimal_Huge> returna_HugeDecimal;

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
