#ifndef H_SCHEDULER_BUCKET_INCLUDED
#define H_SCHEDULER_BUCKET_INCLUDED

#include <map>
#include <list>
#include <scheduler/item.h>

namespace Scheduler {
    // bucket
    class Bucket {
        Bucket(int id, std::map<int, int>* capacity);
    private:
        // identifier of bucket
        int id;

        // relation: resource id -> capacity of resource
        std::map<int, int>* capacity;

        // list items
        std::list<Item*>* items;
    };

}

#endif
