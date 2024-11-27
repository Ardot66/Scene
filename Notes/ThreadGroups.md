# Thread Groups

## Goal

To create a system that manages multithreading on groups of nodes with a single shared mutex and condition.

## Ideas

| Index | Idea | Pros | Cons |
| --- | --- | --- | --- |
| 1 | Multithreading is controlled on each group by a single root node with the ThreadGroup component. This component defines a mutex, condition, mode, and thread count. When a thread wants to access a variable that may be invalidated by another thread at any moment, it will have to first lock the mutex, then check the group mode. If the mode aligns with what the thread wants to do (read or write), or the mode is none, then the thread takes control and does its thing. Otherwise, the thread will go into a cond-wait based on the ThreadGroup's condition. When the last thread finishes in the group, and the thread count becomes 0, the waiting threads will get a chance to take over. |   |   |