/***

License: MIT License

Author: Copyright (c) 2020-2020, Scott McCallum (github.com scott91e1)

***/

#pragma once

#include "src/PyABI/PyABI_header.hpp"

class Singleton final {

public:

    Singleton() : NextID(1) {

    };

    ~Singleton() {

    };

    void Return(Results& result) {
      Mutex.lock();
      try {
        Returns.push(result);
      }
      catch (...) {

      }
      Mutex.unlock();
    }


    void hello_world(const uint64_t CallID, const List& args, const Dict& kwargs, const Tuple& defargs) {
        Results Result(CallID);

        /***

        do some work on the background thread

        ***/

        Return(Result);
    }
    uint64_t hello_world__(const List& args, const Dict& kwargs = Dict(), const Tuple& defargs = Tuple()) {
        const uint64_t ID = NextID++;
        Threads.enqueue(std::bind(&Singleton::hello_world, this, ID, args, kwargs, defargs));
        return ID;
    };

    void hello(const uint64_t CallID, const List& Args, const Dict& kwArgs, const Tuple& defargs) {
        Results Result(CallID);

        const char* name = "scott";

        std::cout << "Hello, " << name << std::endl;

        /***

        do some work on the background thread

        ***/

        Return(Result);
    }
    uint64_t hello__(const List& args, const Dict& kwargs = Dict(), const Tuple& defargs = Tuple()) {
        const uint64_t ID = NextID++;
        Threads.enqueue(std::bind(&Singleton::hello, this, ID, args, kwargs, defargs));
        return ID;
    };

private:

    std::mutex Mutex;

    std::atomic<uint64_t> NextID;

    std::queue<Results> Returns;

    ThreadPool Threads{ PyABI_threads };

};

static Singleton SingletonInstance;

uint64_t hello_world__(PyObject* args, PyObject* kwargs, PyObject* defargs) {
    return SingletonInstance.hello_world__(args, kwargs, defargs);
};

uint64_t hello__(PyObject* args, PyObject* kwargs, PyObject* defargs) {
    return SingletonInstance.hello__(args, kwargs, defargs);
};


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
