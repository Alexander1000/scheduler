#ifndef H_SCHEDULER_RESOURCES_INCLUDED
#define H_SCHEDULER_RESOURCES_INCLUDED

namespace Scheduler {
    // resource
    class Resource {
    public:
        Resource(int id, char *name);
        Resource(int id, const char *name);
    private:
        // identifier of resource
        int id;

        // name of resource
        char *name;
    };
}

#endif
