# Reactor Design
## Lifecycle
### Startup
TODO

### Looping
TODO

### Shutdown
TODO

## Operations
An operation represents an asynchronous tree of system calls. Leaves of the tree are normal system calls like open, close, write, etc. Branches are Tasks that can wait on other operations including other branches, hence the tree shape.

### Primitive
These represent asynchronous system calls like open, close, send, etc. There are also io_uring specific ones that are used to manage io_uring specific resources like files and buffers.

### Composite
Composite operations are Tasks that combine multiple primitive or composite operations to make more convenient abstractions to work with. One example of this would be conjoined ::socket/::connect/::recvmsg calls. These could also be accomplished with linked io_uring requests if eBPF programs get support. That would end up looking a lot like shaders for IO potentially.

## Resources
Resources held by file descriptors and file indexes are tracked per-region in a FileTable class. The entry in the FileTable is referenced using a reference-counted FileHandle object. The FileTable will start a daemonized cleanup operation to unregister the file index if needed, and close the file descriptor.

**NOTE:** The designs of the FileTable and FileHandle classes are not thread-safe. You should duplicate file descriptors if they need to be used by multiple threads. There is a distinct file index namespace per-region as well, so it will need to be registered on each thread as well to take advantage of that feature.
