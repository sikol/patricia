/*
 * Copyright (c) 2021 SiKol Ltd.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <set>

#include <catch.hpp>

#include "sk/patricia.hxx"

auto gen_test_vector() -> std::set<std::string>
{
    std::set<std::string> ret;

    // NOLINTNEXTLINE - random engine with fixed seed
    std::default_random_engine engine(123456u);

    std::uniform_int_distribution<unsigned int> rand_itemlen(10, 50);
    std::uniform_int_distribution<int> rand_char(0, 255);

    unsigned nitems = 50000;

    for (unsigned i = 0; i < nitems; ++i) {
        unsigned len = rand_itemlen(engine);

        std::string item(len, 'X');
        std::ranges::generate(
            item, [&]() -> char { return char(rand_char(engine)); });

        ret.insert(std::move(item));
    }

    return ret;
}

TEST_CASE("benchmark patricia_set vs std::set")
{
    auto data = gen_test_vector();
    auto data2 = gen_test_vector();

    BENCHMARK("insert: set")
    {
        std::set<std::string> set;
        for (auto &&string : data)
            set.insert(string);
        return set.empty();
    };

    BENCHMARK("insert: patricia_set")
    {
        sk::patricia_set<std::string> set;
        for (auto &&string : data)
            set.insert(string);
        return set.empty();
    };

    std::set<std::string> set;
    for (auto &&string : data)
        set.insert(string);

    sk::patricia_set<std::string> pset;
    for (auto &&string : data)
        pset.insert(string);

    BENCHMARK("lookup: set")
    {
        unsigned i = 0;
        for (auto &&string : data)
            i ^= (*set.find(string))[0];
        for (auto &&string : data2)
            i ^= 1 + (set.find(string) == set.end());
        return i;
    };

    BENCHMARK("lookup: patricia_set")
    {
        unsigned i = 0;
        for (auto &&string : data)
            i ^= (*pset.find(string))[0];
        for (auto &&string : data2)
            i ^= 1 + (pset.find(string) == pset.end());
        return i;
    };
}
