# Simple test udp client + server

### Prerequesities
- compiled with gcc v14.1 
- cmake v3.14
- OS: linux
- ninja (optional, for the faster build)

### How to build:

1. Clone the repo:
```
git clone https://github.com/barkinlove/testserver.git
```

2. Create build folder and go there
```
mkdir build && cd build
```

3. Generate build files and then build
```
cmake .. && cmake --build . --parallel $(nproc)
```

4. Run the server and the client
```
./bin/test_server &
./bin/test_client filename.txt
```

Or execute cmake target `run`

```
cmake --target run
```

> note: the client reads only one file and then creates its reversed copy and then 
> sends it as the second one.
