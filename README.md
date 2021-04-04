C++ Patricia trie
=================

A patricia trie is a type of radix tree which, instead of storing part of the
key in every node, only stores the bit where the two child keys differ. 
It is faster than a red-black tree for storing text data, but unlike a hash
table, the lexicographical order of the elements is preserved.

The patricia tree was originally described by Donald R. Morrison for use in
text processing; the implementation here is a slightly modified form which is
commonly used to store CIDR prefixes in IP networking.

```
#include <sk/patricia.hxx>
```

Types:
* `patricia_trie<T>`
* `patricia_set<T>`
* `patricia_map<T>`

Documentation: TODO.
