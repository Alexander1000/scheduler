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

        std::map<int, int>::iterator it;
        for (it = this->capacity->begin(); it != this->capacity->end(); ++it) {
            // zero usage
            this->usage->insert(std::pair<int, int>(it->first, 0));
            // left all resources
            this->left->insert(std::pair<int, int>(it->first, it->second));
        }

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
