#include "Receiver_UDP.h"

#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
struct Sender {
    SOCKET handle = INVALID_SOCKET;
    ~Sender() { if (handle != INVALID_SOCKET) closesocket(handle); }
};
} // namespace

int main() {
    try {
        Receiver_UDP receiver(0, 200, "127.0.0.1");
        require(receiver.Port() != 0, "Ephemeral port was not assigned.");
        Sender sender;
        sender.handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        require(sender.handle != INVALID_SOCKET, "Sender socket failed.");
        sockaddr_in target{};
        target.sin_family = AF_INET;
        target.sin_port = htons(receiver.Port());
        target.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        const auto send = [&](const std::string& bytes) {
            const int sent = sendto(sender.handle, bytes.data(), static_cast<int>(bytes.size()), 0,
                                    reinterpret_cast<const sockaddr*>(&target), sizeof(target));
            require(sent == static_cast<int>(bytes.size()), "Test send failed.");
        };

        for (const std::string& bytes :
             {std::string("hello"), std::string("a\0b\0", 4),
              std::string(65507, 'x'), std::string()}) {
            send(bytes);
            require(receiver.ReceiveBytes() == bytes, "Datagram was truncated or changed.");
            require(receiver.LastSize() == bytes.size(), "Incorrect byte count.");
        }

        send("longer-message");
        require(std::string(receiver.Receive()) == "longer-message", "Legacy API failed.");
        send("x");
        require(std::string(receiver.Receive()) == "x" && receiver.LastSize() == 1,
                "A shorter datagram retained bytes from the previous message.");

        bool busy_port_rejected = false;
        try { Receiver_UDP duplicate(receiver.Port(), 200, "127.0.0.1"); }
        catch (const std::runtime_error&) { busy_port_rejected = true; }
        require(busy_port_rejected, "An occupied port was not rejected.");

        bool timeout_reported = false;
        try { receiver.ReceiveBytes(); }
        catch (const std::runtime_error&) { timeout_reported = true; }
        require(timeout_reported && receiver.LastSize() == 0, "Timeout state was not reported.");
        std::cout << "Payload, NUL, size, empty datagram, stale-buffer, bind and timeout checks passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
