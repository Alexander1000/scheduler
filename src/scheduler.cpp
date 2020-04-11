#include <list>

class Scheduler
{
private:
    // pool of resources
    std::list<*Resource> resource_pool;

    // pool of buckets
    std::list<*Bucket> bucket_pool;

    // list items for schedule
    std::list<*Item> pending_items;
};
