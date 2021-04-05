C++ Patricia trie
=================

A patricia trie is a type of radix tree which, instead of storing part of the
key in every node, only stores the bit where the two child keys differ.  This
reduces the storage requirement in each node, but requires an additional
comparison operation at the end of the search to confirm the right key was
found.

A patricia trie has characteristics somewhere between a binary tree and a
hash table.  It is faster than a red-black tree for storing text data, although
still much slower than a hash table, especially when cache locality is taken
into account, due to the number of pointer indirections required for lookups.
But unlike a hash table, the lexicographical order of the elements is preserved
and the trie can be iterated in-order.

Unlike binary trees, the patricia trie does not provide upper/lower bound
searches.  Instead, it provides a prefix match search, which returns the
longest stored value which is a prefix of the search key.  This makes it 
particularly suitable for storing CIDR prefixes in IP networking.

The patricia tree was originally described by Donald R. Morrison, in a slightly
different form than presented here, for use in text processing.


```
#include <sk/patricia.hxx>
```

Types:
* `patricia_trie<T>`
* `patricia_set<T>`
* `patricia_map<T>`

* [IP address database example](tests/ip_prefix.cxx)

Usage
-----

The easiest way to use the trie is with the container types `patricia_set<>`
and `patricia_map<>`, which work like STL containers:

```c++
sk::patricia_set<std::string> s;
s.insert("foo");
s.insert("bar");
s.insert("quux");

assert(s.find("foo") != s.end());

for (auto &&str: s)
    std::cout << s << '\n';

sk::patricia_map<std::string, int> m;
m["foo"] = 1;
m["bar"] = 42;
m["quux"] = 666;

for (auto &&item: s)
    std::cout << item.first << " = " << item.second << '\n';
```

### Creating keys

Operations on the trie require keys to be provided as a bitstring: an
arbitrary-length sequence of bits.  The utility class `patricia_key` is
provided to make creating and comparing these keys easier.

A `patricia_key` can be constructed from a span of bytes and an optional 
length in bits; if the length is not provided, it assumed to be equal to 
the length of the byte span multiplied by 8.  Constructors are also
provided for any type meeting the requirements of `std::ranges::contiguous_range`
and for C strings (`char const *)`.

```c++
std::vector<std::byte> byte_vec;
patricia_key key1(byte_vec);

std::string s;
patricia_key key3(s);

std::byte short_key = {0b11010000}; 
patricia_key key2(std::span(&short_key, 1), 4); // key is '1101'
```

### Key makers

To allow `patricia_set` and `patricia_map` to generate keys from values, they
require a _key maker_, which is similar to the comparator used with `std::set`
and `std::map`.  A key maker is a functor type which accepts the container's 
value type as an argument and returns a `patricia_key`, and is provided as the
second (for `patricia_set`) or third (for `patricia_map`) template argument.

For example, this keymaker could be used to store IPv6 addresses in a trie:

```c++
struct ip_prefix {
    in6_addr network;
    int length = 128;
};

struct ip_prefix_keymaker {
    auto operator()(ip_prefix const &addr) const -> sk::patricia_key {
        auto bytes = std::span(addr.network.s6_addr, sizeof(addr.network));
        return sk::patricia_key(bytes, addr.length);
    }
};

patricia_map<ip_prefix, std::string, ip_prefix_keymaker> ip_map;
```

### Key byte order

The trie itself operates on arbitrary bit strings and does not impose any byte
order requirements on the key.  However, if you intend to iterate the trie in
order or perform prefix match operations, the keys should be in MSB (big-endian)
byte order, since the trie considers the most significant bit to be the start of
the bitstring.

For the sample `ip_prefix_keymaker` shown above, no byte swapping is required
since IP addresses are always stored in MSB order.

### Prefix searching

In additional to the usual container operations, the Patricia containers provide
a `prefix_match()` operation, which returns the longest key which is a substring
of the provided key.

For example:

```c++
patricia_set<std::string> set;

set.insert("foo");
set.insert("foobar");

set.prefix_match("foo"); // returns "foo"
set.prefix_match("foob"); // returns "foo"
set.prefix_match("foobar"); // return "foobar"
set.prefix_match("foobarbaz"); // returns "foobar"
set.prefix_match("fo"); // no match
```
