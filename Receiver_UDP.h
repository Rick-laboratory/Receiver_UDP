#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

class Receiver_UDP {
public:
    // Port zero asks Windows for an ephemeral port. Timeout zero waits indefinitely.
    explicit Receiver_UDP(std::uint16_t port = 9009, DWORD timeout_ms = 0,
                          const std::string& bind_address = "0.0.0.0");
    ~Receiver_UDP();
    Receiver_UDP(const Receiver_UDP&) = delete;
    Receiver_UDP& operator=(const Receiver_UDP&) = delete;

    // Preferred API: returns every received byte, including any NUL terminators.
    std::string ReceiveBytes();

    // Original C-string API, now terminated safely. Valid until the next receive.
    // Embedded NUL bytes require LastSize() or the preferred ReceiveBytes() API.
    char* Receive();
    std::size_t LastSize() const noexcept { return received_size_; }
    std::uint16_t Port() const noexcept { return port_; }

private:
    void receive_datagram();
    SOCKET socket_ = INVALID_SOCKET;
    std::uint16_t port_ = 0;
    std::array<char, 65508> buffer_{}; // Maximum IPv4 UDP payload plus a sentinel.
    std::size_t received_size_ = 0;
};

