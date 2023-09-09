# TODO
## Buffer

### Utilities
* Flush out the `BufferReader` and `BufferWriter` classes.
* Add a `BufferSlice` class.
* Add a `BufferCursor` class.
### Organizational Changes
* Move the BufferHandle out of `buffer.h/cpp`.
* Centralize the various BufferDescriptor/Entry structs into `buffer.h/cpp`.
* Move Buffer and memory allocation classes into a subdirectory.
## EventLoop
* Create `slag/postal/event_loop.h/cpp`.