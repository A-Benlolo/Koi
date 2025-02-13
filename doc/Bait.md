# bait.h

## alloc.h

```cpp
triton::uint64 koi_calloc(Swimmer *s, triton::uint64 addr);
triton::uint64 koi_free(Swimmer *s, triton::uint64 addr);
triton::uint64 koi_malloc(Swimmer *s, triton::uint64 addr);
triton::uint64 koi_realloc(Swimmer *s, triton::uint64 addr);
```


## io.h

```cpp
triton::uint64 koi_fgets(Swimmer *s, triton::uint64 addr);
```

## string.h

```cpp
triton::uint64 koi_strcpy(Swimmer *s, triton::uint64 addr);
triton::uint64 koi_strlen(Swimmer *s, triton::uint64 addr);
triton::uint64 koi_strncpy(Swimmer *s, triton::uint64 addr);
```
