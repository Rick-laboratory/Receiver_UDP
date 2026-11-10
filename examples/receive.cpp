#include "Receiver_UDP.h"

#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace {
unsigned parse_number(std::string_view text) {
    unsigned value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
        throw std::invalid_argument("Port and timeout must be unsigned integers.");
    }
    return value;
}
} // namespace

int main(int argc, char* argv[]) {
    if (argc == 2 && (std::string_view(argv[1]) == "--help" ||
                     std::string_view(argv[1]) == "-h")) {
        std::cout << "Usage: udp-receive [port] [timeout-ms] [bind-IPv4]\n"
                     "Defaults: port 9009, 30000 ms, 0.0.0.0. Receive one datagram.\n"
                     "Port 0 chooses an ephemeral port; timeout 0 waits indefinitely.\n";
        return 0;
    }
    if (argc > 4) {
        std::cerr << "Usage: udp-receive [port] [timeout-ms] [bind-IPv4]\n";
        return 2;
    }
    try {
        const unsigned port = argc >= 2 ? parse_number(argv[1]) : 9009;
        const unsigned timeout = argc >= 3 ? parse_number(argv[2]) : 30000;
        if (port > 65535) throw std::invalid_argument("Port must be between 0 and 65535.");
        Receiver_UDP receiver(static_cast<std::uint16_t>(port), timeout,
                              argc == 4 ? argv[3] : "0.0.0.0");
        std::cout << "Waiting on port " << receiver.Port() << "...\n" << std::flush;
        const std::string bytes = receiver.ReceiveBytes();
        std::cout << "Received " << bytes.size() << " bytes.\n";
        // Display the legacy Broadcast_UDP terminator as framing, not text.
        const std::size_t text_size = !bytes.empty() && bytes.back() == '\0'
            ? bytes.size() - 1 : bytes.size();
        std::cout.write(bytes.data(), static_cast<std::streamsize>(text_size));
        std::cout << '\n';
        return 0;
    } catch (const std::invalid_argument& error) {
        std::cerr << error.what() << '\n';
        return 2;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
