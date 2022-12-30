#include "base64.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <iostream>

auto main() -> int
{
    std::freopen(NULL, "rb", stdin);
    auto encoder = pr::Base64Encoder {};
    auto decoder = pr::Base64Decoder {};

    while (std::cin) {
        auto c = std::cin.get();
        if (c != EOF) {
            encoder.feed_data((uint8_t)c);
        } else {
            break;
        }
    }
    auto encoded_data = encoder.encode();

    decoder.feed_data((uint8_t*)encoded_data.data(), encoded_data.size());
    auto decoded_data = decoder.decode();

    auto decoded_str = std::string {};
    for (auto i : decoded_data) {

        std::cout << (char)i;
    }
    std::cout << '\n';
    std::cout << "Encoded: " << encoded_data << '\n';

    return EXIT_SUCCESS;
}
