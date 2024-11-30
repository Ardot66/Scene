#include "Scene.h"
#include "Collections.h"
#include <stdlib.h>

pthread_mutex_t FreeQueueMutex = PTHREAD_MUTEX_INITIALIZER;
CArray FreeQueue = {.Body = NULL, .Count = 0, .Length = 0, .Offset = 0};

int FreeQueueInit()
{
    return pthread_mutex_init(&FreeQueueMutex, NULL);
}

int FreeQueuePush(void *ptr)
{
    pthread_mutex_lock(&FreeQueueMutex);
    int result = CArrayInsert(&FreeQueue, sizeof(ptr), FreeQueue.Count, &ptr);
    pthread_mutex_unlock(&FreeQueueMutex);

    return result;
}

void FreeQueueFlush()
{
    pthread_mutex_lock(&FreeQueueMutex);
    for(size_t x = 0; x < FreeQueue.Count; x++)
    {
        void *ptr = *(void **)CArrayGet(&FreeQueue, sizeof(ptr), 0);
        CArrayRemove(&FreeQueue, sizeof(ptr), 0);

        free(ptr);
    }
    pthread_mutex_unlock(&FreeQueueMutex);
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