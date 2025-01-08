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

    return readyable->Initialize(object, interfaceInstance->Component);
}

int ObjectExitCaller(void *object, const ObjectInterfaceInstanceData *interfaceInstance)
{
    IReadyable *readyable = interfaceInstance->VTable;
    if(readyable->Exit == NULL)
        return 0;

    return readyable->Exit(object, interfaceInstance->Component);
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

    result = ObjectCallInterfaceFunction(objectData, object, TYPEOF(IReadyable), ObjectExitCaller);
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