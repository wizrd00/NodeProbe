# NodeProbe API Reference

NodeProbe is used by including a single header and linking against the shared library.

```c
#include "nodeprobe.h"
```

### How to link it?
It is possible to build the library with *pcc* or *gcc*.  
It also possible to link the library with *musl* or *glibc*.  
The Makefile detects the libc automatically.

The second part of library name indicates its libc.  
if the name is *libnodeprobe-musl.so* it means you can only use it with musl libc.  
The same think happens to *libnodeprobe-glibc.so*

#### Linking with *pcc*
```sh
pcc main.c -Wl,--library-path=./,--library=nodeprobe-musl,-rpath=./ -o main
# or if the name is libnodeprobe-glibc.so
pcc main.c -Wl,--library-path=./,--library=nodeprobe-glibc,-rpath=./ -o main
```

#### Linking with *gcc*
```sh
gcc main.c -L./ -lnodeprobe-musl -Wl,-rpath=./ -o main
# or if the name is libnodeprobe-glibc.so
gcc main.c -L./ -lnodeprobe-glibc -Wl,-rpath=./ -o main
```
