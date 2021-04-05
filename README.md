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
longest stored values which is a prefix of the search key.  This makes it 
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

Documentation: TODO.

* [IP address database example](tests/ip_prefix.cxx)
