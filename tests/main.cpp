#include <cpp-unit-test.h>
#include <scheduler.h>
#include <map>
#include <iostream>

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

    // make expected distribution
    std::map<int, int*>* expectedDistribution;
    expectedDistribution = new std::map<int, int*>;

    // data-set for bucket #1
    int items1[9] = {1, 2, 18, 19, 20, 21, 22, 24, 0};
    expectedDistribution->emplace(1, items1);

    // data-set for bucket #2
    int items2[4] = {3, 23, 25, 0};
    expectedDistribution->emplace(2, items2);

    // data-set for bucket #3
    int items3[5] = {4, 26, 27, 29, 0};
    expectedDistribution->emplace(3, items3);

    // data-set for bucket #4
    int items4[5] = {5, 28, 30, 31, 0};
    expectedDistribution->emplace(4, items4);

    // data-set for bucket #5
    int items5[6] = {6, 14, 32, 33, 34, 0};
    expectedDistribution->emplace(5, items5);

    // data-set for bucket #6
    int items6[6] = {7, 15, 35, 36, 37, 0};
    expectedDistribution->emplace(6, items6);

    // data-set for bucket #7
    int items7[6] = {8, 16, 38, 39, 40, 0};
    expectedDistribution->emplace(7, items7);

    // data-set for bucket #8
    int items8[6] = {9, 17, 41, 42, 43, 0};
    expectedDistribution->emplace(8, items8);

    // data-set for bucket #9
    int items9[3] = {10, 11, 0};
    expectedDistribution->emplace(9, items9);

    // data-set for bucket #9
    int items10[3] = {12, 13, 0};
    expectedDistribution->emplace(10, items10);

    // distribution items in buckets
    std::map<int, int*>* distribution = s.__GetDistributionItems();

    assertDistribution(t, expectedDistribution, distribution);

    t->finish();
    return t;
}

int main() {
    CppUnitTest::TestSuite testSuite;

    testSuite.addTestCase(testSchedule_ValidData_Positive());

    testSuite.printTotal();

    return 0;
}
