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

#define PyABI_INCLUDE

#include <string>
#include <sstream>
#include <iostream>

#include "PyABI.hpp"

#include "PyABI_body.cpp"

// Module method definitions
static PyObject* hello_world(PyObject* self, PyObject* args) {
  printf("Hello, world!\n");
  Py_RETURN_NONE;
}

static PyObject* hello(PyObject* self, PyObject* args) {
  const char* name;
  if (!PyArg_ParseTuple(args, "s", &name)) {
    return NULL;
  }

  printf("Hello, %s!\n", name);
  Py_RETURN_NONE;
}

static PyMethodDef cpu_methods[] = {
    {
        "hello_world", hello_world, METH_NOARGS,
        "Print 'hello world' from a method defined in a C extension."
    },
    {
        "hello", hello, METH_VARARGS,
        "Print 'hello xxx' from a method defined in a C extension."
    },
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef hello_definition = {
    PyModuleDef_HEAD_INIT,
    "PyABI_pyd",
    "PyABI C++",
    -1,
    cpu_methods
};

PyMODINIT_FUNC PyInit_PyABI_pyd(void) {
  Py_Initialize();
  return PyModule_Create(&hello_definition);
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

#include "PyABI_footer.cpp"
