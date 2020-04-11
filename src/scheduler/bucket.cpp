#include <scheduler.h>
#include <list>

namespace Scheduler
{
    Bucket::Bucket(int id, std::map<int, int>* capacity)
    {
        this->id = id;
        this->capacity = capacity;
        this->items = new std::list<Item*>;
    }

    int Bucket::GetID()
    {
        return this->id;
    }
}
