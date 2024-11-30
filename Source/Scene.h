#ifndef __SCENE__
#define __SCENE__

#include "pthread.h"
#include "ComponentObjects.h"

typedef struct SaveData SaveData;
typedef struct ComponentReference ComponentReference;
typedef struct InterfaceReference InterfaceReference;

struct SaveData
{

};

struct ComponentReference
{
    void *Object;
    ObjectComponentData *ComponentData;
};

struct InterfaceReference
{
    void *Object;
    ObjectInterfaceInstanceData *Interface;
};

INTERFACE_DECLARE(Saveable, 
    int (*Save)(ComponentReference self, SaveData *saveStack);
    int (*Load)(ComponentReference self, SaveData *saveStack);
)

INTERFACE_DECLARE(Readyable,
    int (*Ready)(ComponentReference self);
    int (*Exit)(ComponentReference self);
)

INTERFACE_DECLARE(INode, 
    ComponentReference *Parent;
    ComponentReference *MutexGroup;
    size_t *ChildCount;

    ComponentReference *(*GetChild)(ComponentReference self, const size_t index);
    void (*RemoveChild)(ComponentReference self, const size_t index);
    int (*AddChild)(ComponentReference self, const ComponentReference *child, size_t *indexDest);
    int (*InsertChild)(ComponentReference self, const size_t index, const ComponentReference *child);
)

COMPONENT_DECLARE(Node, 
    COMPONENT_IMPLEMENTS_DECLARE(Node, INode)
    COMPONENT_IMPLEMENTS_DECLARE(Node, Readyable),
    COMPONENT_USES_DECLARE(Node, Readyable)
    COMPONENT_USES_DECLARE(Node, Saveable),
    ComponentReference Parent;
    ComponentReference MutexGroup;
    
    size_t ChildCount;
    size_t ChildListLength;
    ComponentReference *Children;
)

enum MutexGroupMode {MUTEX_GROUP_MODE_NONE, MUTEX_GROUP_MODE_READ, MUTEX_GROUP_MODE_WRITE};

INTERFACE_DECLARE(IMutexGroup,
    void (*Lock)(ComponentReference self, const enum MutexGroupMode mode);
    void (*Unlock)(ComponentReference self);
)

COMPONENT_DECLARE(NodeMutexGroup,
    COMPONENT_IMPLEMENTS_DECLARE(NodeMutexGroup, Readyable)
    COMPONENT_IMPLEMENTS_DECLARE(NodeMutexGroup, IMutexGroup),
    ,
    pthread_mutex_t Mutex;
    pthread_cond_t Condition;
    enum MutexGroupMode Mode;
    size_t ActiveThreadCount;
)

int FreeQueueInit();
int FreeQueuePush(void *ptr);
void FreeQueueFlush();

#endif