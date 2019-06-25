# Paraglob 2
#### A fairly quick data structure for matching a string against a large list of patterns.

For example, given a list of patterns
```
{*og, do*, ca*, plant}
```
and an input string `dog`, paraglob will return
```
{*og, do*}
```
## How it works
For any pattern, there exist a set of sub-strings that a string must contain in
order for it to have any hope of matching against that pattern. We call these
meta-words. Here are some examples:

```
*og       -> |og|
dog*fish  -> |dog| |fish|
```

When a pattern is added to a Paraglob the pattern is stored and is split into
its meta-words. Those meta words are then added to an Aho-Corasick data
structure that can be found in `multifast-ac`.

When Paraglob is given a query, it first gets the meta-words contained in the
query using `multifast-ac`. Then, it builds a set of all patterns associated with
those meta-words and runs `fnmatch` on the query and those patterns. It finally
returns a vector of all the patterns that match.

## Installation
```
# ./configure && make && make test && make install
```

## How to use it
`paraglob-test` is a small
benchmarking script that takes three parameters: the number of patterns to
generate, the number of queries to perform, and the percentage generated of
patterns that will match.

As an example, running `paraglob-test 10000 50 50` will add 10,000 patterns,
perform 50 queries on them (of which 50% should match), and then return the
results.

## Inside Zeek
Paraglob is integrated with Zeek & provides a simple api inside of its
scripting language. In Zeek, paraglob is implemented as an
`OpaqueType` and its syntax closely follows other similar constructs
inside Zeek. A paraglob can only be instantiated once from a vector of
patterns and then only supports get operations which return a vector
of all patterns matching an input string. These patterns are different than
the `patttern` type in Zeek in that they are just strings. The syntax is as 
follows:

```
  local v = vector("*", "d?g", "*og", "d?", "d[!wl]g");

  local p = paraglob_init(v);

  print paraglob_match(p1, "dog");
```
out:
```
[*, *og, d?g, d[!wl]g]
```

## Notes
Paraglob can make queries very quickly, but does not build instantly. It takes
about 1.5 seconds to build for 10,000 items, 3 seconds for 20,000, and so on.
This is because of the time required to build the Aho-Corasick structure.
