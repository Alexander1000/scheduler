#include <scheduler.h>

namespace Scheduler
{
    Item::Item(int id, std::map<int, int>* resources)
    {
        this->id = id;
        this->resources = resources;
        this->bucketId = nullptr;
    }
}
