#ifndef __SCENE__
#define __SCENE__

#include "pthread.h"
#include "ComponentObjects.h"

typedef struct SaveData SaveData;
typedef struct ComponentReference ComponentReference;

struct SaveData
{

};

struct ComponentReference
{
    void *Object;
    ComponentData *ComponentData;
};

INTERFACE_DECLARE(Saveable, 
    int (*Save)(ComponentReference self, SaveData *saveStack);
    int (*Load)(ComponentReference self, SaveData *saveStack);
)

INTERFACE_DECLARE(Readyable,
    int (*Ready)(ComponentReference self);
    int (*Exit)(ComponentReference self);
)

COMPONENT_DECLARE(Node, 
    ,
    COMPONENT_USES_DECLARE(Node, Readyable)
    COMPONENT_USES_DECLARE(Node, Saveable),
    ComponentReference Parent;
    ComponentReference NodeMutexGroup;
    
    size_t ChildCount;
    size_t ChildListLength;
    ComponentReference *Children;
)

enum MutexGroupMode {MUTEX_GROUP_MODE_NONE, MUTEX_GROUP_MODE_READ, MUTEX_GROUP_MODE_WRITE};

INTERFACE_DECLARE(MutexGroup,
    void (*Lock)(ComponentReference self, enum MutexGroupMode mode);
    void (*Unlock)(ComponentReference self);
)

COMPONENT_DECLARE(NodeMutexGroup,
    COMPONENT_IMPLEMENTS_DECLARE(NodeMutexGroup, Readyable)
    COMPONENT_IMPLEMENTS_DECLARE(NodeMutexGroup, MutexGroup),
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