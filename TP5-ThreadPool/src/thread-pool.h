// thread-pool.h
#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <unordered_set>
#include <stdexcept>
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

    // dispatcher thread
    thread dt;
    // workers
    vector<worker_t> wts;
    // shutdown flag (ahora atómico para evitar data races)
    atomic<bool> done;

    // task queue + semaphore
    mutex queueLock;
    queue<function<void(void)>> taskQueue;
    Semaphore taskSem;
    Semaphore workerAvailableSem;

    // remaining tasks counter
    atomic<int> remainingTasks;

    // para wait(): mutex + condition_variable
    mutex doneMutex;
    condition_variable doneCV;

    // registro de instancias activas para detectar uso tras destrucción
    static mutex registryMutex;
    static unordered_set<ThreadPool*> activePools;

    ThreadPool(const ThreadPool& original) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
};

#endif
