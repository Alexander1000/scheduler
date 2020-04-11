#include <scheduler.h>

namespace Scheduler
{
    Resource::Resource(int id, char *name) {
        this->id = id;
        this->name = name;
    }

    Resource::Resource(int id, const char *name) {
        this->id = id;
        this->name = (char*) name;
    }
}
