# A list of half-baked ideas

## Containers
A stronger TaskGroup with additional capabilities. These are groups of tasks, resources, and resource constraints which can nest.
### Locality
Hypothesis: entities within a container have both high spacial and temporal locality. They should be allocated/stepped as a group to take advantage. This means per-container pools and an executor barrier (new high/idle roots for the container that are children of the parent container's).
### Resource limits
Constrain the amount of and rate that resources are used.
### Nesting
Executing nested containers is O(depth). I don't anticipate deep hierarchies or this being a problem.

## Interval tree clock to track event causality
Need to convince myself that this is possible, but I think that you can use vector clocks to record and replay cluster executions ~perfectly (independent events may be reordered). Would be interesting for testing and debugging. The interval tree variant is a better fit for a dynamic environment (nodes do not have static lifetimes).
### Mirror the task tree
I think the interval tree is isomorphic with the cluster/process/thread/task hierarchy and we can use that to compress interval representations. Might be able to optimize clock updates too.

## Does the buffer sharing scheme work across process boundaries?
Sealed segments that can be allocated from. Offset pointers for
buffer storage?

Anonymity--doesn't leak information about memory being used by
other processes.
