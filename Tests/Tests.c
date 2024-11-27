#include "TestingUtilities.h"
#include "Scene.h"
#include <stdlib.h>

void TestFreeQueue()
{
    TEST(FreeQueueInit(), ==, 0, d)

    void *testAlloc = malloc(16);

    TEST(testAlloc, !=, NULL, p)
    TEST(FreeQueuePush(testAlloc), ==, 0, d)
    
    FreeQueueFlush();
}

int main(int argCount, char **argValues)
{
    TestFreeQueue();

    TestsEnd();
}