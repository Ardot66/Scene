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