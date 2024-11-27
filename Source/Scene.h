#ifndef __SCENE__
#define __SCENE__

#include "ComponentObjects.h"

typedef struct SaveStack SaveStack;
typedef struct TreeObjectRef TreeObjectRef;

struct SaveStack
{

};

struct TreeObjectRef
{
    void *Object;
    ComponentData *ComponentData;
};

INTERFACE_DECLARE(Saveable, 
    int (*Save)(void *object, const ComponentData *component, SaveStack *saveStack);
    int (*Load)(void *object, const ComponentData *component, SaveStack *saveStack);
)

COMPONENT_DECLARE(TreeObject, 
    COMPONENT_IMPLEMENTS_DECLARE(TreeObject, Saveable),
    COMPONENT_USES_DECLARE(TreeObject, Saveable),
    TreeObjectRef Parent;
    
    size_t ChildCount;
    size_t ChildListLength;

)

int FreeQueueInit();
int FreeQueuePush(void *ptr);
void FreeQueueFlush();

#endif