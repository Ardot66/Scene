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

INTERFACE_DEFINE(Saveable)
INTERFACE_DEFINE(Readyable)
INTERFACE_DEFINE(INode)

int Node_Ready(ComponentReference self)
{
    
}

COMPONENT_DEFINE(Node,
    COMPONENT_IMPLEMENTS_DEFINE(INode, 
        .Parent = (void *)offsetof(Node, Parent), 
        .MutexGroup = (void *)offsetof(Node, MutexGroup), 
        .ChildCount = (void *)offsetof(Node, ChildCount),
    )
    COMPONENT_IMPLEMENTS_DEFINE(Readyable,
        .Ready = Node_Ready
    ),
    COMPONENT_USES_DEFINE(Readyable)
    COMPONENT_USES_DEFINE(Saveable)
)

INTERFACE_DEFINE(IMutexGroup)

int NodeMutexGroup_Ready(ComponentReference self)
{

}

int NodeMutexGroup_Exit(ComponentReference self)
{
    
}

COMPONENT_DEFINE(NodeMutexGroup,
    COMPONENT_IMPLEMENTS_DEFINE(Readyable, .Ready = NodeMutexGroup_Ready, .Exit = NodeMutexGroup_Exit)
    COMPONENT_IMPLEMENTS_DEFINE(IMutexGroup),
)