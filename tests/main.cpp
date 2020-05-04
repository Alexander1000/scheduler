#include <cpp-unit-test.h>
#include <scheduler.h>
#include <map>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <io-buffer.h>
#include <yaml-parser.h>
#include <shell-grid.h>
#include <memory>

#include "sequence.cpp"

class AssertException
{};

class AssertDistributionNotExists : AssertException
{};

class AssertItemNotFoundInBucket : AssertException
{};

class AssertUnExpectedItemExists : AssertException
{};

class AssertPropertyNotExists : AssertException
{};

class AssertInvalidYamlElementType : AssertException
{};

class AssertInvalidResource : AssertException
{};

class AssertInvalidStrategy : AssertException
{};

typedef std::map<std::string, YamlParser::Element*> YamlObject;
typedef std::list<YamlParser::Element*> YamlArray;
typedef std::map<std::string, int> ResourceMapDict;
typedef std::map<int, int> ResourceMap;
typedef std::map<int, int*> DistributionMap;

void assertDistribution(CppUnitTest::TestCase* t, std::map<int, int*>* expectedDistribution, std::map<int, int*>* distribution)
{
    // assertions
    std::map<int, int*>::iterator itExpDistribution;
    for (itExpDistribution = expectedDistribution->begin(); itExpDistribution != expectedDistribution->end(); ++itExpDistribution) {
        std::map<int, int*>::iterator itDistr = distribution->find(itExpDistribution->first);
        if (itDistr != distribution->end() && itExpDistribution->second != nullptr) {
            t->increment();

            int i = 0;
            while (itExpDistribution->second[i] != 0) {
                int curItemId = itExpDistribution->second[i];
                bool found = false;
                if (itDistr->second != nullptr) {
                    int j = 0;
                    while (itDistr->second[j] != 0) {
                        if (curItemId == itDistr->second[j]) {
                            found = true;
                            t->increment();
                            break;
                        }
                        j++;
                    }
                }

                if (!found) {
                    std::cout << std::endl;
                    std::cout << "Expected item #" << curItemId << " exists in bucket #" << itExpDistribution->first << std::endl;
                    if (itDistr->second != nullptr) {
                        std::cout << std::endl;
                        std::cout << "List items in bucket #" << itExpDistribution->first << ": " << std::endl;
                        int j = 0;
                        while (itDistr->second[j] != 0) {
                            std::cout << "#" << itDistr->second[j] << std::endl;
                            j++;
                        }
                        std::cout << "Expected items:" << std::endl;
                        j = 0;
                        while (itExpDistribution->second[j] != 0) {
                            std::cout << "#" << itExpDistribution->second[j] << std::endl;
                            j++;
                        }
                    }

                    throw new AssertItemNotFoundInBucket;
                }
                i++;
            }

            // inverted search
            if (itDistr->second != nullptr) {
                i = 0;
                while (itDistr->second[i] != 0) {
                    int curItemId = itDistr->second[i];
                    bool found = false;
                    if (itExpDistribution->second != nullptr) {
                        int j = 0;
                        while (itExpDistribution->second[j] != 0) {
                            if (curItemId == itExpDistribution->second[j]) {
                                t->increment();
                                found = true;
                                break;
                            }
                            j++;
                        }
                    }

                    if (!found) {
                        std::cout << std::endl;
                        std::cout << "Not expected item #" << curItemId << " exists in bucket #" << itDistr->first
                                  << std::endl;
                        if (itExpDistribution->second != nullptr) {
                            std::cout << std::endl;
                            std::cout << "Expected list items in bucket #" << itExpDistribution->first << ": "
                                      << std::endl;
                            int j = 0;
                            while (itExpDistribution->second[j] != 0) {
                                std::cout << "#" << itExpDistribution->second[j] << std::endl;
                                j++;
                            }
                        }

                        throw new AssertUnExpectedItemExists;
                    }
                    i++;
                }
            }
        } else {
            // not exist
            throw new AssertDistributionNotExists;
        }
    }

    std::map<int, int*>::iterator itDistribution;
    for (itDistribution = distribution->begin(); itDistribution != distribution->end(); ++itDistribution) {
        std::map<int, int*>::iterator itExpDistr = expectedDistribution->find(itDistribution->first);
        if (itExpDistr == expectedDistribution->end()) {
            // not found
            std::cout << std::endl;
            std::cout << "Unexpected distribution exists for bucket #" << itDistribution->first << std::endl;
            throw new AssertDistributionNotExists;
        } else {
            t->increment();
        }
    }
}

ResourceMap* parseResourcesFromYaml(YamlObject* objResources, ResourceMapDict* dictResources)
{
    ResourceMap* resourceMap;
    resourceMap = new ResourceMap;

    YamlObject::iterator itResource;

    // iterate bucket resources
    for (itResource = objResources->begin(); itResource != objResources->end(); ++itResource) {
        if (itResource->second->getType() != YamlParser::ElementType::PlainTextType) {
            throw new AssertInvalidYamlElementType;
        }

        std::string* sAmount = (std::string*) itResource->second->getData();
        int amount = atoi(sAmount->c_str());

        ResourceMapDict::iterator itResourceMap = dictResources->find(itResource->first);
        if (itResourceMap == dictResources->end()) {
            throw new AssertInvalidResource;
        }
        resourceMap->insert(std::pair<int, int>(itResourceMap->second, amount));
    }

    return resourceMap;
}

void addResources(ResourceMap* dest, ResourceMap* source)
{
    ResourceMap::iterator itSource;
    for (itSource = source->begin(); itSource != source->end(); ++itSource) {
        ResourceMap::iterator itDest = dest->find(itSource->first);
        itDest->second += itSource->second;
    }
}

CppUnitTest::TestCase* testSchedule_YamlTestCase_Positive(std::string fileName)
{
    Sequence sequence;
    CppUnitTest::TestCase* t = nullptr;

    char buff[FILENAME_MAX];
    memset(buff, 0, sizeof(char) * FILENAME_MAX);
    std::sprintf(buff, "../tests/data/%s", fileName.c_str());

    IOBuffer::IOFileReader fileReader(buff);
    IOBuffer::CharStream charStream(&fileReader);
    YamlParser::Stream yamlStream(&charStream);
    YamlParser::Decoder decoder(&yamlStream);

    YamlParser::Element* rElement = decoder.parse();
    YamlObject* rObj = (YamlObject*) rElement->getData();

    YamlObject::iterator itObject = rObj->find("name");
    if (itObject == rObj->end()) {
        throw new AssertPropertyNotExists;
    }
    if (itObject->second->getType() != YamlParser::ElementType::PlainTextType) {
        throw new AssertInvalidYamlElementType;
    }
    std::string* testSuiteName = (std::string*) itObject->second->getData();
    t = new CppUnitTest::TestCase(testSuiteName->c_str());
    t->printTitle();

    // make scheduler
    Scheduler::Scheduler s;

    itObject = rObj->find("strategy");
    if (itObject == rObj->end()) {
        throw new AssertPropertyNotExists;
    }
    if (itObject->second->getType() != YamlParser::ElementType::PlainTextType) {
        throw new AssertInvalidYamlElementType;
    }
    std::string* strategyName = (std::string*) itObject->second->getData();
    if (strategyName->find("simple") != std::string::npos) {
        s.SetStrategy(Scheduler::StrategyType::SimpleType);
    } else if (strategyName->find("least loaded") != std::string::npos) {
        s.SetStrategy(Scheduler::StrategyType::LeastLoadedType);
    } else if (strategyName->find("statistic") != std::string::npos) {
        s.SetStrategy(Scheduler::StrategyType::StatisticType);
    } else if (strategyName->find("deferred") != std::string::npos) {
        s.SetStrategy(Scheduler::StrategyType::DeferredType);
    } else {
        throw new AssertInvalidStrategy;
    }

    // resource map: (eg: cpu=1; memory=2; gpu=3)
    ResourceMapDict resourceMap;

    // total resources available
    ResourceMap totalResources;

    // total item resources
    ResourceMap totalItemResources;

    // parse from yaml resource map
    itObject = rObj->find("resources");
    if (itObject == rObj->end()) {
        throw new AssertPropertyNotExists;
    }

    if (itObject->second->getType() != YamlParser::ElementType::ListType) {
        throw new AssertInvalidYamlElementType;
    }

    YamlArray* lResources = (YamlArray*) itObject->second->getData();
    YamlArray::iterator itResources;
    int i = 1;
    for (itResources = lResources->begin(); itResources != lResources->end(); ++itResources) {
        if ((*itResources)->getType() != YamlParser::ElementType::PlainTextType) {
            throw new AssertInvalidYamlElementType;
        }

        std::string* resourceName = (std::string*) (*itResources)->getData();
        // fill resource map
        resourceMap.insert(std::pair<std::string, int>(*resourceName, i));
        // init total resources
        totalResources.insert(std::pair<int, int>(i, 0));
        // total item resources
        totalItemResources.insert(std::pair<int, int>(i, 0));
        i++;
    }

    // make buckets

    itObject = rObj->find("buckets");
    if (itObject == rObj->end()) {
        throw new AssertPropertyNotExists;
    }

    if (itObject->second->getType() != YamlParser::ElementType::ListType) {
        throw new AssertInvalidYamlElementType;
    }

    YamlArray* lBuckets = (YamlArray*) itObject->second->getData();
    YamlArray::iterator itBucket;
    i = 1;
    for (itBucket = lBuckets->begin(); itBucket != lBuckets->end(); ++itBucket) {
        if ((*itBucket)->getType() != YamlParser::ElementType::ObjectType) {
            throw new AssertInvalidYamlElementType;
        }
        YamlObject* elBucket = (YamlObject*) (*itBucket)->getData();

        YamlObject::iterator itBucketProp = elBucket->find("resources");
        if (itBucketProp == elBucket->end()) {
            throw new AssertPropertyNotExists;
        }
        if (itBucketProp->second->getType() != YamlParser::ElementType::ObjectType) {
            throw new AssertInvalidYamlElementType;
        }

        YamlObject* objBucketResources = (YamlObject*) itBucketProp->second->getData();
        YamlObject::iterator itResBucket;

        ResourceMap* resBucket = parseResourcesFromYaml(objBucketResources, &resourceMap);

        // add bucket in scheduler
        s.AddBucket(new Scheduler::Bucket(i, resBucket));

        addResources(&totalResources, resBucket);

        i++;
    }

    // make items

    itObject = rObj->find("items");
    if (itObject == rObj->end()) {
        throw new AssertPropertyNotExists;
    }

    if (itObject->second->getType() != YamlParser::ElementType::ListType) {
        throw new AssertInvalidYamlElementType;
    }

    YamlArray* lItems = (YamlArray*) itObject->second->getData();
    YamlArray::iterator itItems;
    for (itItems = lItems->begin(); itItems != lItems->end(); ++itItems) {
        if ((*itItems)->getType() != YamlParser::ElementType::ObjectType) {
            throw new AssertInvalidYamlElementType;
        }
        YamlObject* elItem = (YamlObject*) (*itItems)->getData();
        YamlObject::iterator itItem = elItem->find("resources");
        if (itItem == elItem->end()) {
            throw new AssertPropertyNotExists;
        }
        if (itItem->second->getType() != YamlParser::ElementType::ObjectType) {
            throw new AssertInvalidYamlElementType;
        }
        ResourceMap* resourceItem = parseResourcesFromYaml((YamlObject*) itItem->second->getData(), &resourceMap);
        int count = 1;
        itItem = elItem->find("count");
        if (itItem != elItem->end()) {
            if (itItem->second->getType() != YamlParser::ElementType::PlainTextType) {
                throw new AssertInvalidYamlElementType;
            }
            std::string* sAmount = (std::string*) itItem->second->getData();
            count = atoi(sAmount->c_str());
        }

        for (int j = 0; j < count; ++j) {
            s.ScheduleItem(new Scheduler::Item(sequence.GetNextID(), resourceItem));
            addResources(&totalItemResources, resourceItem);
        }
    }

    s.Schedule();

    // make expected distribution
    DistributionMap* expectedDistribution;
    expectedDistribution = new DistributionMap;

    itObject = rObj->find("distribution");
    if (itObject == rObj->end()) {
        throw new AssertPropertyNotExists;
    }

    if (itObject->second->getType() != YamlParser::ElementType::ListType) {
        throw new AssertInvalidYamlElementType;
    }

    YamlArray* aDistribution = (YamlArray*) itObject->second->getData();
    YamlArray::iterator itDistribution;
    for (itDistribution = aDistribution->begin(); itDistribution != aDistribution->end(); ++itDistribution) {
        if ((*itDistribution)->getType() != YamlParser::ElementType::ObjectType) {
            throw new AssertInvalidYamlElementType;
        }
        YamlObject* elDistribution = (YamlObject*) (*itDistribution)->getData();

        YamlObject::iterator itElementDistribution;

        itElementDistribution = elDistribution->find("id");
        if (itElementDistribution == elDistribution->end()) {
            throw new AssertPropertyNotExists;
        }
        if (itElementDistribution->second->getType() != YamlParser::ElementType::PlainTextType) {
            throw new AssertInvalidYamlElementType;
        }
        std::string* sIdBucket = (std::string*) itElementDistribution->second->getData();
        int idBucket = atoi(sIdBucket->c_str()); // bucket id

        itElementDistribution = elDistribution->find("ids");
        if (itElementDistribution == elDistribution->end()) {
            throw new AssertPropertyNotExists;
        }
        if (itElementDistribution->second->getType() != YamlParser::ElementType::ListType) {
            throw new AssertInvalidYamlElementType;
        }
        YamlArray* aListIds = (YamlArray*) itElementDistribution->second->getData();
        YamlArray::iterator itListIds;
        int* itemIds = new int[aListIds->size() + 1];
        memset(itemIds, 0, sizeof(int) * (aListIds->size() + 1));
        int j = 0;
        for (itListIds = aListIds->begin(); itListIds != aListIds->end(); ++itListIds) {
            if ((*itListIds)->getType() != YamlParser::ElementType::PlainTextType) {
                throw new AssertInvalidYamlElementType;
            }
            std::string* sItemId = (std::string*) (*itListIds)->getData();
            int itemId = atoi(sItemId->c_str()); // item id
            itemIds[j] = itemId;
            j++;
        }

        expectedDistribution->insert(std::pair<int, int*>(idBucket, itemIds));
    }

    // Print info
    std::cout << "< = = = = = = = = = >" << std::endl;

    // Print total resources
    std::cout << "Initial resources:" << std::endl;
    ShellGrid::Grid gridTotalResources(resourceMap.size() + 1, 3);
    gridTotalResources.Set(0, 0, new ShellGrid::CellString("Resource"));
    gridTotalResources.Set(0, 1, new ShellGrid::CellString("Buckets"));
    gridTotalResources.Set(0, 2, new ShellGrid::CellString("Items"));
    ResourceMap::iterator itTotalResources;
    ResourceMapDict::iterator itDictResources;
    for (itTotalResources = totalResources.begin(); itTotalResources != totalResources.end(); ++itTotalResources) {
        for (itDictResources = resourceMap.begin(); itDictResources != resourceMap.end(); ++itDictResources) {
            if (itTotalResources->first == itDictResources->second) {
                gridTotalResources.Set(itDictResources->second, 0, new ShellGrid::CellString(itDictResources->first));
                gridTotalResources.Set(itDictResources->second, 1, new ShellGrid::CellNumeric(itTotalResources->second));
                break;
            }
        }
    }

    ResourceMap::iterator itTotalItemResources;
    for (itTotalItemResources = totalItemResources.begin(); itTotalItemResources != totalItemResources.end(); ++itTotalItemResources) {
        for (itDictResources = resourceMap.begin(); itDictResources != resourceMap.end(); ++itDictResources) {
            if (itTotalItemResources->first == itDictResources->second) {
                gridTotalResources.Set(itDictResources->second, 2, new ShellGrid::CellNumeric(itTotalItemResources->second));
                break;
            }
        }
    }

    gridTotalResources.Output();

    std::cout << "Distribution resources" << std::endl;

    // (head + count buckets + footer) x (bucket + count_items + resources(total + usage + left) + % + k)
    ShellGrid::Grid gridBucketResourceDistribution(s.GetBucketPool()->size() + 2, 7);
    gridBucketResourceDistribution.Set(0, 0, new ShellGrid::CellString("Bucket"));
    gridBucketResourceDistribution.Set(0, 1, new ShellGrid::CellString("Count Items"));
    gridBucketResourceDistribution.Set(0, 2, new ShellGrid::CellString("Total resources"));
    gridBucketResourceDistribution.Set(0, 3, new ShellGrid::CellString("Usage resources"));
    gridBucketResourceDistribution.Set(0, 4, new ShellGrid::CellString("Left resources"));
    gridBucketResourceDistribution.Set(0, 5, new ShellGrid::CellString("% resources"));
    gridBucketResourceDistribution.Set(0, 6, new ShellGrid::CellString("K"));

    int totalItems = 0;
    ResourceMap usedResourcesMap;
    ResourceMap leftResourcesMap;

    char* capacityResources = new char[100];
    char* usageResources = new char[100];
    char* leftResources = new char[100];
    char* percentResources = new char[100];

    std::list<Scheduler::Bucket*>::iterator itBucketPool;
    for (itBucketPool = s.GetBucketPool()->begin(); itBucketPool != s.GetBucketPool()->end(); ++itBucketPool) {
        Scheduler::Bucket* bucket = (*itBucketPool);
        int bucketID = bucket->GetID();

        IOBuffer::IOMemoryBuffer ioMemoryBufferCapacity, ioMemoryBufferUsage, ioMemoryBufferLeft, ioMemoryBufferPercent;

        gridBucketResourceDistribution.Set(bucketID, 0, new ShellGrid::CellNumeric(bucketID));
        gridBucketResourceDistribution.Set(bucketID, 1, new ShellGrid::CellNumeric(bucket->GetItems()->size()));

        totalItems += bucket->GetItems()->size();

        ResourceMapDict::iterator itResourceMap, itResourceMapNext;
        for (itResourceMap = resourceMap.begin(); itResourceMap != resourceMap.end(); ++itResourceMap) {
            int capacity, usage, left;

            // capacity
            memset(capacityResources, 0, 100 * sizeof(char));
            if (bucket->GetCapacity()->find(itResourceMap->second) != bucket->GetCapacity()->end()) {
                capacity = bucket->GetCapacity()->find(itResourceMap->second)->second;
                sprintf(capacityResources, "%d", capacity);
                ioMemoryBufferCapacity.write(capacityResources, strlen(capacityResources));
            } else {
                capacity = 0;
                ioMemoryBufferCapacity.write((char*) "0", 1);
            }

            // usage
            memset(usageResources, 0, 100 * sizeof(char));
            if (bucket->GetUsage()->find(itResourceMap->second) != bucket->GetUsage()->end()) {
                usage = bucket->GetUsage()->find(itResourceMap->second)->second;
                sprintf(usageResources, "%d", usage);
                ioMemoryBufferUsage.write(usageResources, strlen(usageResources));
            } else {
                usage = 0;
                ioMemoryBufferUsage.write((char*) "0", 1);
            }

            // left
            memset(leftResources, 0, 100 * sizeof(char));
            if (bucket->GetLeft()->find(itResourceMap->second) != bucket->GetLeft()->end()) {
                left = bucket->GetLeft()->find(itResourceMap->second)->second;
                sprintf(leftResources, "%d", left);
                ioMemoryBufferLeft.write(leftResources, strlen(leftResources));
            } else {
                left = 0;
                ioMemoryBufferLeft.write((char*) "0", 1);
            }

            memset(percentResources, 0, sizeof(char));
            if (capacity > 0) {
                float percent = ((float) usage / (float) capacity) * 100;
                sprintf(percentResources, "%0.2f", percent);
                ioMemoryBufferPercent.write(percentResources, strlen(percentResources));
            } else {
                ioMemoryBufferPercent.write((char*) "NULL", 4);
            }

            itResourceMapNext = itResourceMap;
            itResourceMapNext++;
            if (itResourceMapNext != resourceMap.end()) {
                ioMemoryBufferCapacity.write((char*) " / ", 3);
                ioMemoryBufferUsage.write((char*) " / ", 3);
                ioMemoryBufferLeft.write((char*) " / ", 3);
                ioMemoryBufferPercent.write((char*) " / ", 3);
            }

            if (usedResourcesMap.find(itResourceMap->second) != usedResourcesMap.end()) {
                usedResourcesMap.find(itResourceMap->second)->second += usage;
            } else {
                usedResourcesMap.insert(std::pair<int, int>(itResourceMap->second, usage));
            }

            if (leftResourcesMap.find(itResourceMap->second) != leftResourcesMap.end()) {
                leftResourcesMap.find(itResourceMap->second)->second += left;
            } else {
                leftResourcesMap.insert(std::pair<int, int>(itResourceMap->second, left));
            }
        }

        memset(capacityResources, 0, 100 * sizeof(char));
        ioMemoryBufferCapacity.read(capacityResources, 100);
        gridBucketResourceDistribution.Set(bucketID, 2, new ShellGrid::CellString(capacityResources));

        memset(usageResources, 0, 100 * sizeof(char));
        ioMemoryBufferUsage.read(usageResources, 100);
        gridBucketResourceDistribution.Set(bucketID, 3, new ShellGrid::CellString(usageResources));

        memset(leftResources, 0, 100 * sizeof(char));
        ioMemoryBufferLeft.read(leftResources, 100);
        gridBucketResourceDistribution.Set(bucketID, 4, new ShellGrid::CellString(leftResources));

        memset(percentResources, 0, 100 * sizeof(char));
        ioMemoryBufferPercent.read(percentResources, 100);
        gridBucketResourceDistribution.Set(bucketID, 5, new ShellGrid::CellString(percentResources));

        char* koefResources = new char[100];
        memset(koefResources, 0, sizeof(char) * 100);
        sprintf(koefResources, "%0.02f", bucket->GetFillRate() *  100);
        gridBucketResourceDistribution.Set(bucketID, 6, new ShellGrid::CellString(koefResources));
    }

    // footer
    int lastRow = s.GetBucketPool()->size() + 1;
    float koef = 0.0f;
    gridBucketResourceDistribution.Set(lastRow, 0, new ShellGrid::CellString("Total"));
    gridBucketResourceDistribution.Set(lastRow, 1, new ShellGrid::CellNumeric(totalItems));

    IOBuffer::IOMemoryBuffer ioMBCapacity, ioMBUsage, ioMBLeft, ioMBPercent;

    ResourceMapDict::iterator itDictResourcesNext;
    for (itDictResources = resourceMap.begin(); itDictResources != resourceMap.end(); ++itDictResources) {
        int capacity, usage, left;

        // capacity
        memset(capacityResources, 0, 100 * sizeof(char));
        capacity = totalResources.find(itDictResources->second)->second;
        sprintf(capacityResources, "%d", capacity);
        ioMBCapacity.write(capacityResources, strlen(capacityResources));

        // usage
        memset(usageResources, 0, 100 * sizeof(char));
        usage = usedResourcesMap.find(itDictResources->second)->second;
        sprintf(usageResources, "%d", usage);
        ioMBUsage.write(usageResources, strlen(usageResources));

        // left
        memset(leftResources, 0, 100 * sizeof(char));
        left = leftResourcesMap.find(itDictResources->second)->second;
        sprintf(leftResources, "%d", left);
        ioMBLeft.write(leftResources, strlen(leftResources));

        float percent = ((float) usage / (float) capacity) * 100;
        sprintf(percentResources, "%0.2f", percent);
        ioMBPercent.write(percentResources, strlen(percentResources));

        koef += (float) usage / (float) capacity;

        itDictResourcesNext = itDictResources;
        itDictResourcesNext++;
        if (itDictResourcesNext != resourceMap.end()) {
            ioMBCapacity.write((char*) " / ", 3);
            ioMBUsage.write((char*) " / ", 3);
            ioMBLeft.write((char*) " / ", 3);
            ioMBPercent.write((char*) " / ", 3);
        }
    }

    memset(capacityResources, 0, 100 * sizeof(char));
    ioMBCapacity.read(capacityResources, 100);
    gridBucketResourceDistribution.Set(lastRow, 2, new ShellGrid::CellString(capacityResources));

    memset(usageResources, 0, 100 * sizeof(char));
    ioMBUsage.read(usageResources, 100);
    gridBucketResourceDistribution.Set(lastRow, 3, new ShellGrid::CellString(usageResources));

    memset(leftResources, 0, 100 * sizeof(char));
    ioMBLeft.read(leftResources, 100);
    gridBucketResourceDistribution.Set(lastRow, 4, new ShellGrid::CellString(leftResources));

    memset(percentResources, 0, 100 * sizeof(char));
    ioMBPercent.read(percentResources, 100);
    gridBucketResourceDistribution.Set(lastRow, 5, new ShellGrid::CellString(percentResources));

    koef = (koef / (float) resourceMap.size()) * 100;
    char* koefResources = new char[100];
    memset(koefResources, 0, sizeof(char) * 100);
    sprintf(koefResources, "%0.02f", koef);
    gridBucketResourceDistribution.Set(lastRow, 6, new ShellGrid::CellString(koefResources));

    gridBucketResourceDistribution.Output();

    // distribution items in buckets
    DistributionMap* distribution = s.__GetDistributionItems();

    assertDistribution(t, expectedDistribution, distribution);

    t->finish();
    return t;
}

static int filter(const struct dirent* dir_ent)
{
    if (!strcmp(dir_ent->d_name, ".") || !strcmp(dir_ent->d_name, "..")) {
        return 0;
    }

    std::string fname = dir_ent->d_name;

    if (fname.find(".yaml") == std::string::npos) {
        return 0;
    }

    return 1;
}

int main() {
    CppUnitTest::TestSuite testSuite;

    std::vector<std::string> v;

    struct dirent **namelist;

    // running from cmake-build-debug dir
    int n = scandir("../tests/data", &namelist, *filter, alphasort);
    for (int i = 0; i<n; i++) {
        try {
            testSuite.addTestCase(testSchedule_YamlTestCase_Positive(namelist[i]->d_name));
        } catch (...) {
            std::cout << "The test case failed" << std::endl;
        }

        free(namelist[i]);
        std::cout << std::endl;
    }
    free(namelist);

    testSuite.printTotal();

    return 0;
}
