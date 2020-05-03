#include <scheduler.h>
#include <list>
#include <map>

namespace Scheduler
{
    Scheduler::Scheduler()
    {
        this->resource_pool = new std::list<Resource*>;
        this->bucket_pool = new std::list<Bucket*>;
        this->pending_items = new std::list<Item*>;
        this->scheduled_items = new std::list<Item*>;
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
            // schedule first compatible bucket
            std::list<Bucket *>::iterator it;
            for (it = this->bucket_pool->begin(); it != this->bucket_pool->end(); ++it) {
                Bucket *bucket = *it;
                if (bucket->HasCapacityForItem(item)) {
                    this->bindBucketWidthItem(bucket, item);
                    scheduled = true;
                    break;
                }
            }
        } else if (this->strategy == StrategyType::LeastLoadedType) {
            // search least loaded bucket
            scheduled = this->scheduleLeastLoad(item);
        } else if (this->strategy == StrategyType::StatisticType) {
            // statistic
        } else if (this->strategy == StrategyType::DeferredType) {
            // deferred
        }

        if (!scheduled) {
            this->pending_items->push_back(item);
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

    void Scheduler::QueueItem(Item * item)
    {
        this->pending_items->push_back(item);
    }

    std::list<Bucket*>* Scheduler::GetBucketPool()
    {
        return this->bucket_pool;
    }

    void Scheduler::bindBucketWidthItem(Bucket* bucket, Item* item)
    {
        bucket->AddItem(item);
        item->SetBucket(bucket->GetID());
        this->scheduled_items->push_back(item);
    }

    bool Scheduler::scheduleLeastLoad(Item *item)
    {
        float bestScore = 2.0f;
        Bucket* bestBucket = nullptr;

        std::list<Bucket*>::iterator itBucket;

        for (itBucket = this->bucket_pool->begin(); itBucket != this->bucket_pool->end(); ++itBucket) {
            Bucket* bucket = (*itBucket);
            if (bucket->HasCapacityForItem(item)) {
                float score = bucket->GetFillRate();
                if (score < bestScore) {
                    bestScore = score;
                    bestBucket = bucket;
                    continue;
                }
            }
        }

        if (bestBucket == nullptr) {
            return false;
        }

        this->bindBucketWidthItem(bestBucket, item);
        return true;
    }
}
