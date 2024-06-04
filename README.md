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
cmake --build . --target run
```

or

```
make run
```

> note: the client reads one file only and then creates its reversed copy and
> sends it as the second one.

> note2: to see a file parted in multiple packets you need to add content to
> the `build/file.txt` or create another and pass it to the client
