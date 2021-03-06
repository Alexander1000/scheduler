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
            std::map<int, int>::iterator itElementUsage, itElementLeft;

            // update usage
            itElementUsage = this->usage->find(it->first);
            itElementUsage->second += it->second;

            // left resources
            itElementLeft = this->left->find(it->first);
            itElementLeft->second -= it->second;
        }

        this->items->push_front(item);
    }

    std::list<Item*>* Bucket::GetItems()
    {
        return this->items;
    }

    std::map<int, int>* Bucket::GetCapacity()
    {
        return this->capacity;
    }

    std::map<int, int>* Bucket::GetUsage()
    {
        return this->usage;
    }

    std::map<int, int>* Bucket::GetLeft()
    {
        return this->left;
    }

    float Bucket::GetFillRate()
    {
        float k = 0.0f;
        std::map<int, int>::iterator itCapacity;
        for (itCapacity = this->capacity->begin(); itCapacity != this->capacity->end(); ++itCapacity) {
            int usage = 0;
            if (this->usage->find(itCapacity->first) != this->usage->end()) {
                usage = this->usage->find(itCapacity->first)->second;
            }

            k += (float) usage /  (float) itCapacity->second;
        }

        return k / (float) this->capacity->size();
    }
}
