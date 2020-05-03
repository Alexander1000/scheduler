#ifndef H_SCHEDULER_SCHEDULER_INCLUDED
#define H_SCHEDULER_SCHEDULER_INCLUDED

#include <list>
#include <map>
#include <scheduler/resources.h>
#include <scheduler/bucket.h>
#include <scheduler/item.h>
#include <scheduler/strategy.h>

namespace Scheduler {
    // scheduler
    class Scheduler {
    public:
        Scheduler();
        // add bucket to pool
        void AddBucket(Bucket* bucket);

        // schedule item
        void ScheduleItem(Item* item);

        void SetStrategy(StrategyType strategy);

        void QueueItem(Item* item);

        std::map<int, int*>* __GetDistributionItems();

        std::list<Bucket*>* GetBucketPool();
    private:
        // pool of resources
        std::list<Resource*>* resource_pool;

        // pool of buckets
        std::list<Bucket*>* bucket_pool;

        // list items for schedule
        std::list<Item*>* pending_items;

        std::list<Item*>* scheduled_items;

        StrategyType strategy;

        void bindBucketWidthItem(Bucket* bucket, Item* item);
    };
}

#endif
