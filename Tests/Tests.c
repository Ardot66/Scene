#include "TestingUtilities.h"
#include "Scene.h"
#include <stdlib.h>

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

int TestObjectReady(ObjectData *objectData, void *object)
{
    int result;
    ObjectInterfaceData *readyableInterface = ObjectGetInterface(objectData, TYPEOF(Readyable));

    for(size_t x = 0; x < readyableInterface->ImplementingComponentsCount; x++)
    {
        ObjectInterfaceInstanceData *readyableInstance = readyableInterface->ImplementingComponents + x;
        Readyable *readyable = readyableInstance->VTable;

        result = readyable->Ready(object, readyableInstance->Component);

        if(result) return result;
    }

    return 0;
}

int TestObjectExit(ObjectData *objectData, void *object)
{
    int result;
    ObjectInterfaceData *readyableInterface = ObjectGetInterface(objectData, TYPEOF(Readyable));

    for(size_t x = 0; x < readyableInterface->ImplementingComponentsCount; x++)
    {
        ObjectInterfaceInstanceData *readyableInstance = readyableInterface->ImplementingComponents + x;
        Readyable *readyable = readyableInstance->VTable;

        result = readyable->Exit(object, readyableInstance->Component);

        if(result) return result;
    }

    return 0;
}

void TestFreeQueue()
{
    TEST(FreeQueueInit(), ==, 0, d)

    void *testAlloc = malloc(16);

    TEST(testAlloc, !=, NULL, p)
    TEST(FreeQueuePush(testAlloc), ==, 0, d)
    
    FreeQueueFlush();
}

void TestNode()
{
    const size_t componentCount = 1;
    const ComponentData **objectComponents = COMPONENTS(TYPEOF(Node));

    ObjectData *objectData = NULL;
    void *object = NULL;
    int result = TestObjectCreate(&objectData, &object, componentCount, objectComponents);
    TEST(result, ==, 0, d, goto Exit;)

    ObjectInterfaceInstanceData *iNodeInstance = ObjectGetInterface(objectData, TYPEOF(INode))->ImplementingComponents;
    INode *iNode = iNodeInstance->VTable;
    void *node = POINTER_OFFSET(object, iNodeInstance->Component->Offset);

    ComponentReference *nodeParent = POINTER_OFFSET(node, iNode->Parent);
    *nodeParent = (ComponentReference){.Object = object, .ComponentData = iNodeInstance->Component};

    result = TestObjectReady(objectData, object);
    TEST(result, ==, 0, d, goto Exit;)

    size_t *nodeChildCount = POINTER_OFFSET(node, iNode->ChildCount);
    TEST(*nodeChildCount, ==, 0, llu)

    result = TestObjectExit(objectData, object);
    TEST(result, ==, 0, d, goto Exit;)

    Exit:

    free(objectData);
    free(object);
}

int main(int argCount, char **argValues)
{
    TestFreeQueue();
    TestNode();

    TestsEnd();
}