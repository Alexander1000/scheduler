#include <scheduler.h>
#include <list>

namespace Scheduler
{
    Scheduler::Scheduler()
    {
        this->resource_pool = new std::list<Resource*>;
        this->bucket_pool = new std::list<Bucket*>;
        this->pending_items = new std::list<Item*>;
        this->strategy = StrategyType::SimpleType;
    }

    void Scheduler::AddBucket(Bucket* bucket)
    {
        this->bucket_pool->push_back(bucket);
    }

    void Scheduler::ScheduleItem(Item* item)
    {
        bool scheduled = false;

        if (this->strategy == StrategyType::SimpleType) {
            std::list<Bucket *>::iterator it;
            for (it = this->bucket_pool->begin(); it != this->bucket_pool->end(); ++it) {
                Bucket *bucket = *it;
                if (bucket->HasCapacityForItem(item)) {
                    bucket->AddItem(item);
                    item->SetBucket(bucket->GetID());
                    scheduled = true;
                    break;
                }
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
            items = new int[bucket->GetItems()->size() + 1];
            memset(items, 0, sizeof(int) * (bucket->GetItems()->size() + 1));

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

    void Scheduler::SetStrategy(StrategyType strategy)
    {
        this->strategy = strategy;
    }
}
