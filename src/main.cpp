#include "argparser.hpp"
#include "base64.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <iostream>

auto main(int argc, char** argv) -> int
{
    const char* input_filepath = nullptr;
    // TODO: add support for output to a file
    // const char* output_filepath = nullptr;
    bool decode = false;
    int wrap = 0;

    auto argparser = pr::ArgParser("Base64");

    argparser.add_option(input_filepath, "input file (default is STDIN)", "input", 'i');
    // TODO
    // argparser.add_option(output_filepath, "output file (defaults to stdout)", "output", 'o');
    argparser.add_option(decode, "decode, instead of encoding", "decode", 'd');
    argparser.add_option(wrap, "insert newline after VAL chars", "wrap", 'w');
    argparser.parse(argc, argv);

    size_t wrap_after = wrap > 0 ? (size_t)wrap : 0UL;

    // decode data from file
    if (decode && input_filepath != nullptr) {
        auto optional_val = pr::decode_file(input_filepath);
        if (!optional_val.has_value()) {
            std::cerr << "Error: Some error occured while reading/decoding the file '" << input_filepath << "'\n";
            return EXIT_FAILURE;
        }
        auto val = optional_val.value();
        for (auto i : val) {
            std::cout << (char)i;
        }
        std::cout << std::endl;

        return EXIT_SUCCESS;
    }

    // encode data from file
    if (!decode && input_filepath != nullptr) {
        auto optional_val = pr::encode_file(input_filepath, wrap_after);
        if (!optional_val.has_value()) {
            std::cerr << "Error: Some error occured while reading/encoding the file '" << input_filepath << "'\n";
            return EXIT_FAILURE;
        }
        auto val = optional_val.value();
        std::cout << val << std::endl;

        return EXIT_SUCCESS;
    }

    // read data from stdin

    // reopen stdin for reading binary data.
    // https://stackoverflow.com/a/1599093/19271034
    std::freopen(NULL, "rb", stdin);

    auto buff = std::vector<uint8_t> {};

    while (std::cin) {
        auto c = std::cin.get();
        if (c == EOF) {
            break;
        }
        buff.push_back((uint8_t)c);
    }

    // decode from stdin
    if (decode) {
        auto decoder = pr::Base64Decoder {};
        decoder.feed_data(std::move(buff));
        auto val = decoder.decode();
        for (auto i : val) {
            std::cout << (char)i;
        }
        std::cout << std::endl;

        return EXIT_SUCCESS;
    }

    // encode from stdin
    auto encoder = pr::Base64Encoder {};
    encoder.feed_data(std::move(buff));
    auto val = encoder.encode(wrap_after);
    std::cout << val << std::endl;

    return EXIT_SUCCESS;
}
