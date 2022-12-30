#include "base64.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring> // memcpy

// default implementations of feed_data
void pr::Base64::feed_data(std::uint8_t data) { m_data.push_back(data); }

void pr::Base64::feed_data(std::uint8_t const* data, std::size_t size)
{
    auto old_size = m_data.size();
    auto new_size = size + old_size;
    m_data.resize(new_size);
    std::memcpy(&m_data[old_size], data, size);
}

auto pr::Base64Encoder::encode() -> std::string
{
    auto m_data = data();
    std::uint8_t* d = m_data.data();
    std::size_t size = m_data.size();

    std::size_t blocks = size / 3UL; // number of 3 (8-bit) bytes blocks
    std::size_t remainder = size % 3UL;

    auto encoded_str = std::string {}; // FIXME: reserve sufficient space beforehand
    auto byte_pos = size_t { 0UL };

    for (auto block = 0UL; block < blocks; ++block) {
        encoded_str.push_back(nth_of_alphabet(d[byte_pos] >> 2));
        encoded_str.push_back(
            nth_of_alphabet(((d[byte_pos] & 0x03U) << 4) | (d[byte_pos + 1] >> 4)));
        encoded_str.push_back(
            nth_of_alphabet(((d[byte_pos + 1] & 0x0fU) << 2) | (d[byte_pos + 2] >> 6)));
        encoded_str.push_back(nth_of_alphabet(d[byte_pos + 2] & 0x3f));

        byte_pos += 3;
    }

    if (remainder == 0) {
        return encoded_str;
    }
    // do not increment byte_pos now
    encoded_str.push_back(nth_of_alphabet(d[byte_pos] >> 2));

    if (remainder == 2) {
        encoded_str.push_back(
            nth_of_alphabet(
                ((d[byte_pos] & 0x03U) << 4) | (d[byte_pos + 1] >> 4)));
        encoded_str.push_back(nth_of_alphabet((d[byte_pos + 1] & 0x0fU) << 2));
    } else {
        encoded_str.push_back(nth_of_alphabet((d[byte_pos] & 0x03U) << 4));
        encoded_str.push_back('=');
    }

    encoded_str.push_back('=');
    return encoded_str;
}

auto pr::Base64Decoder::decode() -> std::vector<uint8_t>
{
    auto m_data = data();
    auto byte_data = std::vector<uint8_t> {};
    byte_data.reserve(m_data.size());
    for (auto i : m_data) {
        auto pos = letter_index((char)i);
        if (pos == std::string::npos) {
            continue; // ignore garbage values
        }
        byte_data.push_back((uint8_t)pos);
    }

    std::uint8_t* d = byte_data.data();
    std::size_t size = byte_data.size();

    std::size_t blocks = size / 4UL;

    // FIXME: reserve sufficient space beforehand
    auto decoded_data = std::vector<uint8_t> {};
    auto byte_pos = size_t { 0UL };

    for (auto block = 0UL; block < blocks - 1; ++block) {
        decoded_data.push_back(
            (uint8_t)(d[byte_pos] << 2 | d[byte_pos + 1] >> 4));
        decoded_data.push_back(
            (uint8_t)((d[byte_pos + 1] & 0x0fU) << 4 | (d[byte_pos + 2] >> 2)));
        decoded_data.push_back(
            (uint8_t)((d[byte_pos + 2] & 0x03U) << 6 | d[byte_pos + 3]));

        byte_pos += 4;
    }

    if (m_data[size - 2] == padding_char()) {
        // 2 padding
        decoded_data.push_back(
            (uint8_t)(d[byte_pos] << 2 | d[byte_pos + 1] >> 4));

    } else if (m_data[size - 1] == padding_char()) {
        // 1 padding
        decoded_data.push_back(
            (uint8_t)(d[byte_pos] << 2 | d[byte_pos + 1] >> 4));
        decoded_data.push_back(
            (uint8_t)((d[byte_pos + 1] & 0x0fU) << 4 | (d[byte_pos + 2] >> 2)));

    } else {
        decoded_data.push_back(
            (uint8_t)(d[byte_pos] << 2 | d[byte_pos + 1] >> 4));
        decoded_data.push_back(
            (uint8_t)((d[byte_pos + 1] & 0x0fU) << 4 | (d[byte_pos + 2] >> 2)));
        decoded_data.push_back(
            (uint8_t)((d[byte_pos + 2] & 0x03U) << 6 | d[byte_pos + 3]));
        byte_pos += 4;
    }

    return decoded_data;
}
