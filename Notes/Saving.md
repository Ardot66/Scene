Goal: To create a system that is able to save and load trees of nodes efficiently and with no data loss.

## Ideas

| Num | Concept | Pros | Cons |
| --- | --- | --- | --- |
| 1 | Save stack that goes through the tree from leafs to root and saves all data to a large contiguous stack. Two passes are probably needed to first store object data, then store tree ordering. | Memory contiguity would make it trivial to save a stack to file. | Execution on all write threads would need to be paused while this system executes, due to the need for two passes to aquire heirarchy data and object data seperately, which could break if objects are added or removed during this time. |
| 2 | Modification of (1) that utlilizes two stacks instead of one. One stack would store information for individual components, and the other would store data regarding tree ordering. The system would go down to the bottom of a branch, then save the lowest object's data to both stacks, with data regarding children in one, and data about the object itself in another. | Allows loading of tree heirarchy and individual objects in seperate passes, which makes aquiring objects from paths much easier on load. | The use of a stack would make it very difficult to introduce some kind of 'subscene' system, where branches of the tree are compressed by using an existing scene as a template. |
| 3 | Adding on to (2), this system would also need to have a seperate, unique list of all types of objects (as a list of ObjectData \*), which is later flattened down to a list of a list of component IDs that are used to reconstitute objects on load. This might involve adding yet another list of references to object types for each object. | Not much to say here, this will be required for any save system. |   |
| 4 |   |   |   |
| Mockup: |   |   |   |

\[start\] \[list of object data\] \[tree heirarchy\] \[

|   |   |