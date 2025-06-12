#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include "Semaphore.h"

using namespace std;

typedef struct worker {
    int id;
    thread ts;
    function<void(void)> task;
    Semaphore* sem;
    mutex lock;
    bool available;
} worker_t;

class ThreadPool {
  public:
    ThreadPool(size_t numThreads);
    void schedule(const function<void(void)>& thunk);
    void wait();
    ~ThreadPool();

  private:
    void worker(int id);
    void dispatcher();

    thread dt;
    vector<worker_t> wts;
    bool done;
    mutex queueLock;

    queue<function<void(void)>> taskQueue;
    Semaphore taskSem;
    Semaphore workerAvailableSem;
    Semaphore allTasksDone;
    atomic<int> remainingTasks;

    ThreadPool(const ThreadPool& original) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
};

#endif
