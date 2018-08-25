#ifndef OS2CX_GUI_PROJECT_RUNNER_HPP_
#define OS2CX_GUI_PROJECT_RUNNER_HPP_

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QWaitCondition>

#include "project_run.hpp"

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

    /* These variables are normally only accessed from the worker thread, but
    may be accessed from the application thread during
    project_run_checkpoint(). */
    std::unique_ptr<Project> project_on_worker_thread;
    bool should_interrupt;

    /* mutex controls access to the above variables and checkpoint_active.
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

    std::shared_ptr<const Project> get_project() const {
        return std::shared_ptr<const Project>(project_on_application_thread);
    }

    enum class Status {Running, Done, Interrupting, Interrupted};
    Status status() const;

    /* Cancels running the project. Cancelling takes some time, so it happens
    asynchronously. interrupt() returns immediately, and then some time later
    we emit project_updated(true). */
    void interrupt();

signals:
    void project_updated();
    void status_changed(Status new_status);

public slots:

private:
    /* project_on_application_thread is only ever accessed from the application
    thread. */
    std::shared_ptr<Project> project_on_application_thread;

    std::unique_ptr<GuiProjectRunnerWorkerThread> worker_thread;

    bool interrupted;

    Status last_emitted_status;
    void maybe_emit_status_changed();

private slots:
    void checkpoint_slot();
};

} /* namespace os2cx */

#endif
