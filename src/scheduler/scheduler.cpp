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
        for (it = this->bucket_pool->begin(); it != this->bucket_pool->end(); ++it) {
            Bucket* bucket = *it;
            if (bucket->HasCapacityForItem(item)) {
                break;
            }
        }
    }
}
