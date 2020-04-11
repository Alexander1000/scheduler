#ifndef H_SCHEDULER_ITEM_INCLUDED
#define H_SCHEDULER_ITEM_INCLUDED

#include <map>

namespace Scheduler {
    // item
    class Item {
    public:
        Item(int id, std::map<int, int>* resources);
        std::map<int, int>* GetResources();
    private:
        // identifier of item
        int id;

        // resources for item
        std::map<int, int>* resources;

        // identifier of bucket
        int* bucketId;
    };
}

#endif
