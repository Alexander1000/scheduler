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
        std::map<int, int>::iterator it;
        bool hasResources = true;
        for (it = item->GetResources()->begin(); it != item->GetResources()->end(); ++it) {
            std::map<int, int>::iterator itElement;
            itElement = this->left->find(it->first);
            if (itElement != this->left->end()) {
                // found key
                if (itElement->second < it->second) {
                    // resources required more than exists
                    hasResources = false;
                    break;
                }
            } else {
                // resource not exists in this bucket
                hasResources = false;
                break;
            }
        }
        return hasResources;
    }

    void Bucket::AddItem(Item* item)
    {
        std::map<int, int>::iterator it;

        for (it = item->GetResources()->begin(); it != item->GetResources()->end(); ++it) {
            std::map<int, int>::iterator itElement;
            itElement = this->usage->find(it->first);

            // update usage
            this->usage->emplace(it->first, itElement->second + it->second);

            // update usage
            this->left->emplace(it->first, itElement->second - it->second);
        }

        this->items->push_front(item);
    }

    void Bucket::calculate()
    {
    }
}
