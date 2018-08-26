#ifndef OS2CX_PROJECT_RUN_HPP_
#define OS2CX_PROJECT_RUN_HPP_

#include "project.hpp"

namespace os2cx {

/* project_run() performs all of the computations for a project. It gets run in
a separate worker thread separate from the main application thread. Each thread
has its own copy of the Project; project_run() periodically calls callbacks->
project_run_checkpoint(), which synchronizes with the main application thread to
copy the latest project state over from the worker thread. The copying process
is inexpensive because all the complex data structures on the Project are stored
as shared_ptr<const Whatever>. project_run_checkpoint() can also throw
ProjectInterruptedException to cancel the computation. */

class ProjectRunCallbacks {
public:
    virtual void project_run_log(const std::string &) { }
    virtual void project_run_checkpoint() { }
};

class ProjectInterruptedException : std::exception {
    const char *what() const throw () {
        return "project interrupted";
    }
};

void project_run(Project *project, ProjectRunCallbacks *callbacks);

} /* namespace os2cx */

#endif
