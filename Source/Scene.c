#include "Scene.h"
#include "Collections.h"
#include <stdlib.h>

#include <stdio.h>

pthread_mutex_t FreeQueueMutex = PTHREAD_MUTEX_INITIALIZER;
CArray FreeQueue = {.Body = NULL, .Count = 0, .Length = 0, .Offset = 0};

typedef struct FreeQueueObject FreeQueueObject;

// If objectData is null, this is just a pointer, otherwise it's an object pointer.
struct FreeQueueObject
{
    void *object;
    ObjectData *objectData;
};

int FreeQueueInit()
{
    return pthread_mutex_init(&FreeQueueMutex, NULL);
}

int FreeQueuePushPointer(void *ptr)
{
    return FreeQueuePushObject(ptr, NULL);
}

int FreeQueuePushObject(void *object, ObjectData *objectData)
{

    FreeQueueObject freeQueueObject;
    freeQueueObject.object = object;
    freeQueueObject.objectData = objectData;

    pthread_mutex_lock(&FreeQueueMutex);
    int result = CArrayInsert(&FreeQueue, sizeof(freeQueueObject), FreeQueue.Count, &freeQueueObject);
    pthread_mutex_unlock(&FreeQueueMutex);

    return result;
}

int FreeQueueFlush()
{
    int result = 0;

    const size_t freeQueueCount = FreeQueue.Count;

    pthread_mutex_lock(&FreeQueueMutex);
    for(size_t x = 0; x < freeQueueCount; x++)
    {
        FreeQueueObject freeQueueObject = *(FreeQueueObject *)CArrayGet(&FreeQueue, sizeof(freeQueueObject), 0);
        CArrayRemove(&FreeQueue, sizeof(freeQueueObject), 0);

        if(freeQueueObject.objectData != NULL)
        {
            ObjectInterfaceData *objectReadyableInterface = ObjectGetInterface(freeQueueObject.objectData, TYPEOF(IReadyable));

            for(size_t x = 0; x < objectReadyableInterface->ImplementingComponentsCount && result == 0; x++)
            {
                ObjectInterfaceInstanceData *readyableInterfaceInstance = objectReadyableInterface->ImplementingComponents + x;
                IReadyable *readyable = readyableInterfaceInstance->VTable;

                if(readyable->Exit == NULL)
                    continue;

                result = readyable->Exit(freeQueueObject.object, readyableInterfaceInstance->Component);
            }
        }

        free(freeQueueObject.object);

        if(result)
            return result;
    }
    pthread_mutex_unlock(&FreeQueueMutex);

    return result;
}

INTERFACE_DEFINE(ISaveable, )
INTERFACE_DEFINE(IReadyable, )

int Node_Initialize(void *object, ObjectComponentData *componentData)
{
    Node *node = POINTER_OFFSET(object, componentData->Offset);

    node->ChildCount = 0;
    node->ChildListLength = 0;
    node->Children = NULL;

    const ObjectComponentData *mutexGroup = COMPONENT_GET_USE(componentData, Node, MutexGroup)->Component;
    
    if(mutexGroup == NULL)
    {
        Node *parent = POINTER_OFFSET(node->Parent.Object, node->Parent.Component->Offset);
        node->MutexGroup = parent->MutexGroup;
    }
    else
        node->MutexGroup = (ComponentReference){.Object = object, .Component = mutexGroup};

    return 0;
}

int Node_Exit(void *object, ObjectComponentData *componentData)
{
    Node *node = POINTER_OFFSET(object, componentData->Offset);

    free(node->Children);

    return 0;
}

COMPONENT_DEFINE(Node,
    COMPONENT_IMPLEMENTS_DEFINE(IReadyable,
        .Initialize = Node_Initialize,
        .Exit = Node_Exit
    ),
    USES_DEFINE(IReadyable)
    USES_DEFINE(ISaveable)
    USES_DEFINE(MutexGroup)
)

int MutexGroup_Ready(void *object, ObjectComponentData *componentData)
{

}

int MutexGroup_Exit(void *object, ObjectComponentData *componentData)
{
    
}

COMPONENT_DEFINE(MutexGroup,
    COMPONENT_IMPLEMENTS_DEFINE(IReadyable, .Ready = MutexGroup_Ready, .Exit = MutexGroup_Exit),
)