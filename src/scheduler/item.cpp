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

    int Item::GetId()
    {
        return this->id;
    }

    std::map<int, int>* Item::GetResources()
    {
        return this->resources;
    }

    void Item::SetBucket(int bucketId)
    {
        this->bucketId = new int;
        *this->bucketId = bucketId;
    }
}
