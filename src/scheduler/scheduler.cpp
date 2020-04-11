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
}
