#ifndef H_SCHEDULER_SCHEDULER_INCLUDED
#define H_SCHEDULER_SCHEDULER_INCLUDED

#include <list>
#include <map>
#include <scheduler/resources.h>
#include <scheduler/bucket.h>
#include <scheduler/item.h>
#include <scheduler/strategy.h>

#define SCHEDULE_STATISTIC_ITEMS_COUNT 5
#define SCHEDULE_STATISTIC_RANDOM_ITEMS_COUNT 15
#define MAX_PENDING_ITEMS_FOR_SCHEDULE 5

// itemID: fill factor
typedef std::map<int, float> FillFactorMap;

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

        bool scheduleLeastLoad(Item* item);

        bool scheduleStatistic(Item* item);

        bool scheduleDeferred(Item* item);

        Item* getAverageItem();

        void bindBucketWidthItem(Bucket* bucket, Item* item);

        std::list<Item*>* getRandomListItems(int count);

        static float getFillFactor(Item* item, std::map<int, int>* leftResources);

        static int analyzeFillFactorMatrix(std::map<int, FillFactorMap*>* matrix);

        Bucket* getBucketByID(int bucketID);
    };
}

#endif
