// thread-pool.cc
#include "thread-pool.h"

using namespace std;

// definición de miembros estáticos
mutex ThreadPool::registryMutex;
unordered_set<ThreadPool*> ThreadPool::activePools;

ThreadPool::ThreadPool(size_t numThreads)
  : wts(numThreads),
    done(false),
    taskSem(0),
    workerAvailableSem(numThreads),
    remainingTasks(0)
{
    // registrar instancia activa
    {
        lock_guard<mutex> reg(registryMutex);
        activePools.insert(this);
    }

    // arrancar hilos workers
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].id = static_cast<int>(i);
        wts[i].available = true;
        wts[i].task = nullptr;
        wts[i].sem = new Semaphore(0);
        wts[i].ts = thread(&ThreadPool::worker, this, static_cast<int>(i));
    }
    // arrancar dispatcher
    dt = thread(&ThreadPool::dispatcher, this);
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    if (!thunk)
        throw invalid_argument("ThreadPool::schedule: tarea nula no permitida");

    {
        lock_guard<mutex> reg(registryMutex);
        if (activePools.find(this) == activePools.end())
            throw runtime_error("ThreadPool destruido: schedule() no permitido");
    }

    // encolar tarea
    {
        lock_guard<mutex> lk(queueLock);
        taskQueue.push(thunk);
        remainingTasks.fetch_add(1);
    }
    taskSem.signal();
}

void ThreadPool::dispatcher() {
    while (true) {
        taskSem.wait();
        if (done.load() && taskQueue.empty())
            break;

        function<void(void)> task;
        {
            lock_guard<mutex> lk(queueLock);
            if (taskQueue.empty())
                continue;
            task = taskQueue.front();
            taskQueue.pop();
        }

        workerAvailableSem.wait();

        for (auto& w : wts) {
            lock_guard<mutex> lk(w.lock);
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
        if (done.load() && w.task == nullptr)
            break;

        function<void(void)> fn;
        {
            lock_guard<mutex> lk(w.lock);
            fn = w.task;
        }
        if (fn) fn();

        {
            lock_guard<mutex> lk(w.lock);
            w.available = true;
            w.task = nullptr;
        }
        workerAvailableSem.signal();

        // si era la última tarea pendiente, notificar a todos los waiters
        if (remainingTasks.fetch_sub(1) == 1) {
            lock_guard<mutex> lk(doneMutex);
            doneCV.notify_all();
        }
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lk(doneMutex);
    doneCV.wait(lk, [&]() { return remainingTasks.load() == 0; });
}

ThreadPool::~ThreadPool() {
    // retirar del registro de instancias
    {
        lock_guard<mutex> reg(registryMutex);
        activePools.erase(this);
    }

    // esperar a que todas las tareas terminen
    wait();
    done.store(true);               // store() en lugar de asignación directa

    // despertar dispatcher
    taskSem.signal();
    if (dt.joinable())
        dt.join();

    // despertar a todos los workers para que salgan, protegiendo w.task
    for (auto& w : wts) {
        {
            lock_guard<mutex> lk(w.lock);
            w.task = nullptr;
        }
        w.sem->signal();
    }
    // unirse y liberar semáforos
    for (auto& w : wts) {
        if (w.ts.joinable())
            w.ts.join();
        delete w.sem;
    }
}
