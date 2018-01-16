---
layout: post
title: "Pure functional programming for beginners"
date:   2018-01-17 00:00:00 +1000
categories: fun
---

## Step 1

Become familiar with Aaron Cairo's manner of speaking.

I highly recommend reading text from step 2 using one particular voice. So watch
at least 25 seconds of [this video](https://www.youtube.com/watch?v=p3NXd3DhH08)
to capture author's voice and his manner of speaking.

![]({{ "/assets/2018-01-17-how-to-functional-programming/functional-skater.png" | absolute_url }})

## Step 2

Read this text using the voice of Aaron Cairo *(safely skip to next section once
you became bored)*.

My name is XXX and I'm a professional functional programmer from the San
Francisco Bay Area. I've been programming using pure functions for over 20 years
and teaching people how to structure their computations for a very very long
time. Today I'm going to give you a breakdown of how to write functional
programs for beginners. Now keep in mind this is a beginners tutorial so we're
going to start out with the most basic basics it's figuring out how to combine
functions. You're going to combine functions it is one of the most important and
sometimes the most confused things for a beginner functional programmer.

I feel like people get too eager when they are very first starting out with the
functional programming and they just want to go straight into learning how to
use Monads. But it's very important to take it back to the basic basics and just
learn to apply functions around a ton. After you're really comfortable just
composing and decomposing straight with your functions we're going to learn how
to do more advanced techniques. This is called an `Applicative Functor`. There
are two basic ways to change it's content one is called a `fmap` which directly
maps its content using supplied function. Other is using sequential application
where you actually create a container with functions and apply one container to
another container. Here you need your previous imperative background a little
bit looser so you don't want to use objects because a function is the most fun
way to write programs.

That's also very important to learn just so you can get those down and you got
to remember these are the basic basic basics so it's very very important you
spend a really good amount of time just learning how to decompose the problem
into pieces and compose solution using functions. Getting very very comfortable
with your functions just computing around. When I was a kid we used to just get
on our functions and just compute down the arithmetic. We would apply the
function everywhere, call it to parse text. Apply it to your friend's toy
programming language right into the lexer. No matter what we were doing we
seemed to just always have our functions. So again I just want to make it very
very clear and very important that the more time you spend applying your
functions the more comfortable you're going to get with functional programming.
Then really anything you can do to get more comfortable with your function even
if it's just having it with you all the time to carry it then put it down and do
a couple calls and composing around. Anything that you do like that is going to
make you a better functional programmer. In the end, you can even do some very
very very beginner like silly tricks like currying. This put your function with
multiple parameters and call it using only first argument, you just obtained
another function. A very very simple basic technique that pretty much anyone can
learn right away. Once you get into doing harder techniques you're going to
start out in a very exact sequence. It's going to be `Lists`, `Functors`,
`Applicative` `Functors`, `Monads` and `File IO`. So start the educational
course and have an awesome time using functional programming for the rest of
your life.

## Are you crazy?

No, I am not crazy. Often learning techniques are transferable between knowledge
domains. And I just going to give you one good learning technique.

Since last year I have been learning skateboarding. As you can imagine,
skateboarding is about balance and balance can't be achieved solely by
consciousness. This is because when you are riding a skateboard things just
happen too fast. You need to train your brain and muscles to work automatically,
otherwise, you will fall. Falling is dangerous so if you want to stay healthy
and to skateboard, you need to obey Aeron's advice. Master fundamental
techniques and then move to advanced staff.

I found that Aaron is really good at teaching this simple idea that learning
should be incremental. This is because he has teaching experience and in case of
failure, his students obtain injuries _(which is probably worse than failure to
apply university knowledge in real life?)_.

Let's call the phrase "again I just want to make it very very clear and very
important that the more time you spend doing XXX the more comfortable you're
going to get with YYY" a learning mantra. I internalized the learning mantra and
apply it to force myself in different learning scenarios. For example, it can be
applied to motivate regular practice to master a musical instrument. Everyone
wants to play melodies but moving exercises, learning scale, rhythm training
will better your sound and will allow learning more advanced melodies.

![]({{ "/assets/2018-01-17-how-to-functional-programming/functional-thinking.png" | absolute_url }})

I have applied the learning mantra many times. However, it was an unexpected
discovery that I need to apply learning mantra to better my functional
programming skills! I have been sneaking ideas from Haskell and Scala for many
years. Immutability, property-based testing, [separation of pure code from
effects](https://gist.github.com/kbilsted/abdc017858cad68c3e7926b03646554e) are
great techniques and I have been successfully applying them in my day to day job
in C++, Java, and Groovy. I even tried reactive *(functional)* programming to
build a user interface in Java. But I never truly learned hardcore pure
functional programming *(aka Haskell)*.

Haskell is an expressive language, it has type inference which truly works
*(hello C++)*, it has several types of polymorphism *(read C++ templates, Java
generics and more, hello golang)*, easy template programming *(hello C++)* and
syntax capable of implementing eDSLs *(read libraries with very nice API)*. But
core language lacks objects with inheritance and dynamic dispatch *(aka virtual
functions)* and it looks like nobody cares about it. I thought to apply Haskell
to solve several "real life" problems but I rejected the idea because I wasn't
comfortable enough to design the whole solution using only functions. After all,
how can you build nontrivial software without objects, interfaces and dynamic
dispatch? It appears you can, but to do so you need to learn how to apply
functions.

Actually, I am not in position for teaching and I just want to share my
experience. Hardcore functional programming feels different from imperative
object-oriented programming. And to became productive with it you need to return
to basic basics and to spend some time just applying functions. If you are not
comfortable with functions then Haskell Standard library will look really silly
because it uses function composition in different sometimes unexpected ways.

I hope the learning mantra will help you to master functional programming or
some other skill. Thanks for reading and remember that functions are the most
fun way to write programs.

{% include disqus.html %}
