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
        void *ptr = CArrayGet(&FreeQueue, sizeof(ptr), 0);
        CArrayRemove(&FreeQueue, sizeof(ptr), 0);

        free(ptr);
    }
    pthread_mutex_unlock(&FreeQueueMutex);
}

INTERFACE_DEFINE(Saveable)
INTERFACE_DEFINE(Readyable)

COMPONENT_DEFINE(Node,
    ,
    COMPONENT_USES_DEFINE(Readyable)
    COMPONENT_USES_DEFINE(Saveable)
)

int ThreadGroupReady(void *object, const ComponentData *component)
{

}

int ThreadGroupExit(void *object, const ComponentData *component)
{
    
}

COMPONENT_DEFINE(ThreadGroup,
    COMPONENT_IMPLEMENTS_DEFINE(Readyable, .Ready = ThreadGroupReady, .Exit = ThreadGroupExit),

)