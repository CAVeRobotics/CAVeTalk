# CAVeTalk

CAVEMAN controller message protocol

## Protocol Structure

| Version | ID     | Length | Payload       | CRC     |
| ------- | ------ | ------ | ------------- | ------- |
| 1 Byte  | 1 Byte | 1 Byte | 0 - 255 Bytes | 4 Bytes |

1. Version refers to the communication version that packet adheres to, starting from V1 represented as 0x01
2. ID refers to the specific message being sent by the packet. Here is a list of the currently supported messages.

| ID   | Name                            | Descriptor                                                                                                                                             |
| ---- | ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 0x01 | Ooga                            | CAVeTalk Ping Message, should receive "Booga" back from the controller                                                                                 |
| 0x02 | Movement                        | Describes the Speed [m/s] & Turn Rate of the Rover [rad/s]                                                                                             |
| 0x03 | Camera Movement                 | Describes the Camera Pan [radians] and Tilt Servo Angles [radians]                                                                                     |
| 0x04 | Lights                          | Toggles the Onboard Headlights                                                                                                                         |
| 0x05 | Arm                             | Enable or disable driving and steering                                                                                                                 |
| 0x06 | Odometry                        | Describes the 3 relative acceleration axes [m/s^2], 3 gyroscopic axes [rad/s], 4 unit quaternion positions and 4 encoder values for each wheel [rad/s] |
| 0x07 | Logging                         | Send String Messages Between Devices                                                                                                                   |
| 0x08 | Configuration - Servos - Wheels | Send Wheel Servo Configuration Parameters Between Devices                                                                                              |
| 0x09 | Configuration - Servos - Cams   | Send Camera Servo Configuration Parameters Between Devices                                                                                             |
| 0x0A | Configuration - Motors          | Send Motor Configuration Parameters Between Devices                                                                                                    |
| 0x0B | Configuration - Encoders        | Send Encoder Configuration Parameters Between Devices                                                                                                  |
| 0x0C | Configuration - Log             | Send Log Verbosity Configuration Between Devices                                                                                                       |
| 0x0D | Configuration - Wheel Speed PID | Send PID Tuning Parameters for Rover Wheel Speed                                                                                                       |
| 0x0E | Configuration - Turn Rate PID   | Send PID Tuning Parameters for Rover Turn Rate                                                                                                         |
| 0x0F | Air Quality Readings            | Send Air Quality measurements like dust and gas content                                                                                                |

3. Length refers to the length of the packet in bytes
4. Payload refers to the main piece of information sent in the packet

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

- Gcovr (optional)

1. Setup Protobufs for the version(s) of the library being built. See [Protobufs](#protobufs).

2. See the `docs` directory for how to perform static analysis and code formatting. **You must set up Cppcheck and Uncrustify before configuring CMake.**

3. Configure CMake.

   `cmake -B build -G Ninja` or `cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCAVETALK_BUILD_TESTS=ON` to build with tests

4. Run the static analysis and code formatting tools.
   - Cppcheck: `cmake --build build -t cppcheck`
   - Uncrustify: `cmake --build build -t uncrustify`

5. Build the project.

   `cmake --build build`

6. If the project was configured to build tests, run the tests.

   `cmake --build build -t test`

7. If the project was configured to build tests and Gcovr is installed, generate a coverage report. The coverage report can be found in the `build` directory at `coverage.html`.

   `cmake --build build -t coverage-no-test` or `cmake --build build -t coverage` to run the tests and generate the coverage report in a single command
