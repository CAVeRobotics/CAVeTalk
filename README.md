# CAVeTalk

CAVEMAN controller message protocol

## Protocol

The CAVeTalk protocol is specifically designed to work with pub/sub frameworks, though it does not have to be used exclusively with pub/sub frameworks. A key string is generated alongside each message that can be used as a topic in pub/sub frameworks.

## Protobufs

[Protobufs](https://protobuf.dev/) are Google’s language-neutral, platform-neutral, extensible mechanism for serializing structured data. In this project, they are used to serialize message payloads.

### C/Embedded

When building the C version of this library and/or using this library on an embedded system, follow these steps to setup Protobufs:

1. Initialize and update submodules

   `git submodule update --init --recursive `
   - Use this command to see the progress of each submodule pull

     `git submodule update --init --recursive --progress`

2. Make the script to generate the Protobuf payloads for C with `nanopb` executable.

   `chmod +x tools/nanopb/generate.sh`

3. Run the `generate` script to generate the Protobuf payloads for C with `nanopb`. This step requires Python3 to be installed.

   `./tools/nanopb/generate.sh`

### C++

When building the C++ version of this library, follow these steps to setup Protobufs:

1. Initialize and update submodules

   `git submodule update --init --recursive`
   - Use this command to see the progress of each submodule pull

     `git submodule update --init --recursive --progress`

2. Navigate to the `protobuf` directory

   `cd external/protobuf`

3. Configure CMake build

   `cmake -S . -B _build -DCMAKE_INSTALL_PREFIX=_build/protobuf-install -DCMAKE_CXX_STANDARD=20 -G Ninja -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF -DABSL_PROPAGATE_CXX_STD=ON`

4. Build `protobufs`

   `cmake --build _build --config Release`

5. Install `protobufs` in `external/protobufs/_build/protobuf-install`

   `cmake --build _build -t install`

## Build

Prerequisites

- CMake >= 3.30

- Ninja

- C compiler that supports at least C11

- C++ compiler that supports at least C++20

- Python3 >= 3.9 with python3-venv installed

- Cppcheck (optional)

- Uncrustify (optional)

- Gcovr (optional)

1. Setup Protobufs for the version(s) of the library being built. See [Protobufs](#protobufs).

2. Configure CMake with a preset. Current presets include `Debug` and `Release`. To build with tests and generate coverage reports, use `Debug`.

   `cmake --preset <preset> -G Ninja`

3. Run the static analysis and code formatting tools if installed.
   - Cppcheck: `cmake --build --preset <preset> -t cppcheck`
   - Uncrustify: `cmake --build --preset <preset> -t uncrustify`

4. Build the project.

   `cmake --build --preset <preset>`

5. If the project was configured to build tests, run the tests.

   `cmake --build --preset Debug -t test`

6. If the project was configured to build tests and Gcovr is installed, generate a coverage report. The coverage report can be found in the `build/<preset>` directory at `coverage.html`.

   `cmake --build --preset Debug -t coverage-no-test` or `cmake --build --preset Debug -t coverage` to run the tests and generate the coverage report in a single command
