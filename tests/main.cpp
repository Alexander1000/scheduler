#include <cpp-unit-test.h>
#include <scheduler.h>
#include <map>

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
        resourceMap->insert(std::pair<int, int>(rCpu, 64));
        resourceMap->insert(std::pair<int, int>(rMemory, 200000));
        resourceMap->insert(std::pair<int, int>(rGpu, 4));

        Scheduler::Bucket* bucket;
        bucket = new Scheduler::Bucket(i, resourceMap);
        s.AddBucket(bucket);
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
