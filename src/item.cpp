#include <map>

class Item
{
private:
    // identifier of item
    int id;

    // resources for item
    std::map<int, int> resources;

    // identifier of bucket
    int* bucketId;
};
