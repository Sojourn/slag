# Service Design
## Lifecycle
Service instances are created and bound to a ServiceRegistry object in the domain before the EventLoop is created, and destroyed after the EventLoop.

## Types
### Service
The Service base class.

### ServiceType
An enumeration for each distinct service interface (Memory, System, Scheduler, etc.).

### ServiceInterface
This is a subclass of Service and is specialized for every ServiceType.

### ServiceRegistry
This has a collection of registered services that can be efficiently queried by type.
