# Save System

## Goal

To create a system that can save and load scenes deterministically, taking into account subscenes.

## Ideas

| Index | Idea | Pros | Cons |
| --- | --- | --- | --- |
| 1 | Saves are structured with three key sections, which are each saved as individual stacks, and then pushed into the same file sequentially. The first section is a list of all objects, as a list of a list of components. The second section is a list of all nodes, marked by an index to an object type, or a negative index to an external scene. The order of nodes in this list determines their index. The third list is a list of all node data. Node data is in the form \[index\], \[size\], \[data\], with the index referring to a specific node. Data in the node data list can override data in sub-scenes. | Should be relatively compact, and saving should be quick, with the most computationally expensive case being when figuring out subscene overrides. | Requires three seperate lists, which is somewhat inefficient. Saving everything in binary won't play well with most version management, like Git. |
| 2 | Adding on to (1), references to other scenes could be handled by a central 'catalogue' that assigns an ID to each scene that other scenes use to reference it. The catalogue should probably use hexadecimal or some other human-readable system by default so that version management is guaranteed to function. | References to other scenes would be very small, just a single number. If a scene is moved in the file system, only the reference in the catalogue will need to be updated, rather than every reference. | Doesn't account for scenes created outside of the current project. If scenes are created externally and imported, their IDs will not exist in the catalogue, and may conflict with existing scenes. |
| 3 | Potential improvement for (2), the catalogue could assign a completely random VERY large number as the ID, such as 128 bit, or even 256 bit. | Still works with just IDs, which is very efficient. | Should work with imported scenes most of the time, however, VERY rarely, scene IDs could conflict, leading to issues. There would need to be a system that handles conflicts, even if it doesn't work very well, it would need to at least protect scenes from corruption. |
| 4 | As an alternative to (2), instead of a catalogue, each resource file could begin with its unique ID (whether random or some other system), and when resources are loaded by the engine, they are automatically added to some kind of collection where resources can be found based on their ID. | References between resources would be very small. This would subvert the need for a catalogue, making the system more efficient. As a result, adding scenes created elsewhere would be far simpler, as they would not need to be added to the catalogue. | Conflicts between IDs would still need to be handled, and directories containing resources will have to be manually loaded to be added to the collection. |
| 5 | To make (1) work better with version management, it may be beneficial to set up some option that saves save files as hexadecimal, with newlines between chunks of data. This would be less efficient, but Git should be able to handle basic hexadecimal and newlines with no problem. This could also be handled by designing a special program that does this automatically for Git, using the gitattributes system. Base 64 might also be useful for better compression, but it is less efficient to compute base64. | Allows version management to properly generate scene diffs and perform merges. | Reduces data compression efficiency. Still not human readable, although it will be easier to view and edit that pure binary. |
| 6 | As an alternative to (2), each scene file could instead keep references as a file paths to other scene files. | This would remove the need for a catalogue and ID system, so conflicts would be impossible. | This would take far more storage space, especially for scenes with very nested file paths. If the file location of a scene changes, all other scenes will need to be checked for references, which will increase the difficulty of resolving scene file corruption and greatly increase the computation involved with moving files. |