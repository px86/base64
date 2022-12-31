#pragma once

#include <cassert>
#include <cstdint>
#include <fstream>
#include <ios>
#include <optional>
#include <string>
#include <vector>

namespace pr {

class Base64;
class Base64Encoder;
class Base64Decoder;

using std::size_t;
using std::uint8_t;

class Base64 {
private:
    std::string const m_alphabet {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "+/"
        "="
    };
    std::vector<uint8_t> m_data {};

protected:
    // constructors with protected scope, so that only derived classes can
    // instantiate the variables in the base(super) class.
    Base64() = default;
    explicit Base64(std::string&& alphabet)
        : m_alphabet(std::move(alphabet))
    {
    }

    inline auto nth_of_alphabet(unsigned int n) -> char
    {
        assert(n < 65); // 64 digits + 1 for padding
        return m_alphabet.at(n);
    }

    inline auto letter_index(char c) -> size_t
    {
        return m_alphabet.find(c);
    }

    inline auto data() -> decltype(m_data)& { return m_data; }
    inline auto padding_char() -> char { return m_alphabet.at(m_alphabet.size() - 1); }

public:
    virtual void feed_data(uint8_t data);
    virtual void feed_data(const uint8_t* data, size_t size);
    virtual void feed_data(std::vector<uint8_t>&& data);
    virtual ~Base64() = default;
};

class Base64Encoder : public Base64 {
public:
    Base64Encoder() = default;
    explicit Base64Encoder(std::string alphabet)
        : Base64(std::move(alphabet)) {};
    [[nodiscard]] auto encode(size_t wrap_after = 0UL) -> std::string;
};

class Base64Decoder : public Base64 {
public:
    Base64Decoder() = default;
    explicit Base64Decoder(std::string alphabet)
        : Base64(std::move(alphabet)) {};
    [[nodiscard]] auto decode() -> std::vector<uint8_t>;
};

auto read_entire_file(const char* filepath) -> std::optional<std::vector<uint8_t>>;
auto encode_file(const char* filepath, size_t wrap_after = 0UL) -> std::optional<std::string>;
auto decode_file(const char* filepath) -> std::optional<std::vector<uint8_t>>;

} // namespace pr
