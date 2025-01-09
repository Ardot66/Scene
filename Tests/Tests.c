#include "TestingUtilities.h"
#include "ComponentObjects.h"
#include "Scene.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// Both objectData and object returned need to be freed.
int TestObjectCreate(ObjectData **objectDataDest, void **objectDest, const size_t componentCount, const ComponentData **components)
{
    ObjectData *objectData;
    int result = ObjectInitialize(&objectData, componentCount, components);
    if(result) return result;

    void *object = malloc(objectData->Size);
    if(object == NULL) return errno;

    *objectDataDest = objectData;
    *objectDest = object;

    return 0;
}

int ObjectCallInterfaceFunction(ObjectData *objectData, void *object, const InterfaceData *interface, int (*Caller)(void *object, const ObjectInterfaceInstanceData *interfaceInstance))
{
    int result;
    ObjectInterfaceData *objectInterface = ObjectGetInterface(objectData, interface);

    for(size_t x = 0; x < objectInterface->ImplementingComponentsCount; x++)
    {
        ObjectInterfaceInstanceData *interfaceInstance = objectInterface->ImplementingComponents + x;
        result = Caller(object, interfaceInstance);

        if(result) return result;
    }

    return 0;
}

int ObjectInitializeCaller(void *object, const ObjectInterfaceInstanceData *interfaceInstance)
{
    IReadyable *readyable = interfaceInstance->VTable;
    if(readyable->Initialize == NULL)
        return 0;

    return readyable->Initialize(CRef(object, interfaceInstance->Component));
}

int ObjectExitCaller(void *object, const ObjectInterfaceInstanceData *interfaceInstance)
{
    IReadyable *readyable = interfaceInstance->VTable;
    if(readyable->Exit == NULL)
        return 0;

    return readyable->Exit(CRef(object, interfaceInstance->Component));
}

void TestFreeQueue()
{
    TEST(FreeQueueInit(), ==, 0, d)

    void *testAlloc = malloc(16);

    TEST(testAlloc, !=, NULL, p)
    TEST(FreeQueuePushPointer(testAlloc), ==, 0, d)
    
    TEST(FreeQueueFlush(), ==, 0, d)
}

void TestNode()
{
    const size_t componentCount = 1;
    const ComponentData **objectComponents = COMPONENTS(TYPEOF(Node));

    ObjectData *objectData = NULL;
    void *object = NULL;
    int result = TestObjectCreate(&objectData, &object, componentCount, objectComponents);
    TEST(result, ==, 0, d, goto Exit;)

    ObjectComponentData *nodeData = ObjectGetComponent(objectData, TYPEOF(Node));
    Node *node = POINTER_OFFSET(object, nodeData->Offset);

    node->Parent = (ComponentReference){object, nodeData};

    result = ObjectCallInterfaceFunction(objectData, object, TYPEOF(IReadyable), ObjectInitializeCaller);
    TEST(result, ==, 0, d, goto Exit;)
    TEST(node->ChildCount, ==, 0, llu)

    size_t addedIndex;
    NodeAddChild(CRef(object, nodeData), CRef(object, nodeData), &addedIndex);

    TEST(addedIndex, ==, 0, zu)
    TEST(node->Children[0].Component, ==, nodeData, p)

    NodeRemoveChild(CRef(object, nodeData), addedIndex);

    TEST(node->ChildCount, ==, 0, zu)
    TEST(node->Children[0].Object, ==, NULL, p)

    NodeInsertChild(CRef(object, nodeData), CRef(object, nodeData), 2);

    TEST(node->ChildCount, ==, 3, zu)
    TEST(node->Children[2].Component, ==, nodeData, p)
    TEST(node->Children[1].Component, ==, NULL, p)

    result = ObjectCallInterfaceFunction(objectData, object, TYPEOF(IReadyable), ObjectExitCaller);
    TEST(result, ==, 0, d, goto Exit;)

    Exit:
    free(objectData);
    free(object);
}

struct MutexGroupTestContext
{
    ComponentReference MutexGroupCRef;
    int *FinishedFlag;
};

void *MutexGroupTestThread(void *param)
{
    struct MutexGroupTestContext testContext = *(struct MutexGroupTestContext*)param;
    MutexGroupLock(testContext.MutexGroupCRef, MUTEX_GROUP_MODE_WRITE);
    *testContext.FinishedFlag = 1;
    MutexGroupUnlock(testContext.MutexGroupCRef);
    return NULL;
}

void TestNodeMutexGroup()
{
    const ComponentData **testComponentData = COMPONENTS(TYPEOF(MutexGroup));

    void *object;
    ObjectData *objectData;
    TEST(TestObjectCreate(&objectData, &object, 1, testComponentData), ==, 0, d, return;)
    TEST(ObjectCallInterfaceFunction(objectData, object, TYPEOF(IReadyable), ObjectInitializeCaller), ==, 0, d, return;)

    ObjectComponentData *mutexGroupComponent = ObjectGetComponent(objectData, TYPEOF(MutexGroup));
    ComponentReference mutexGroupCRef = CRef(object, mutexGroupComponent);
    MutexGroup *mutexGroup = CRefComponent(mutexGroupCRef);

    MutexGroupLock(mutexGroupCRef, MUTEX_GROUP_MODE_READ);

    int finishedFlag = 0;
    struct MutexGroupTestContext testContext = {mutexGroupCRef, &finishedFlag};
    pthread_t thread;
    pthread_create(&thread, NULL, MutexGroupTestThread, &testContext);

    struct timespec waitTime;
    waitTime.tv_nsec = 10000000; // wait ten milliseconds
    waitTime.tv_sec = 0;
    nanosleep(&waitTime, NULL);
    TEST(finishedFlag, ==, 0, d);

    MutexGroupUnlock(mutexGroupCRef);
    
    nanosleep(&waitTime, NULL);
    TEST(finishedFlag, ==, 1, d);

    free(objectData);
    free(object);
}

int main(int argCount, char **argValues)
{
    TestFreeQueue();
    TestNode();
    TestNodeMutexGroup();

    TestsEnd();
}