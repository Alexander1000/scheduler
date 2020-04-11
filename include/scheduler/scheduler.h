#ifndef H_SCHEDULER_SCHEDULER_INCLUDED
#define H_SCHEDULER_SCHEDULER_INCLUDED

#include <list>
#include <scheduler/resources.h>
#include <scheduler/bucket.h>
#include <scheduler/item.h>

namespace Scheduler {
    // scheduler
    class Scheduler {
    private:
        // pool of resources
        std::list<Resource*> resource_pool;

        // pool of buckets
        std::list<Bucket*> bucket_pool;

        // list items for schedule
        std::list<Item*> pending_items;
    };
}

#endif
