#include <map>
#include <list>

class Bucket
{
private:
    // identifier of bucket
    int id;

    // relation: resource id -> capacity of resource
    std::map<int, int> capacity;

    std::list<*Item> items;
};
