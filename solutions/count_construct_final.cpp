#include <iostream>
#include <string>
#include <string_view>
#include <chrono>
#include <cstring>

auto maximum_odd_binary(std::string_view s) -> std::string
{
    // Count '1' (0x31) bytes 8-at-a-time via zero-byte detection trick
    size_t n = 0;
    const char* p = s.data();
    size_t len = s.size(), i = 0;

    for (; i + 8 <= len; i += 8) {
        uint64_t word;
        std::memcpy(&word, p + i, 8);
        uint64_t mask = word ^ 0x3131313131313131ULL;
        mask = (~mask & (mask - 0x0101010101010101ULL)) & 0x8080808080808080ULL;
        n += __builtin_popcountll(mask) >> 3;
    }
    for (; i < len; ++i) n += (p[i] == '1'); 
    if (n == 0) return "";

    std::string out;
    out.resize_and_overwrite(len, [n](char* buf, size_t sz) noexcept
    {
        std::memset(buf, '1', n - 1);
        std::memset(buf + n - 1, '0', sz - n);
        buf[sz - 1] = '1';
        return sz;
    });
    return out;
}

void test(std::string_view input, std::string_view expected)
{
    auto res = maximum_odd_binary(input);

    if (res == expected)
        std::cout << "PASS | " << input << " -> " << res << "\n";
    else
        std::cout << "FAIL | " << input << " -> got: "
                  << res << " expected: " << expected << "\n";
}

double measure(std::string_view input, int repeat)
{
    volatile size_t dummy = 0;

    for (int i = 0; i < 100; ++i)
        dummy += maximum_odd_binary(input).size();

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < repeat; ++i)
        dummy += maximum_odd_binary(input).size();

    auto end = std::chrono::high_resolution_clock::now();

    auto total = std::chrono::duration<double, std::micro>(end - start).count();
    return total / repeat;
}

int main()
{
    test("0101", "1001");
    test("1110", "1101");
    test("0001", "0001");
    test("101010", "110001");

    std::cout << "\n--- Benchmark ---\n";
    std::string s(100000, '1');
    const int REPEAT = 1;
    for (int i = 0; i < 5; ++i)
    {
        double per_call = measure(s, REPEAT);
        std::cout << "Run " << i+1 << ": " << per_call << " us\n";
    }

    return 0;
}
