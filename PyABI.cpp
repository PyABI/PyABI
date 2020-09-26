/***

Python PyABI C++ Sepcification

***/

/***

//
//  MIT License
//
//  Copyright(c) 2020 - 2020, Scott McCallum (https github.com scott91e1)
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this softwareand associated documentation files(the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions :
//
//  The above copyright noticeand this permission notice shall be included in all
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

/***

instruct the compiler to actually complile the source

***/

#define PyABI_INCLUDE 1

#include <string>
#include <sstream>
#include <iostream>

/*

by default we are going to have a small number of workers

its also possible to match the number of cores on the host with:

auto PyABI_threads = std::thread::hardware_concurrency();

*/

auto PyABI_threads = 4;

#include "PyABI.hpp"

#include "PyABI_singleton.hpp"

//static Singleton::Instance = Singleton();

#include "PyABI_body.hpp"

// Module method definitions
static PyObject* hello_world(PyObject* module, PyObject* args, PyObject* kwargs) {

  PY_DEFAULT_ARGUMENT_INIT(encoding, PyUnicode_FromString("utf-8"), NULL);
  PY_DEFAULT_ARGUMENT_INIT(the_id, PyLong_FromLong(0L), NULL);
  PY_DEFAULT_ARGUMENT_INIT(must_log, PyBool_FromLong(1L), NULL);

  static const char* kwlist[] = { "encoding", "the_id", "must_log", NULL };
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Oip", const_cast<char**>(kwlist), &encoding, &the_id, &must_log)) {
    return NULL;
  }

  PY_DEFAULT_ARGUMENT_SET(encoding);
  PY_DEFAULT_ARGUMENT_SET(the_id);
  PY_DEFAULT_ARGUMENT_SET(must_log);

  PyObject* defaultargs = PyList_New(3);
  PyList_SetItem(defaultargs, 0, encoding);
  PyList_SetItem(defaultargs, 1, the_id);
  PyList_SetItem(defaultargs, 2, must_log);
  auto dispatch_success = false;
  try {
    hello_world__(args, kwargs, defaultargs);
    //auto callback = std::bind(&Singleton::hello, singleton, std::placeholders::_1);
    //singleton.PyABI_dispatch(module, args, kwargs, defaultargs, "hello_world", callback);
    dispatch_success = true;
  }
  catch (...) {

  };
  Py_DECREF(defaultargs);

  Py_DECREF(encoding);
  Py_DECREF(the_id);
  Py_DECREF(must_log);

  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject* hello(PyObject* module, PyObject* args, PyObject* kwargs) {
  const char* name;
  if (!PyArg_ParseTuple(args, "s", &name)) {
    return NULL;
  }

  PyObject* defaultargs = PyList_New(0);
  auto dispatch_success = false;
  try {
    //auto callback = std::bind(&Singleton::hello, singleton, std::placeholders::_1);
    //singleton.PyABI_dispatch(module, args, kwargs, defaultargs, "hello", callback);
    dispatch_success = true;
  }
  catch (...) {

  };
  Py_DECREF(defaultargs);

  printf("Hello, %s!\n", name);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef abi_methods[] = {
    {
        "hello_world", (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS,
        "Print 'hello world' from a method defined in a C extension."
    },
    {
        "hello", (PyCFunction)hello, METH_VARARGS | METH_KEYWORDS,
        "Print 'hello xxx' from a method defined in a C extension."
    },
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef abi_definition = {
    PyModuleDef_HEAD_INIT,
    "PyABI_pyd",
    "PyABI C++",
    -1,
    abi_methods
};

PyMODINIT_FUNC PyInit_PyABI_pyd(void) {
  Py_Initialize();
  return PyModule_Create(&abi_definition);
}

// Create some work to test the Thread Pool
void spitId()
{
  std::cout << "thread #" << std::this_thread::get_id() << '\n';
}

void sayAndNoReturn()
{
  auto tid = std::this_thread::get_id();
  std::cout << "thread #" << tid << "says " << " and returns... ";
  std::cout << typeid(tid).name() << '\n';    // std::thread::id
}

char sayWhat(int arg)
{
  auto tid = std::this_thread::get_id();
  std::stringstream sid;
  sid << tid;
  int id = std::stoi(sid.str());
  std::cout << "\nthread #" << id << " says " << arg << " and returns... ";
  if (id > 7000)
    return 'X';
  return 'a';
}

struct Member
{
  int i_ = 4;
  void sayCheese(int i)
  {
    std::cout << "CHEESEE!" << '\n';
    std::cout << i + i_ << '\n';
  }
};

int vv() { puts("nothing"); return 0; }
int vs(const std::string& str) { puts(str.c_str()); return 0; }


int main(int argc, char** argv) {

  ThreadPool threadPool{ std::thread::hardware_concurrency() };

  threadPool.enqueue(spitId);
  threadPool.enqueue(&spitId);
  threadPool.enqueue(sayAndNoReturn);

  auto f1 = threadPool.enqueue([]() -> int
    {
      std::cout << "lambda 1\n";
      return 1;
    });

  //auto sayWhatRet = threadPool.enqueue(sayWhat, 100);
  //std::cout << sayWhatRet.get() << '\n';

  Member member{ 1 };
  threadPool.enqueue(std::bind(&Member::sayCheese, member, 100));

  std::cout << f1.get() << '\n';

  auto f2 = threadPool.enqueue([]()
    {
      std::cout << "lambda 2\n";
      return 2;
    });
  auto f3 = threadPool.enqueue([]()
    {
      return sayWhat(100);
    });

  //threadPool.enqueue(std::function<void(int)>{Member{}.sayCheese(100)});

  std::cout << "f1 type = " << typeid(f2).name() << '\n';

  std::cout << f2.get() << '\n';
  std::cout << f3.get() << '\n';

  return EXIT_SUCCESS;

}

#include "PyABI_footer.hpp"
