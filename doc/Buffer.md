# buffer.h (is a cove.h)

## Public

### Public Class Members

```cpp
std::string alias;
```
An alias associated with this buffer.


```cpp
std::vector<triton::engines::symbolic::SharedSymbolicVariable> vars;
```
The symbolic variables for the bytes of this buffer.

### Public Functions

#### Constructors

```cpp
Buffer(std::string id, triton::uint64 addr, triton::uint64 ptr, size_t len);
```
- `id`: Identifying name created the buffer (i.e. malloc, calloc, etc)
- `addr`: Address that the buffer was created.
- `ptr`: Address of the buffer
- `len`: Size of the buffer


```cpp
bool kill(triton::uint64 addr);
```
Mark a Buffer as dead.
- `addr`: Point at which the buffer is killed.
Returns true if the buffer was not previously dead.


```cpp
bool stat();
```
Check the liveliness of a Buffer.
Returns true if the buffer is alive, else false.


```cpp
triton::uint64 getSink();
```
Get the last address where the buffer state was changed.
Returns the buffer's sink.


```cpp
triton::uint64 getOrigin();
```
Get the address where the buffer was originally created.
Returns the buffer's origin.


## Protected

### Protected Class Members

```cpp
enum BufferState {
    Live,
    Dead,
};
BufferState state;
```
A Buffer can be live or dead (allocated or free'd)



```cpp
triton::uint64 sink
```
The last address that the buffer state changed.


```cpp
triton::uint64 origin
```
The address that the buffer was created.
