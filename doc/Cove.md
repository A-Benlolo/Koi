# cove.h

## Public

### Public Functions

#### Constructors

```cpp
Cove();
Cove(triton::uint64 a, size_t s);
```
Constructor a new Cove.
- `a`: Address of the cove.
- `s`: Size of the cove.
Returns a new `Cove` object.


#### Getters

```cpp
triton::uint64 getAddress();
```
Get the Cove start address


```cpp
triton::uint64 getSize();
```
Get the Cove size


## Protected

### Protected Class Members

```cpp
triton::uint64 addr;
```
Start address of the cove.


```cpp
size_t sz;
```
Size of the cove.
