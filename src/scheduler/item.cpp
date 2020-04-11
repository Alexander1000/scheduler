#include <scheduler.h>
#include <map>

namespace Scheduler
{
    Item::Item(int id, std::map<int, int>* resources)
    {
        this->id = id;
        this->resources = resources;
        this->bucketId = nullptr;
    }

    std::map<int, int>* Item::GetResources()
    {
        return this->resources;
    }
}
