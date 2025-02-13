# stackframe.h (is a cove.h)

## Public

### Public Functions

#### Constructors

```cpp
Stackframe();
Stackframe(triton::uint64 a, size_t s);
```
- `a`: Address of the stackframe
- `s` - Size of the stackframe

#### Updaters

```cpp

bool addAccess(triton::uint64 offs);
```
Add an access offset to a Stackframe
- `offs`: Offset of the access
Returns true if the access is new


```cpp
size_t getAccessGap(triton::uint64 offs);
```
Get the length until the next access.
- `offs`: Offset to start search from.
Returns the length until the next access.


```cpp
void update(triton::uint64 a, size_t s);
```
Update the stackframe information
- `a`: Address of the stackframe
- `s` - Size of the stackframe


## Private

### Private Class Members

```cpp
std::vector<triton::uint64> accesses;
```
Sorted vector of access offsets for the stackframe.
