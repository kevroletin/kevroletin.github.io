---
layout: post
title:  "Notes about recursion schemes"
date:   2018-05-10 00:00:00 +1000
categories: functional-programming notes
---

## Intro

I started this post as a set of notes, insights, and breadcrumbs for my future
self. There is a chance that I will improve it once I gain better insights or
examples.

Short motivation: Haskell is a cool language and it allows to abstract concepts
which are hard/impossible to abstract in other languages. In particular it
allows us to play with:
* open recursion
* recursion schemes.

## Preparation

### How fix works

Think about recursion: you can construct recursive data types. You can construct
recursive values. And you can introduce "loops" in your values:

```haskell
zeros = (0 :) zeros
```

Below is the same code using hand-written data types (note `List a` on both
sides of equation):

```haskell
data List a = Leaf | Node a (List a)
zeros = (Node 0) zeros
```

`Node 0` is a function of a type `List a -> List a` and it can be applied to its
result. This snippet also can be reformulated like this:

```haskell
f = Node 0
zeros = f zeros
```

If you try to evaluate it then you will obtain `zeros = f ( f ( f ( ... )))`. An
interesting thing happens here: f has Type `a -> a`, but `zeros` has type `a`.
If you think, why an arrow `->` has gone: once you called `f` function, it
returns `a`. So `zeros = f ( something )` makes sense because if you managed to
pass a right argument of type `a` to `f` then `zeros` will have type `a`. But if
`zeros` have type `a` then it is the right thing to pass as an argument to `f`
function. Check and mate.

That pattern `zeros = f ( f ( f ( ... )))` can be abstracted into more general
combinator:

```haskell
fix :: (a -> a) -> a
fix f = f (fix f)
```

And now:

```haskell
f = Node 0
zeros = fix f
```

In some sense, we factored out recursion into `fix` combinator.

My main intuition about `fix` is this: `fix` changes type of its argument from
`a -> a` to `a`. So it "removes" an arrow from the type.

#### Simple fix examples

There are other examples of function with type `a -> a`, for example `(+1)`. You
can infinitely apply it to its own result *(however it's not quite useful
because you will never obtain the final value and will not be able to pattern
match on intermediate values)*. `(0:)` from our original `zeros` definition also
counts.

Below are two more examples using open recursion *(see (1) about open
recursion)*. Note that `succGen` has a type `(t -> [t]) -> (t -> [t])` so, it
can be applied to itself.

```haskell
zeroGen f = 0 : f
zeros = fix zeroGen

succGen f n = n : f (n + 1)
succ = fix succGen

rangeGen f n m
  | n <= m    = n : f (n + 1) m
  | otherwise = []
range = fix rangeGen
```

Please note that `fix` function is not magical. If types make sense, that
doesn't mean that things will magically work. `fix (+1)` has a type of a
concrete value, but you can't obtain it because `fix (+1)` will never terminate.
Only things which produce new constructors *(so that you can pattern match on
them)* or functions with termination condition *(like rangeGen defined above)*
are useful with `fix`.

(1) Open recursion: recursive function calls something which it has received as
a parameter instead of directly calling itself. This approach allows extension
*(or decoration)* of a recursive function. Open recursion also reminds me of how
objects are constructed in Perl's OOP. In Perl, you don't "bless" a value to a
class which constructor was defined. Instead, you obtain a class name as a
parameter and bless the value to that type. This allows reuse of constructor
from child classes.

## How Fix work

In the previous section, we defined several functions and then factored
recursion out. Here we will do the same thing for types. Let's review our toy
example:

```haskell
data List a = Leaf | Node a (List a)
```

Here `List a` is used in its own definition. It is explicit recursion, let's
replace it with an open recursion:

```haskell
data ListF a n = Leaf | Node a n
```

But how we can turn this type into an "infinite" one which we saw previously?
Let's think, `ListF a` has kind `* -> *`, but we want `*`. Hmmm.. this reminds
me something that we did previously: we "removed arrow" from type `a -> a` by
using `fix`. Let's do similar for types:

```haskell
data Fix f = Fix { unFix :: f (Fix f) }
```

Here we need to use some indirection with `Fix` and `unfix`, but the idea is the
same:

```haskell
type List a = Fix (ListF a)
```

Mental evaluation of the `List a` type yields `Fix (ListF a (Fix (ListF a (Fix
(ListF a (..))))))`. Very similar to a value level `fix`.

Note that our `List` type has `Leaf` constructor which makes it possible to
terminate data structure. It's not a requirement for using `Fix` and for using
recursion schemes described below. Both folds and unfolds can work on infinite
data structures *(but computing length of the infinite list will not
terminate)*.

## Simple morphisms

### Folds

My intuition for thinking about recursive data types is a tree, so I will
occasionally use words like Node, children, etc., I hope it will make sense.

All types below define one step of collapsing *(or transforming)* a data
structure into a type `a`. The difference between them is how much information a
transformation step obtains.

Algebra receives the "body" of a current node without its children *(children
are collapsed into `a` values)*. RAlgebra additionally obtains original children
of the node. CVAlgebra uses the fact that `Fix` allows extension of original
data type by attaching additional information to every node. Using that
technique CVAlgebra receives its children annotated with the corresponding `a`
values. Pay attention to similarities in these types:

```haskell
type Algebra f a = f a -> a

type RAlgebra f a = f (Fix f, a) -> a

type CVAlgebra f a = f (Fix (Attr a f)) -> a

data Attr a f n = Attr a (f n)

attribute (Attr a _) = a
```

`cata` transforms original data structure (aka `Fix f`) into something else (aka
`a`) by using `Algebra` *(the simplest transformation step defined above)*. I
added types to describe how transformation happens:

```haskell
cata :: Functor f => Algebra f a -> Fix f -> a
cata f = f . fmap (cata f) . unFix
--       |   |               | f (Fix f)
--       |   | f a
--       | a
```

I defined `para` using `annotateCurr` helper. Witness that the type of
`annotateCurr` is the type of `cata` where a was replaced by (Fix f, a). Also
note that except for `wrap` helper, body of this function is identical to
`cata`, and it works in very similar manner:

```haskell
annotateCurr :: (Functor f) => RAlgebra f a -> Fix f -> (Fix f, a)
annotateCurr h t = wrap . fmap (annotateCurr h) $ unFix t
  where
    wrap x = (t, h x)

para :: Functor f => RAlgebra f c -> Fix f -> c
para h = snd . annotateCurr h
```

And here again: `annotate` is very similar to the previous function. In types
`(Fix f, a)` was replaced by `Attr a f`. `wrap` became slightly more complicated
because we need to "sandwich" annotations between `Fix` layers :)

```haskell
annotate :: Functor f => CVAlgebra f a -> Fix f -> Fix (Attr a f)
annotate h =  wrap . fmap (annotate h) . unFix
  where
    wrap x = Fix (Attr (h x) x)

histo :: Functor f => CVAlgebra f a -> Fix f -> a
histo h = attribute . unFix . annotate h
```

### Unfolds

Unfolds are about generating data structure from a seed. Coalgebra generates one
layer per call. RCoalgebra can "manually" generate a result and return it
instead of producing a new layer. CVCoalgebra can either generate a single new
layer as Coalgebra or it can produce a "cake" of several new layers.

```haskell
type Coalgebra f a = a -> f a

type RCoalgebra f a = a -> f (Either a (Fix f))

type CVCoalgebra f a = a -> f (Fix (Coattr a f))

data Coattr a f n = Automatic a
                  | Manual (f n)
                  deriving Functor

```

Note that all unfolds are very similar to each other:

```haskell
ana :: Functor f => Coalgebra f a -> a -> Fix f
ana f = Fix . fmap (ana f) . f

apo :: forall f a . Functor f => RCoalgebra f a -> a -> Fix f
apo f = Fix . fmap (either (apo f) id) . f

futu :: forall f a . Functor f => CVCoalgebra f a -> a -> Fix f
futu f = Fix . fmap go . f
  where
    go :: Fix (Coattr a f) -> Fix f
    go (Fix (Automatic a)) = futu f a
    go (Fix (Manual fa))   = Fix (fmap go fa)
```

### Refolds

`ana` takes seed and produces a tree. `cata` takes a tree and converts it into
something else. `hylo` takes seed and converts it into something else.
Basically, it is `cata` applied after `ana`:

```haskell
hylo :: Functor f => Algebra f a -> Coalgebra f b -> b -> a
hylo a c = a . fmap (hylo a c) . c
```

Let's generate an infinite list and convert in into an ordinary list using
`hylo`:

```haskell
data ListF a n = Node a n deriving Functor

type List a = Fix (ListF a)

gen n = Node n (n + 1)

toList (Node n xs) = n : xs

-- take 2 (hylo toList gen 0) == [0, 1]
```

### Effectful folds

TBD.

### Solving dynamic programming tasks

TBD.

## More morphisms, generalized morphisms

TBD.

### Links

* <https://bartoszmilewski.com/2017/12/29/stalking-a-hylomorphism-in-the-wild/>
* <http://blog.sumtypeofway.com/an-introduction-to-recursion-schemes/ (the whole series)>
* <https://jtobin.io/monadic-recursion-schemes>
* <http://comonad.com/reader/2009/recursion-schemes/>
* <https://www.schoolofhaskell.com/user/edwardk/recursion-schemes/catamorphisms>
* <https://hackage.haskell.org/package/recursion-schemes/docs/Data-Functor-Foldable.html>
* <http://www.cs.ox.ac.uk/people/jeremy.gibbons/publications/urs.pdf>
* <https://www.cs.cmu.edu/~tom7/papers/wang-murphy-recursion.pdf>
