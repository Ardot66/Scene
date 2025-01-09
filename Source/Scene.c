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
    void *Object;
    ObjectData *ObjectData;
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
    freeQueueObject.Object = object;
    freeQueueObject.ObjectData = objectData;

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

        if(freeQueueObject.ObjectData != NULL)
        {
            ObjectInterfaceData *objectReadyableInterface = ObjectGetInterface(freeQueueObject.ObjectData, TYPEOF(IReadyable));

            for(size_t x = 0; x < objectReadyableInterface->ImplementingComponentsCount && result == 0; x++)
            {
                ObjectInterfaceInstanceData *readyableInterfaceInstance = objectReadyableInterface->ImplementingComponents + x;
                IReadyable *readyable = readyableInterfaceInstance->VTable;

                if(readyable->Exit == NULL)
                    continue;

                result = readyable->Exit(CRef(freeQueueObject.Object, readyableInterfaceInstance->Component));
            }
        }

        free(freeQueueObject.Object);

        if(result)
            return result;
    }
    pthread_mutex_unlock(&FreeQueueMutex);

    return result;
}

INTERFACE_DEFINE(ISaveable, )
INTERFACE_DEFINE(IReadyable, )

int NodeInitialize(ComponentReference component)
{
    Node *node = CRefComponent(component);

    node->ChildCount = 0;
    node->ChildListLength = 0;
    node->Children = NULL;

    const ObjectComponentData *mutexGroup = COMPONENT_GET_USE(component.Component, Node, MutexGroup)->Component;
    
    if(mutexGroup == NULL)
    {
        Node *parent = POINTER_OFFSET(node->Parent.Object, node->Parent.Component->Offset);
        node->MutexGroup = parent->MutexGroup;
    }
    else
        node->MutexGroup = CRef(component.Object, mutexGroup);

    return 0;
}

int NodeExit(ComponentReference component)
{
    Node *node = CRefComponent(component);
    free(node->Children);

    return 0;
}

int NodeAddChild(ComponentReference component, ComponentReference child, size_t *indexDest)
{
    Node *node = CRefComponent(component);
    size_t addIndex = node->ChildCount;

    for(size_t x = 0; x < node->ChildCount; x++)
    {
        ComponentReference child = node->Children[x];
        if(child.Object == NULL)
        {
            addIndex = x;
            break;
        }
    }

    *indexDest = addIndex;
    return ArrayInsert(
        (void **)&node->Children,
        &node->ChildCount,
        &node->ChildListLength,
        sizeof(*node->Children),
        addIndex,
        &child
    );
}

void NodeRemoveChild(ComponentReference component, size_t index)
{
    Node *node = CRefComponent(component);
    node->Children[index] = CRef(NULL, NULL);

    if(index == node->ChildCount - 1)
        while(node->Children[node->ChildCount - 1].Object == NULL)
            node->ChildCount--;
}

int NodeInsertChild(ComponentReference component, ComponentReference child, size_t index)
{
    Node *node = CRefComponent(component);

    if(index < node->ChildCount)
    {
        if(node->Children[index].Object != NULL)
            return EINVAL;

        node->Children[index] = child;
    }
    else
    {
        const size_t oldChildCount = node->ChildCount;

        ArrayInsert(
            (void **)&node->Children,
            &node->ChildCount,
            &node->ChildListLength,
            sizeof(*node->Children),
            index,
            &child
        );

        for(int x = oldChildCount; x < index; x++)
            node->Children[x] = CRef(NULL, NULL);
    }
}

COMPONENT_DEFINE(Node,
    COMPONENT_IMPLEMENTS_DEFINE(IReadyable,
        .Initialize = NodeInitialize,
        .Exit = NodeExit
    ),
    USES_DEFINE(IReadyable)
    USES_DEFINE(ISaveable)
    USES_DEFINE(MutexGroup)
)

int MutexGroupInitialize(ComponentReference component)
{
    MutexGroup *mutexGroup = CRefComponent(component);

    mutexGroup->ActiveThreadCount = 0;
    mutexGroup->Mode = MUTEX_GROUP_MODE_NONE;
    mutexGroup->Mutex = PTHREAD_MUTEX_INITIALIZER;
    mutexGroup->Condition = PTHREAD_COND_INITIALIZER;

    int result;

    if(result = pthread_mutex_init(&mutexGroup->Mutex, NULL)) return result;
    if(result = pthread_cond_init(&mutexGroup->Condition, NULL)) return result;

    return 0;
}

int MutexGroupExit(ComponentReference component)
{
    MutexGroup *mutexGroup = CRefComponent(component);

    pthread_mutex_destroy(&mutexGroup->Mutex);
    pthread_cond_destroy(&mutexGroup->Mutex);
    return 0;
}

void MutexGroupLock(ComponentReference component, const enum MutexGroupMode mode)
{
    MutexGroup *mutexGroup = CRefComponent(component);
    pthread_mutex_lock(&mutexGroup->Mutex);

    while(mutexGroup->Mode == MUTEX_GROUP_MODE_READ && mode != MUTEX_GROUP_MODE_READ || mutexGroup->Mode == MUTEX_GROUP_MODE_WRITE)
        pthread_cond_wait(&mutexGroup->Condition, &mutexGroup->Mutex);

    mutexGroup->Mode = mode;
    mutexGroup->ActiveThreadCount += 1;
    pthread_mutex_unlock(&mutexGroup->Mutex);
}

void MutexGroupUnlock(ComponentReference component)
{
    MutexGroup *mutexGroup = CRefComponent(component);
    pthread_mutex_lock(&mutexGroup->Mutex);

    mutexGroup->ActiveThreadCount--;
    if(mutexGroup->ActiveThreadCount == 0)
    {
        mutexGroup->Mode = MUTEX_GROUP_MODE_NONE;
        pthread_cond_signal(&mutexGroup->Condition);
    }

    pthread_mutex_unlock(&mutexGroup->Mutex);
}

COMPONENT_DEFINE(MutexGroup,
    COMPONENT_IMPLEMENTS_DEFINE(IReadyable, .Initialize = MutexGroupInitialize, .Exit = MutexGroupExit),
)