#ifndef OS2CX_GUI_PROJECT_RUNNER_HPP_
#define OS2CX_GUI_PROJECT_RUNNER_HPP_

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>

#include "project.hpp"

namespace os2cx {

/* GuiProjectRunner interfaces the blocking project_run() function with the
Qt event loop, by setting up a side thread in which to run. */

class GuiProjectRunnerWorkerThread :
    public QThread, private ProjectRunCallbacks
{
    Q_OBJECT
public:
    GuiProjectRunnerWorkerThread(
        QObject *parent,
        const Project &original_project);

    void run();

    /* project_on_worker_thread is normally only accessed from the worker
    thread, but may be accessed from the application thread during
    project_run_checkpoint(). */
    std::unique_ptr<Project> project_on_worker_thread;

    /* mutex controls access to project_on_worker_thread and checkpoint_active.
    The application thread uses wait_condition and checkpoint_active to notify
    the worker thread when it's OK to continue. checkpoint_active is used to
    avoid spurious wakeups. */
    QMutex mutex;
    QWaitCondition wait_condition;
    bool checkpoint_active;

signals:
    void checkpoint_signal();

private:
    void project_run_checkpoint();
};

class GuiProjectRunner : public QObject
{
    Q_OBJECT
public:
    GuiProjectRunner(QObject *parent, const std::string &scad_path);

    const Project *get_project() const {
        return project_on_application_thread.get();
    }

signals:
    void project_updated();

public slots:

private:
    /* project_on_application_thread is only ever accessed from the application
    thread. */
    std::unique_ptr<Project> project_on_application_thread;

    std::unique_ptr<GuiProjectRunnerWorkerThread> worker_thread;

private slots:
    void checkpoint_slot();
};

} /* namespace os2cx */

#endif
