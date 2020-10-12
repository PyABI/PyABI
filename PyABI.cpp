/***

License: MIT License

Author: Copyright (c) 2020-2020, Scott McCallum (github.com scott91e1)

***/

#include <string>
#include <sstream>
#include <iostream>

/***

by default we are going to have a small number of workers

its also possible to match the number of cores on the host with:

auto PyABI_threads = std::thread::hardware_concurrency();

***/

auto PyABI_threads = 4;

#include "PyABI.hpp"

#include "src/singleton.hpp"

#include "src/body.hpp"

// Module method definitions
static PyObject* hello_world(PyObject* module, PyObject* args, PyObject* kwargs) {

  PY_DEFAULT_ARGUMENT_INIT(encoding, PyUnicode_FromString("utf-8"), nullptr);
  PY_DEFAULT_ARGUMENT_INIT(the_id, PyLong_FromLong(0L), nullptr);
  PY_DEFAULT_ARGUMENT_INIT(must_log, PyBool_FromLong(1L), nullptr);

  static const char* kwlist[] = { "encoding", "the_id", "must_log", nullptr };
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|Oip", const_cast<char**>(kwlist), &encoding, &the_id, &must_log)) {
    return nullptr;
  }

  PY_DEFAULT_ARGUMENT_SET(encoding);
  PY_DEFAULT_ARGUMENT_SET(the_id);
  PY_DEFAULT_ARGUMENT_SET(must_log);

  PyObject* defargs = PyList_New(3);
  PyList_SetItem(defargs, 0, encoding);
  PyList_SetItem(defargs, 1, the_id);
  PyList_SetItem(defargs, 2, must_log);
	uint64_t call_id = 0;
  try {
    call_id = hello_world__(args, kwargs, defargs);
  }
  catch (...) {

  };
  Py_DECREF(defargs);

  Py_DECREF(encoding);
  Py_DECREF(the_id);
  Py_DECREF(must_log);

  return PyLong_FromUnsignedLong(call_id);
}


static PyObject* hello(PyObject* module, PyObject* args, PyObject* kwargs) {
  const char* name;
  if (!PyArg_ParseTuple(args, "s", &name)) {
    return nullptr;
  }


  PyObject* defargs = PyList_New(0);
	uint64_t call_id = 0;
  try {
    call_id = hello__(args, kwargs, defargs);
  }
  catch (...) {

  };
  Py_DECREF(defargs);

  Py_INCREF(Py_None);
  return Py_None;
}

//static PyObject* PyABI_main(PyObject* module, PyObject* args, PyObject* kwargs);
//static PyObject* PyABI_stop(PyObject* module, PyObject* args, PyObject* kwargs);

static PyMethodDef abi_methods[] = {
    {
        "hello_world", (PyCFunction)hello_world, METH_VARARGS | METH_KEYWORDS,
        "Print 'hello world' from a method defined in a C extension."
    },
    {
        "hello", (PyCFunction)hello, METH_VARARGS | METH_KEYWORDS,
        "Print 'hello xxx' from a method defined in a C extension."
    },
    {nullptr, nullptr, 0, nullptr}
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

static void
urkel_usage(void) {
  std::cout <<
    "\n"
    "  Usage: urkel [options] [action] [args]\n"
    "\n"
    "  Actions:\n"
    "\n"
    "    create                create a new database\n"
    "    destroy               destroy database\n"
    "    info                  print database information\n"
    "    root                  print root hash\n"
    "    get <key>             retrieve value\n"
    "    insert <key> <value>  insert value\n"
    "    remove <key>          remove value\n"
    "    list                  list all keys\n"
    "    prove <key>           create proof\n"
    "    verify <key> <proof>  verify proof (requires --root)\n"
    "\n"
    "  Options:\n"
    "\n"
    "    -p, --path <path>     path to database (default: $URKEL_PATH)\n"
    "    -r, --root <hash>     root hash to use for snapshots\n"
    "    -H, --hash            hash key with BLAKE2b-256\n"
    "    -h, --help            output usage information\n"
    "\n"
    "  Environment Variables:\n"
    "\n"
    "    URKEL_PATH            path to database (default: ./)\n"
    "\n"
    "  Example:\n"
    "\n"
    "    $ urkel create -p db\n"
    "    $ cd db\n"
    "    $ urkel insert foo 'deadbeef' -H\n"
    "    $ urkel get foo -H\n"
    "    $ urkel prove foo -H\n"
    "    $ urkel verify foo $(urkel prove foo -H) -r $(urkel root) -H\n"
    "    $ urkel destroy\n"
    "\n";

}

#include "src/argparse/argparse.hpp"

int main(int argc, char** argv) {

  ThreadPool threadPool{ std::thread::hardware_concurrency() };

  argparse::ArgumentParser program("test");

  program.add_argument("--verbose")
    .help("increase output verbosity")
    .default_value(false)
    .implicit_value(true);

  try {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }

  if (program["--verbose"] == true) {
    std::cout << "Verbosity enabled" << std::endl;
  }

  /***

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

  ***/

  return EXIT_SUCCESS;

}

#include "src/footer.hpp"

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
