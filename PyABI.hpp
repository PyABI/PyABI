#include "PyABI_header.hpp"

class Singleton final {

public:

	Singleton() : NextID(0) {

	};

	~Singleton() {

	};

	void hello_world(size_t CallID, List Args, Dictionay kwArgs, List DefaultArgs) {
		ResultBundle Result(CallID);
		Mutex.lock();
		try {
			Results.push(Result);
		}
		catch (...){

		}
		Mutex.unlock();

	}
	size_t hello_world__(PyObject* args, PyObject* kwargs, PyObject* defaultargs) {
		List Args(args), DefaultArgs(defaultargs);
		Dictionay kwArgs(kwargs);
		size_t CallID = ++NextID;
		Threads.enqueue(std::bind(&Singleton::hello_world, this, CallID, Args, kwArgs, DefaultArgs));
		return CallID;
	};

	void hello(size_t CallID, List Args, Dictionay kwArgs, List DefaultArgs) {


	}
	size_t hello__(PyObject* args, PyObject* kwargs, PyObject* defaultargs) {
		List Args(args), DefaultArgs(defaultargs);
		Dictionay kwArgs(kwargs);
		size_t CallID = ++NextID;
		Threads.enqueue(std::bind(&Singleton::hello, this, CallID, Args, kwArgs, DefaultArgs));
		return CallID;
	};

private:

	size_t NextID;

	std::mutex Mutex;

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
