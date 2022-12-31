#include "base64.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring> // memcpy
#include <sstream>

// default implementations of feed_data
void pr::Base64::feed_data(std::uint8_t data) { m_data.push_back(data); }

void pr::Base64::feed_data(std::uint8_t const* data, std::size_t size)
{
    auto old_size = m_data.size();
    auto new_size = size + old_size;
    m_data.resize(new_size);
    std::memcpy(&m_data[old_size], data, size);
}

void pr::Base64::feed_data(std::vector<uint8_t>&& data)
{
    m_data = std::move(data);
}

auto pr::Base64Encoder::encode(size_t wrap_after) -> std::string
{
    auto m_data = data();
    std::uint8_t* d = m_data.data();
    std::size_t size = m_data.size();

    std::size_t blocks = size / 3UL; // number of 3 (8-bit) bytes blocks
    std::size_t remainder = size % 3UL;

    auto strm = std::ostringstream {};
    auto byte_pos = size_t { 0UL };
    auto buff = std::array<char, 4> { 0 };

    auto wrap_pos = size_t { 0UL };
    auto insert_chars = [&]() {
        // Insert chars into stream
        for (char c : buff) {
            strm << c;
            if (wrap_after > 0 && ++wrap_pos == wrap_after) {
                strm << '\n';
                wrap_pos = 0UL;
            }
        }
    };

    for (auto block = 0UL; block < blocks; ++block) {
        buff[0] = nth_of_alphabet(d[byte_pos] >> 2);

        buff[1] = nth_of_alphabet(
            (d[byte_pos] & 0x03U) << 4 | d[byte_pos + 1] >> 4);

        buff[2] = nth_of_alphabet(
            (d[byte_pos + 1] & 0x0fU) << 2 | d[byte_pos + 2] >> 6);

        buff[3] = nth_of_alphabet(d[byte_pos + 2] & 0x3f);

        insert_chars();
        byte_pos += 3;
    }

    if (remainder != 0) {
        buff[0] = nth_of_alphabet(d[byte_pos] >> 2);
        if (remainder == 2) {
            buff[1] = nth_of_alphabet(
                ((d[byte_pos] & 0x03U) << 4) | (d[byte_pos + 1] >> 4));
            buff[2] = (nth_of_alphabet((d[byte_pos + 1] & 0x0fU) << 2));
        } else {
            buff[1] = (nth_of_alphabet((d[byte_pos] & 0x03U) << 4));
            buff[2] = '=';
        }

        buff[3] = '=';
        insert_chars();
    }

    auto enc_str = strm.str();
    if (enc_str.back() == '\n') {
        enc_str.pop_back();
    }
    return enc_str;
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

auto pr::read_entire_file(const char* filepath) -> std::optional<std::vector<uint8_t>>
{
    auto inf = std::ifstream { filepath, std::ios_base::binary };
    if (!inf) {
        return {}; // error occured!
    }
    // determine file size
    auto fsize = inf.tellg();
    inf.seekg(0, std::ios_base::end);
    fsize = inf.tellg() - fsize;
    inf.seekg(0, std::fstream::beg);

    auto buff = std::vector<std::uint8_t> {};
    buff.resize((size_t)fsize); // reserve sufficient space
    inf.read((char*)buff.data(), fsize);

    assert(inf.gcount() == fsize); // ensure all data is read

    return buff;
}

auto pr::encode_file(const char* filepath, size_t wrap_after) -> std::optional<std::string>
{
    auto opt = pr::read_entire_file(filepath);
    if (!opt.has_value()) {
        return {}; // file can not be read
    }
    auto buff = opt.value();
    auto encoder = pr::Base64Encoder {};
    encoder.feed_data(std::move(buff));

    return encoder.encode(wrap_after);
}

auto pr::decode_file(const char* filepath) -> std::optional<std::vector<uint8_t>>
{
    auto opt = pr::read_entire_file(filepath);
    if (!opt.has_value()) {
        return {}; // file can not be read
    }
    auto buff = opt.value();
    auto decoder = pr::Base64Decoder {};
    decoder.feed_data(std::move(buff));

    return decoder.decode();
}
