#include "thread-pool.h"
#include <queue>

using namespace std;

ThreadPool::ThreadPool(size_t numThreads)
    : wts(numThreads),
      done(false),
      taskSem(0),
      workerAvailableSem(numThreads),
      allTasksDone(0),
      remainingTasks(0)
{
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].id = i;
        wts[i].available = true;
        wts[i].sem = new Semaphore(0);
        wts[i].ts = thread([this, i]() { worker(i); });
    }

    dt = thread([this]() { dispatcher(); });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    {
        lock_guard<mutex> lock(queueLock);
        taskQueue.push(thunk);
        remainingTasks++;
    }
    taskSem.signal();
}

void ThreadPool::dispatcher() {
    while (true) {
        taskSem.wait();

        if (done && taskQueue.empty())
            break;

        function<void(void)> task;
        {
            lock_guard<mutex> lock(queueLock);
            if (taskQueue.empty()) continue;
            task = taskQueue.front();
            taskQueue.pop();
        }

        workerAvailableSem.wait();

        for (auto& w : wts) {
            lock_guard<mutex> lock(w.lock);
            if (w.available) {
                w.available = false;
                w.task = task;
                w.sem->signal();
                break;
            }
        }
    }
}

void ThreadPool::worker(int id) {
    worker_t& w = wts[id];

    while (true) {
        w.sem->wait();

        if (done && w.task == nullptr)
            break;

        function<void(void)> task;
        {
            lock_guard<mutex> lock(w.lock);
            task = w.task;
        }

        if (task) task();

        {
            lock_guard<mutex> lock(w.lock);
            w.available = true;
            w.task = nullptr;
        }

        workerAvailableSem.signal();

        if (--remainingTasks == 0)
            allTasksDone.signal();
    }
}

void ThreadPool::wait() {
    while (remainingTasks > 0)
        allTasksDone.wait();
}

ThreadPool::~ThreadPool() {
    wait();
    done = true;

    taskSem.signal(); // Wake up dispatcher
    dt.join();

    // Despertar a todos los workers
    for (auto& w : wts) {
        w.task = nullptr; // ← clave para que worker sepa que no hay más tareas
        w.sem->signal();
    }

    for (auto& w : wts) {
        if (w.ts.joinable()) w.ts.join();
        delete w.sem;
    }
}
