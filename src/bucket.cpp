#include <map>

class Bucket
{
private:
    // relation: resource id -> capacity of resource
    capacity std::map<int, int>;
};
