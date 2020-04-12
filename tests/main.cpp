#include <cpp-unit-test.h>
#include <scheduler.h>
#include <map>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <io-buffer.h>
#include <yaml-parser.h>

#include "sequence.cpp"

#define RESOURCE_CPU 1
#define RESOURCE_MEMORY 2
#define RESOURCE_GPU 3

Sequence sequence;

class AssertDistributionNotExists
{};

class AssertItemNotFoundInBucket
{};

class AssertUnExpectedItemExists
{};

class AssertPropertyNotExists
{};

class AssertInvalidYamlElementType
{};

class AssertInvalidResource
{};

typedef std::map<std::string, YamlParser::Element*> YamlObject;
typedef std::list<YamlParser::Element*> YamlArray;
typedef std::map<std::string, int> ResourceMapDict;
typedef std::map<int, int> ResourceMap;

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
                    std::cout << "Expected item #" << curItemId << " exists in bucket #" << itExpDistribution->first << std::endl;
                    if (itDistr->second != nullptr) {
                        std::cout << "List items in bucket #" << itExpDistribution->first << ": " << std::endl;
                        int j = 0;
                        while (itDistr->second[j] != 0) {
                            std::cout << "#" << itDistr->second[j] << std::endl;
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
                        std::cout << "Not expected item #" << curItemId << " exists in bucket #" << itDistr->first
                                  << std::endl;
                        if (itExpDistribution->second != nullptr) {
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
            std::cout << "Unexpected distribution exists for bucket #" << itDistribution->first << std::endl;
            throw new AssertDistributionNotExists;
        } else {
            t->increment();
        }
    }
}

Scheduler::Item* createItem(int cpu, int memory, int gpu)
{
    std::map<int, int>* resourceMap;
    resourceMap = new std::map<int, int>;
    if (cpu > 0) {
        resourceMap->insert(std::pair<int, int>(RESOURCE_CPU, cpu));
    }
    if (memory > 0) {
        resourceMap->insert(std::pair<int, int>(RESOURCE_MEMORY, memory));
    }
    if (gpu > 0) {
        resourceMap->insert(std::pair<int, int>(RESOURCE_GPU, gpu));
    }

    return new Scheduler::Item(sequence.GetNextID(), resourceMap);
}

CppUnitTest::TestCase* testSchedule_ValidData_Positive()
{
    CppUnitTest::TestCase* t = nullptr;
    t = new CppUnitTest::TestCase("001-first-integration-test");

    t->printTitle();

    // make scheduler
    Scheduler::Scheduler s;

    for (int i = 0; i < 10; i++) {
        std::map<int, int>* resourceMap;
        resourceMap = new std::map<int, int>;
        // 64 cpu
        resourceMap->insert(std::pair<int, int>(RESOURCE_CPU, 64));
        // 200 Gb memory
        resourceMap->insert(std::pair<int, int>(RESOURCE_MEMORY, 200000));
        // 4 gpu
        resourceMap->insert(std::pair<int, int>(RESOURCE_GPU, 4));

        Scheduler::Bucket* bucket;
        bucket = new Scheduler::Bucket(i + 1, resourceMap);
        s.AddBucket(bucket);
    }

    s.ScheduleItem(createItem(5, 1000, 0));

    for (int i = 0; i < 4; ++i) {
        s.ScheduleItem(createItem(9, 10000, 4));
    }

    for (int i = 0; i < 4; ++i) {
        s.ScheduleItem(createItem(10, 10000, 3));
    }

    for (int i = 0; i < 4; ++i) {
        s.ScheduleItem(createItem(10, 10000, 2));
    }

    for (int i = 0; i < 4; ++i) {
        s.ScheduleItem(createItem(10, 10000, 1));
    }

    for (int i = 0; i < 5; ++i) {
        s.ScheduleItem(createItem(5, 30000, 0));
    }

    s.ScheduleItem(createItem(30, 9000, 0));

    for (int i = 0; i < 5; ++i) {
        s.ScheduleItem(createItem(20, 5000, 0));
    }

    for (int i = 0; i < 15; ++i) {
        s.ScheduleItem(createItem(12, 50000, 0));
    }

    s.ScheduleItem(createItem(20, 90000, 0));

    s.ScheduleItem(createItem(40, 150000, 0));

    s.ScheduleItem(createItem(12, 50000, 0));

    for (int i = 0; i < 15; ++i) {
        s.ScheduleItem(createItem(3, 5000, 0));
    }

    // make expected distribution
    std::map<int, int*>* expectedDistribution;
    expectedDistribution = new std::map<int, int*>;

    // data-set for bucket #1
    int items1[10] = {1, 2, 18, 19, 20, 21, 22, 24, 47, 0};
    expectedDistribution->emplace(1, items1);

    // data-set for bucket #2
    int items2[5] = {3, 23, 25, 48, 0};
    expectedDistribution->emplace(2, items2);

    // data-set for bucket #3
    int items3[6] = {4, 26, 27, 29, 49, 0};
    expectedDistribution->emplace(3, items3);

    // data-set for bucket #4
    int items4[8] = {5, 28, 30, 31, 50, 51, 52, 0};
    expectedDistribution->emplace(4, items4);

    // data-set for bucket #5
    int items5[8] = {6, 14, 32, 33, 34, 53, 54, 0};
    expectedDistribution->emplace(5, items5);

    // data-set for bucket #6
    int items6[8] = {7, 15, 35, 36, 37, 55, 56, 0};
    expectedDistribution->emplace(6, items6);

    // data-set for bucket #7
    int items7[8] = {8, 16, 38, 39, 40, 57, 58, 0};
    expectedDistribution->emplace(7, items7);

    // data-set for bucket #8
    int items8[8] = {9, 17, 41, 42, 43, 59, 60, 0};
    expectedDistribution->emplace(8, items8);

    // data-set for bucket #9
    int items9[7] = {10, 11, 44, 46, 61, 0};
    expectedDistribution->emplace(9, items9);

    // data-set for bucket #9
    int items10[4] = {12, 13, 45, 0};
    expectedDistribution->emplace(10, items10);

    // distribution items in buckets
    std::map<int, int*>* distribution = s.__GetDistributionItems();

    assertDistribution(t, expectedDistribution, distribution);

    t->finish();
    return t;
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

CppUnitTest::TestCase* testSchedule_YamlTestCase_Positive(std::string fileName)
{
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

    // resource map: (eg: cpu=1; memory=2; gpu=3)
    std::map<std::string, int> resourceMap;

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
        i++;
    }

    // make scheduler
    Scheduler::Scheduler s;

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
    i = 0;
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

        i++;
    }

    // @todo: make items

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
    }

    // @todo: make expected distribution
    itObject = rObj->find("distribution");
    if (itObject == rObj->end()) {
        throw new AssertPropertyNotExists;
    }

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

    testSuite.addTestCase(testSchedule_ValidData_Positive());
    std::vector<std::string> v;

    struct dirent **namelist;

    // running from cmake-build-debug dir
    int n = scandir("../tests/data", &namelist, *filter, alphasort);
    for (int i = 0; i<n; i++) {
        testSuite.addTestCase(testSchedule_YamlTestCase_Positive(namelist[i]->d_name));
        free(namelist[i]);
    }
    free(namelist);

    testSuite.printTotal();

    return 0;
}
