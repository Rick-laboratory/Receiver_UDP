#include "Receiver_UDP.h"

#include <ws2tcpip.h>
#include <stdexcept>

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

namespace {
std::runtime_error socket_error(const char* operation) {
    return std::runtime_error(std::string(operation) + " failed: " +
                              std::to_string(WSAGetLastError()));
}
} // namespace

Receiver_UDP::Receiver_UDP(std::uint16_t port, DWORD timeout_ms,
                          const std::string& bind_address) {
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    if (inet_pton(AF_INET, bind_address.c_str(), &local.sin_addr) != 1) {
        throw std::invalid_argument("The bind address must be a numeric IPv4 address.");
    }

    WSADATA data{};
    const int startup = WSAStartup(MAKEWORD(2, 2), &data);
    if (startup != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(startup));
    }
    try {
        socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_ == INVALID_SOCKET) throw socket_error("socket");
        if (timeout_ms != 0 &&
            setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms)) == SOCKET_ERROR) {
            throw socket_error("setsockopt(SO_RCVTIMEO)");
        }
        if (bind(socket_, reinterpret_cast<const sockaddr*>(&local), sizeof(local)) == SOCKET_ERROR) {
            throw socket_error("bind");
        }
        int length = sizeof(local);
        if (getsockname(socket_, reinterpret_cast<sockaddr*>(&local), &length) == SOCKET_ERROR) {
            throw socket_error("getsockname");
        }
        port_ = ntohs(local.sin_port);
    } catch (...) {
        if (socket_ != INVALID_SOCKET) closesocket(socket_);
        WSACleanup();
        throw;
    }
}

Receiver_UDP::~Receiver_UDP() {
    closesocket(socket_);
    WSACleanup();
}

void Receiver_UDP::receive_datagram() {
    received_size_ = 0;
    buffer_[0] = '\0';
    const int received = recv(socket_, buffer_.data(), static_cast<int>(buffer_.size() - 1), 0);
    if (received == SOCKET_ERROR) throw socket_error("recv");
    received_size_ = static_cast<std::size_t>(received);
    buffer_[received_size_] = '\0';
}

std::string Receiver_UDP::ReceiveBytes() {
    receive_datagram();
    return {buffer_.data(), received_size_};
}

char* Receiver_UDP::Receive() {
    receive_datagram();
    return buffer_.data();
}


