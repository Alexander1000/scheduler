#include <cpp-unit-test.h>
#include <scheduler.h>
#include <map>
#include <iostream>

class AssertDistributionNotExists
{};

class AssertItemNotFoundInBucket
{};

CppUnitTest::TestCase* testSchedule_ValidData_Positive()
{
    CppUnitTest::TestCase* t = nullptr;
    t = new CppUnitTest::TestCase("001-first-integration-test");

    t->printTitle();

    // make scheduler
    Scheduler::Scheduler s;

    // resources
    int rCpu = 1, rMemory = 2, rGpu = 3;

    for (int i = 0; i < 10; i++) {
        std::map<int, int>* resourceMap;
        resourceMap = new std::map<int, int>;
        // 64 cpu
        resourceMap->insert(std::pair<int, int>(rCpu, 64));
        // 200 Gb memory
        resourceMap->insert(std::pair<int, int>(rMemory, 200000));
        // 4 gpu
        resourceMap->insert(std::pair<int, int>(rGpu, 4));

        Scheduler::Bucket* bucket;
        bucket = new Scheduler::Bucket(i, resourceMap);
        s.AddBucket(bucket);
    }

    std::map<int, int> itemR1;
    itemR1.insert(std::pair<int, int>(rCpu, 5));
    itemR1.insert(std::pair<int, int>(rMemory, 1000));
    Scheduler::Item item1(1, &itemR1);

    s.ScheduleItem(&item1);

    for (int i = 0; i < 4; ++i) {
        std::map<int, int>* itemR2;
        itemR2 = new std::map<int, int>;
        itemR2->insert(std::pair<int, int>(rCpu, 9));
        itemR2->insert(std::pair<int, int>(rMemory, 10000));
        itemR2->insert(std::pair<int, int>(rGpu, 4));
        Scheduler::Item* item2;
        item2 = new Scheduler::Item(i + 1, itemR2);
        s.ScheduleItem(item2);
    }

    // make expected distribution
    std::map<int, int*>* expectedDistribution;
    expectedDistribution = new std::map<int, int*>;
    int* items1;
    items1 = new int[10];
    memset(items1, 0, 10);
    items1[0] = 1;
    expectedDistribution->emplace(1, items1);

    // distribution items in buckets
    std::map<int, int*>* distribution = s.__GetDistributionItems();

    // assertions
    std::map<int, int*>::iterator itExpDistribution;
    for (itExpDistribution = expectedDistribution->begin(); itExpDistribution != expectedDistribution->end(); ++itExpDistribution) {
        std::map<int, int*>::iterator itDistr = distribution->find(itExpDistribution->first);
        if (itDistr != distribution->end() && itExpDistribution->second != nullptr) {
            int i = 0;
            while (itExpDistribution->second[i] != 0) {
                int curItemId = itExpDistribution->second[i];
                bool found = false;
                if (itDistr->second != nullptr) {
                    int j = 0;
                    while (itDistr->second[j] != 0) {
                        if (curItemId == itDistr->second[j]) {
                            found = true;
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
        } else {
            // not exist
            throw new AssertDistributionNotExists;
        }
    }

    t->finish();
    return t;
}

int main() {
    CppUnitTest::TestSuite testSuite;

    testSuite.addTestCase(testSchedule_ValidData_Positive());

    testSuite.printTotal();

    return 0;
}
