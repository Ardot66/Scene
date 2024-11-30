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

INTERFACE_DEFINE(ISaveable)
INTERFACE_DEFINE(IReadyable)
INTERFACE_DEFINE(INode)

int Node_Initialize(void *object, ObjectComponentData *componentData)
{
    Node *node = POINTER_OFFSET(object, componentData->Offset);

    node->ChildCount = 0;
    node->ChildListLength = 0;
    node->Children = NULL;

    ObjectInterfaceData *mutexGroupInterface = ObjectGetInterface(componentData->ObjectData, TYPEOF(IMutexGroup));
    
    if(mutexGroupInterface == NULL)
    {
        void *parent = POINTER_OFFSET(node->Parent.Object, node->Parent.Interface->Component->Offset);
        INode *parentInterface = node->Parent.Interface->VTable;

        node->MutexGroup = *(InterfaceReference *)POINTER_OFFSET(parent, parentInterface->MutexGroup);
    }
    else
        node->MutexGroup = (InterfaceReference){.Object = object, .Interface = mutexGroupInterface->ImplementingComponents};

    return 0;
}

int Node_Exit(void *object, ObjectComponentData *componentData)
{
    Node *node = POINTER_OFFSET(object, componentData->Offset);

    free(node->Children);

    return 0;
}

COMPONENT_DEFINE(Node,
    COMPONENT_IMPLEMENTS_DEFINE(INode, 
        .Parent = (void *)offsetof(Node, Parent), 
        .MutexGroup = (void *)offsetof(Node, MutexGroup), 
        .ChildCount = (void *)offsetof(Node, ChildCount),
    )
    COMPONENT_IMPLEMENTS_DEFINE(IReadyable,
        .Initialize = Node_Initialize,
        .Exit = Node_Exit
    ),
    COMPONENT_USES_DEFINE(IReadyable)
    COMPONENT_USES_DEFINE(ISaveable)
)

INTERFACE_DEFINE(IMutexGroup)

int MutexGroup_Ready(void *object, ObjectComponentData *componentData)
{

}

int MutexGroup_Exit(void *object, ObjectComponentData *componentData)
{
    
}

COMPONENT_DEFINE(MutexGroup,
    COMPONENT_IMPLEMENTS_DEFINE(IReadyable, .Ready = MutexGroup_Ready, .Exit = MutexGroup_Exit)
    COMPONENT_IMPLEMENTS_DEFINE(IMutexGroup),
)