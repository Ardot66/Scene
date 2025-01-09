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
    const ObjectComponentData *Component;
};

struct InterfaceReference
{
    void *Object;
    const ObjectInterfaceInstanceData *Interface;
};

INTERFACE_DECLARE(ISaveable, ,
    int (*Save)(ComponentReference component, SaveData *saveStack);
    int (*Load)(ComponentReference component, SaveData *saveStack);
)

INTERFACE_DECLARE(IReadyable, ,
    // Initialize will start at the top of the tree and iterate down.
    int (*Initialize)(ComponentReference component);

    // Ready will start at the bottom of the tree and iterate up, this occurs after all Initialize calls
    int (*Ready)(ComponentReference component);
    int (*Exit)(ComponentReference component);
)

COMPONENT_DECLARE(Node,
    COMPONENT_IMPLEMENTS_DECLARE(Node, IReadyable),
    USES_DECLARE(Node, IReadyable)
    USES_DECLARE(Node, ISaveable)
    USES_DECLARE(Node, MutexGroup),
    ComponentReference Parent;
    ComponentReference MutexGroup;
    
    size_t ChildCount;
    size_t ChildListLength;
    ComponentReference *Children;
)

enum MutexGroupMode {MUTEX_GROUP_MODE_NONE, MUTEX_GROUP_MODE_READ, MUTEX_GROUP_MODE_WRITE};

COMPONENT_DECLARE(MutexGroup,
    COMPONENT_IMPLEMENTS_DECLARE(MutexGroup, IReadyable),
    ,
    pthread_mutex_t Mutex;
    pthread_cond_t Condition;
    enum MutexGroupMode Mode;
    size_t ActiveThreadCount;
)

static inline void *CRefComponent(ComponentReference component)
{
    return POINTER_OFFSET(component.Object, component.Component->Offset);
}

static inline ComponentReference CRef(void *object, const ObjectComponentData *componentData)
{
    return (ComponentReference){.Object = object, .Component = componentData};
}

static inline InterfaceReference IRef(void *object, const ObjectInterfaceInstanceData *interface)
{
    return (InterfaceReference){.Object = object, .Interface = interface};
}

int FreeQueueInit();
int FreeQueuePushPointer(void *ptr);
int FreeQueuePushObject(void *object, ObjectData *objectData);
int FreeQueueFlush();
int NodeAddChild(ComponentReference component, ComponentReference child, size_t *indexDest);
void NodeRemoveChild(ComponentReference component, size_t index);
int NodeInsertChild(ComponentReference component, ComponentReference child, size_t index);

#endif