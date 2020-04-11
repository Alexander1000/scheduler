#include <scheduler.h>
#include <list>

namespace Scheduler
{
    Scheduler::Scheduler()
    {
        this->resource_pool = new std::list<Resource*>;
        this->bucket_pool = new std::list<Bucket*>;
        this->pending_items = new std::list<Item*>;
    }

    void Scheduler::AddBucket(Bucket* bucket)
    {
        this->bucket_pool->push_front(bucket);
    }

    void Scheduler::ScheduleItem(Item* item)
    {
        std::list<Bucket*>::iterator it;
        bool scheduled = false;
        for (it = this->bucket_pool->begin(); it != this->bucket_pool->end(); ++it) {
            Bucket* bucket = *it;
            if (bucket->HasCapacityForItem(item)) {
                bucket->AddItem(item);
                item->SetBucket(bucket->GetID());
                scheduled = true;
                break;
            }
        }

        if (!scheduled) {
            this->pending_items->push_front(item);
        }
    }

    std::map<int, int*>* Scheduler::__GetDistributionItems()
    {
        std::map<int, int*>* distribution;
        distribution = new std::map<int, int*>;
        std::list<Bucket*>::iterator it;
        for (it = this->bucket_pool->begin(); it != this->bucket_pool->end(); ++it) {
            Bucket* bucket = *it;
            int* items;
            items = new int[bucket->GetItems()->size()];

            std::list<Item*>::iterator itElement;
            int i = 0;
            for (itElement = bucket->GetItems()->begin(); itElement != bucket->GetItems()->end(); ++itElement) {
                items[i] = (*itElement)->GetId();
                i++;
            }

            distribution->emplace(bucket->GetID(), items);
        }
        return distribution;
    }
}
