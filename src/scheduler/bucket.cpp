#include <scheduler.h>
#include <list>

namespace Scheduler
{
    Bucket::Bucket(int id, std::map<int, int>* capacity)
    {
        this->id = id;

        // resources
        this->capacity = capacity;
        this->usage = new std::map<int, int>;
        this->left = new std::map<int, int>;

        this->items = new std::list<Item*>;

        this->calculate();
    }

    int Bucket::GetID()
    {
        return this->id;
    }

    bool Bucket::HasCapacityForItem(Item* item)
    {
        return true;
    }

    void Bucket::calculate()
    {
    }
}
