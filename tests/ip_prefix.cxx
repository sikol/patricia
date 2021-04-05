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

#include <stdexcept>
#include <string>

#include <catch.hpp>

/*
 * Example of storing IP address prefixes in the patricia trie and using the
 * prefix_match() operation to retrieve the prefix for an IP address.
 */

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <windows.h>
#    include <winsock2.h>
#    include <ws2tcpip.h>
#else
#    include <sys/types.h>
#    include <sys/socket.h>
#    include <arpa/inet.h>
#    include <netinet/in.h>
#endif

#include "sk/patricia.hxx"

/*
 * An ip network and a prefix.
 */
struct prefix {
    in6_addr network{};
    int length = 128;

    prefix(char const *s) : prefix(std::string(s)) {} // NOLINT

    prefix(std::string s) // NOLINT
    {
        auto slash = s.find('/');

        if (slash != std::string::npos) {
            length = std::stoi(s.substr(slash + 1));
            s = s.substr(0, slash);
        }

        auto r = ::inet_pton(AF_INET6, s.c_str(), &network);
        if (r != 1)
            throw std::invalid_argument("invalid address: " + s);
    }
};

auto operator==(prefix const &a, prefix const &b)
{
    return std::memcmp(
               a.network.s6_addr, b.network.s6_addr, sizeof(a.network)) == 0 &&
           a.length == b.length;
}

auto operator<<(std::ostream &strm, prefix const &addr) -> std::ostream &
{
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, addr.network.s6_addr, buf, sizeof(buf));
    strm << buf;
    strm << '/' << addr.length;
    return strm;
}

// A keymaker for IP addresses.
struct prefix_keymaker {
    auto operator()(prefix const &addr) const -> sk::patricia_key
    {
        // in6_addr is already in MSB order.
        auto bytes = std::span(addr.network.s6_addr, sizeof(addr.network));
        return sk::patricia_key(bytes, addr.length);
    }
};

/*
 * A database of prefixes that can be searched by address.
 */
class prefix_set {
    sk::patricia_set<prefix, prefix_keymaker> prefixes;

public:
    auto lookup(prefix const &addr) -> prefix
    {
        auto it = prefixes.prefix_match(addr);

        if (it == prefixes.end())
            throw std::domain_error("prefix not found");

        return *it;
    }

    auto store(prefix const &addr) -> bool
    {
        return prefixes.insert(addr).second;
    }
};

TEST_CASE("IP prefix database")
{
    prefix_set db;

    db.store("::1/128");
    db.store("3ffe::/16");
    db.store("2000::/3");
    db.store("2001:db8::/32");
    db.store("2001:db8:1000::/48");
    db.store("2001:db8:1000::42/128");
    db.store("2001:db8:1000::/51");

    // Find prefixes by exact match.
    REQUIRE(db.lookup("::1") == prefix("::1"));
    REQUIRE(db.lookup("2000::/3") == prefix("2000::/3"));
    REQUIRE(db.lookup("2001:db8::/32") == prefix("2001:db8::/32"));
    REQUIRE(db.lookup("2001:db8:1000::/48") == prefix("2001:db8:1000::/48"));
    REQUIRE(db.lookup("2001:db8:1000::42/128") ==
            prefix("2001:db8:1000::42/128"));

    // Find prefixes by string match.
    REQUIRE(db.lookup("2000::1") //
            == prefix("2000::/3"));

    REQUIRE(db.lookup("2a02:1234:fedc::1") //
            == prefix("2000::/3"));

    REQUIRE(db.lookup("2001:db8:1000::43") //
            == prefix("2001:db8:1000::/51"));

    REQUIRE(db.lookup("2001:db8:1000::ffff:1") //
            == prefix("2001:db8:1000::/51"));

    REQUIRE(db.lookup("2001:db8:1000:2000::1") //
            == prefix("2001:db8:1000::/48"));

    REQUIRE(db.lookup("2001:db8:2000::1") //
            == prefix("2001:db8::/32"));

    REQUIRE(db.lookup("2001:db8:1003::1") //
            == prefix("2001:db8::/32"));

    REQUIRE(db.lookup("3ffe::1") //
            == prefix("3ffe::/16"));
}