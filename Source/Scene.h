#ifndef __SCENE__
#define __SCENE__

#include "pthread.h"
#include "ComponentObjects.h"

typedef struct SaveData SaveData;
typedef struct ObjectRef ObjectRef;

struct SaveData
{

};

struct ObjectRef
{
    void *Object;
    ComponentData *ComponentData;
};

INTERFACE_DECLARE(Saveable, 
    int (*Save)(void *object, const ComponentData *component, SaveData *saveStack);
    int (*Load)(void *object, const ComponentData *component, SaveData *saveStack);
)

INTERFACE_DECLARE(Readyable,
    int (*Ready)(void *object, const ComponentData *component);
    int (*Exit)(void *object, const ComponentData *component);
)

COMPONENT_DECLARE(Node, 
    ,
    COMPONENT_USES_DECLARE(Node, Readyable)
    COMPONENT_USES_DECLARE(Node, Saveable),
    ObjectRef Parent;
    ObjectRef ThreadGroup;
    
    size_t ChildCount;
    size_t ChildListLength;
    ObjectRef *Children;
)

enum ThreadGroupMode {THREAD_GROUP_MODE_NONE, THREAD_GROUP_MODE_READ, THREAD_GROUP_MODE_WRITE};

COMPONENT_DECLARE(ThreadGroup,
    COMPONENT_IMPLEMENTS_DECLARE(ThreadGroup, Readyable),
    ,
    pthread_mutex_t Mutex;
    pthread_cond_t Condition;
    enum ThreadGroupMode Mode;
    size_t ActiveThreadCount;
)

int FreeQueueInit();
int FreeQueuePush(void *ptr);
void FreeQueueFlush();

#endif