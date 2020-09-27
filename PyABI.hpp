/***

License: MIT License

Author: Copyright (c) 2020-2020, Scott McCallum (github.com scott91e1)

***/

#pragma once

#include "depends/PyABI/PyABI_header.hpp"

class Singleton final {

public:

    Singleton() : NextID(1) {

    };

    ~Singleton() {

    };

    void AtomicResultsPush(ResultBundle& result) {
      Mutex.lock();
      try {
        Results.push(result);
      }
      catch (...) {

      }
      Mutex.unlock();
    }


    void hello_world(size_t CallID, List Args, Dictionay kwArgs, List DefaultArgs) {
        ResultBundle Result(CallID);
        
        /***

        do some work

        ***/

        AtomicResultsPush(Result);
    }
    size_t hello_world__(PyObject* args, PyObject* kwargs, PyObject* defaultargs) {
        List Args(args), DefaultArgs(defaultargs);
        Dictionay kwArgs(kwargs);
        size_t ID = NextID++;
        Threads.enqueue(std::bind(&Singleton::hello_world, this, ID, Args, kwArgs, DefaultArgs));
        return ID;
    };

    void hello(size_t CallID, List Args, Dictionay kwArgs, List DefaultArgs) {
        ResultBundle Result(CallID);

        /***

        do some work

        ***/

        AtomicResultsPush(Result);
    }
    size_t hello__(PyObject* args, PyObject* kwargs, PyObject* defaultargs) {
        List Args(args), DefaultArgs(defaultargs);
        Dictionay kwArgs(kwargs);
        size_t ID = NextID++;
        Threads.enqueue(std::bind(&Singleton::hello, this, ID, Args, kwArgs, DefaultArgs));
        return ID;
    };

private:

    std::mutex Mutex;

    std::atomic<size_t> NextID;

    std::queue<ResultBundle> Results;

    ThreadPool Threads{ PyABI_threads };

};

static Singleton SingletonInstance;

size_t hello_world__(PyObject* args, PyObject* kwargs, PyObject* defaultargs) {
    return SingletonInstance.hello_world__(args, kwargs, defaultargs);
};

size_t hello__(PyObject* args, PyObject* kwargs, PyObject* defaultargs) {
    return SingletonInstance.hello__(args, kwargs, defaultargs);
};


/***

//
//  MIT License
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this softwareand associated documentation files(the "Software"), to deal
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