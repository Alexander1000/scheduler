#ifndef H_SCHEDULER_BUCKET_INCLUDED
#define H_SCHEDULER_BUCKET_INCLUDED

#include <map>
#include <list>
#include <scheduler/item.h>

namespace Scheduler {
    // bucket
    class Bucket {
    public:
        Bucket(int id, std::map<int, int>* capacity);
        int GetID();
        bool HasCapacityForItem(Item* item);
        void AddItem(Item* item);
        std::list<Item*>* GetItems();
        std::map<int, int>* GetCapacity();
        std::map<int, int>* GetUsage();
        std::map<int, int>* GetLeft();
        float GetFillRate();
    private:
        // identifier of bucket
        int id;

        // relation: resource id -> capacity of resource
        std::map<int, int>* capacity;

        // cache, usage of resources (calculated parameters)
        std::map<int, int>* usage;

        // cache, left resources (calculated parameters)
        std::map<int, int>* left;

        // list items
        std::list<Item*>* items;
    };

}

#endif
