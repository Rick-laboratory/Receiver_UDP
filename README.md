# UDP Receiver

A small Windows C++ class for receiving IPv4 UDP datagrams. It owns its socket and Winsock lifetime, checks errors, and keeps byte counts separate from C-string termination.

```cpp
#include "Receiver_UDP.h"

Receiver_UDP receiver(9009, 5000); // Port 9009; five-second receive timeout.
std::string bytes = receiver.ReceiveBytes();
```

## Build and run

Requirements: Windows, CMake 3.16+, and a C++17 compiler with Windows headers (MSVC or MinGW-w64).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

For MinGW-w64 with Ninja, add `-G Ninja`. Visual Studio executables are in `build/Release/`; Ninja executables are in `build/`.

```powershell
.\build\udp-receive.exe                         # Receive one datagram; 30-second timeout
.\build\udp-receive.exe 9009 5000 127.0.0.1      # Loopback only
.\build\udp-receive.exe --help
```

The executable reports the bound port, received byte count, and payload. It removes one trailing NUL only when displaying the text, matching the [Broadcast_UDP sender](https://github.com/Rick-laboratory/Broadcast_UDP). The library preserves all received bytes.

Exit codes: `0` on success, `1` for Winsock errors including timeout, `2` for invalid arguments.

## Interface

| Member | Behavior |
| --- | --- |
| `Receiver_UDP(port, timeout_ms, bind_address)` | Defaults to port `9009`, no timeout, all IPv4 interfaces |
| `ReceiveBytes()` | Returns an owning string containing the complete datagram, including embedded/trailing NUL bytes |
| `Receive()` | Original `char*` API, now always safely terminated; valid until the next receive call |
| `LastSize()` | Exact length of the last successful datagram; reset before a new receive |
| `Port()` | Bound port, including the actual ephemeral port when constructed with port `0` |

The receive buffer accepts the maximum IPv4 UDP payload of 65,507 bytes and reserves one extra byte for the compatibility API's terminator. A zero-byte datagram is valid. After a short message, old bytes are never exposed as part of the new C string.

Construction and receive errors throw exceptions. Objects are non-copyable. Calls on one receiver must be serialized; receive and destruction must not run concurrently.

## Code guide and validation

`Receiver_UDP.h` defines the ownership contract, `Receiver_UDP.cpp` contains checked Winsock operations, and `examples/receive.cpp` demonstrates argument handling and one receive operation. `SO_BROADCAST` is a sender option and is not needed to receive broadcasts.

The loopback test covers ordinary and embedded-NUL payloads, the maximum size, empty datagrams, long-then-short messages, occupied ports, and receive timeout. It binds only to `127.0.0.1` on an ephemeral port.

UDP can lose, duplicate, or reorder datagrams. Reception depends on network routing and firewall configuration; the library does not modify system settings.

## License

This repository does not currently include a license file.
