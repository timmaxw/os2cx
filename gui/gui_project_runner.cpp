#include "gui_project_runner.hpp"

namespace os2cx {

GuiProjectRunnerWorkerThread::GuiProjectRunnerWorkerThread(
    QObject *parent,
    const Project &original_project) :
    QThread(parent)
{
    project_on_worker_thread.reset(new Project(original_project));
    checkpoint_active = false;
}

void GuiProjectRunnerWorkerThread::run() {
    mutex.lock();
    project_run(project_on_worker_thread.get(), this);
    mutex.unlock();
}

/* project_run() calls project_run_checkpoint() on the worker thread */
void GuiProjectRunnerWorkerThread::project_run_checkpoint() {
    checkpoint_active = true;
    emit checkpoint_signal();

    /* The QWaitCondition documentation doesn't explicitly state whether or not
    QWaitCondition is susceptible to spurious wakeups; better safe than sorry.
    */
    while (checkpoint_active) {
        wait_condition.wait(&mutex);
    }
    assert(!checkpoint_active);
}

GuiProjectRunner::GuiProjectRunner(
        QObject *parent,
        const std::string &scad_path) :
    QObject(parent),
    project_on_application_thread(new Project(scad_path, "os2cx_temp"))
{
    worker_thread.reset(new GuiProjectRunnerWorkerThread(
        this,
        *project_on_application_thread
    ));
    connect(
        worker_thread.get(), &GuiProjectRunnerWorkerThread::checkpoint_signal,
        this, &GuiProjectRunner::checkpoint_slot);
    worker_thread->start();
}

void GuiProjectRunner::checkpoint_slot() {
    {
        QMutexLocker mutex_locker(&worker_thread->mutex);
        *project_on_application_thread =
            *worker_thread->project_on_worker_thread;
        worker_thread->checkpoint_active = false;
        worker_thread->wait_condition.wakeAll();
    }
    emit project_updated();
}

} /* namespace os2cx */
