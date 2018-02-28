---
layout: post
title:  "Blockchain overview: Bitcoin"
date:   2018-1-8 00:00:00 +1000
categories: blockchain
---

Bitcoin is a really well-explained topic. My first intention was to skip Bitcoin
and to dig straight into altcoins. But it turns out that there are people with a
technical background who heard about Bitcoin and probably even used it. But they
don't have enough time to read the [whole
book](http://chimera.labs.oreilly.com/books/1234000001802/index.html), to take
[online course](https://www.coursera.org/learn/cryptocurrency) or to lurk
through different blogs which describe how crypto will disrupt your future
without many technical details.

This isn't a hardcore technical article but just a light tough on how things
work. Please make yourself familiar with the article structure before reading. I
failed to make this post compact so it's possible to lose track of thoughts at
some point.

## Table of contents

* TOC
{:toc}

## Everyday usage

Before diving into technical details I will describe an ordinary day of an
ordinary Bitcoin user. There are several possible roles:
* a user with a light client
* a user with a full node
* a miner

If you are familiar with the subject then it's safe to jump to the [How things
work](#how-things-work) section. Probably it worth looking to "Highlights"
section of each story.

### Light client

#### Story

This user chose and installed a wallet software on his/her mobile phone or
his/her computer. A walled generated an account which appears to be a pair of
unreadable strings just like this `1BHdMQPzskL5WYcPMicRMhhnPesBz5pGnr` *(an
address)* and `L27WaECdFu8UvExuTBC7BpCbMYWZtPqtCuYbP6aSgeWFNeDb3qaM` *(a public
key)*. Those strings can be represented as QR codes for mistake-free usage with
mobile phones.

The user has sent his public key for a friend. And the friend sent few Satoshis
*(0.00000001 part of Bitcoin)* to that address in exchange for dollars. Later
the user used his private key and wallet software to send his digital money to
other people. Each new operation consumes a transaction fee which is suggested
by a wallet software. Transaction fees are optional but they directly affect
transaction time. Even with a good fee, it takes about 50 minutes for a
transaction to finish. It seems like today transactions don't commit without a
fee anymore. This is because there is a constant supply of transactions with
good fees and Bitcoin's throughput is limited.

Later, the user sold his car and invested into Bitcoins. To protect his
investment he generated a new account, printed private key on a paper and locked
it in his safe. He wiped the private key from the hard drive so there is no way
for hackers to steal his money. Such approach is called cold/hot wallets. The
cold one is in the safe. It's not intended for everyday use and it is secure.
The hot one is on the mobile phone. It's for everyday use but its security
suffers from software bugs, social engineering, and other hacking techniques.

#### Highlights

* A light client can work on a "weak" device like a mobile phone.
* There is no bootstrapping time for a light client. All you need is to install a
  software and to generate an account.
* An account is as secure as a private key.

### Full client

#### Story

Aka fully validating client. This user installed software, configured a firewall
to allow incoming connections and started bitcoind background process. A few
days later he/she found that bitcoind has occupied 120 GB of storage and it
constantly communicates over the network. But CPU and network usage aren't too
bad and storage consumption doesn't seem to grow too fast. The user tries to run
his computer 24/7 and occasional reboots don't seem to affect bitcoind behavior:
after reboot software catches up quickly.

Now the user can do all the same things that light client user can *(and even
more!)*. An additional feature of his investment is feeling of security. He
knows that all transactions from the Bitcoin network go through his/her computer
and his/her Bitcoin client validates all transactions. A single instance of a
full client doesn't make the network secure by itself. But common efforts from
the thousands of individuals is the main idea behind Bitcoin and this is what
makes the network secure *(although it seems to work slightly differently)*.
Also, light clients can work only if they are connected to full clients.

#### Highlights

* A full client is a server software.
* It requires long bootstrapping time and consumes >120 GB of hard drive.
* It constantly consumes the CPU and the network bandwidth, but the load is not
  severe.

![]({{ "/assets/2018-01-09-blockchain-bitcoin/light-full-miner.png" |
absolute_url }})

### Miner

#### Story

The user runs a data center of specialized mining hardware. Mining produces
Bitcoins and huge electricity bills. Likely, Bitcoin price is high enough
to compensate bills and to provide revenue. Mining requires separate software
but the user also runs a full client. The user is an active part of Bitcoin
community because his/her wealth directly depends on the Bitcoin ecosystem
health. The user also knows that his/her service is required to "commit"
transactions of other bitcoiners into the blockchain. And his/her honesty is
required to secure the network.

Nowadays solo mining doesn't work. Modern miners form groups to compute together
and to share a reward. This approach is called mining pools. The main idea here
is to smooth rewards over time. Instead of a huge reward once per century, you
get a tiny reward daily. So our miner user is also a part of some mining pool.

#### Highlights

Mining
* is an activity required to "commit" transactions of other users; we can think
  that miners serve the network;
* requires expensive hardware: video cards or even specialized mining computers;
* consumes a lot of energy;
* gives a reward to miners;
* is performed *(mostly)* by mining pools.

## How things work

Bitcoin is a cryptocurrency. It is very similar to a banking application. There
are accounts, balances, and transactions which move money between accounts.

Recall our classification from the previous post. A state, State
transformations, consensus. Let's start with a state. We need to define what is
stored and how it's stored. Bitcoin uses several simple but clever data
structures, let's review them.

### Hash pointers, Merkle tree

A hash function collapses arbitrary input into a shuffled shorter version which
is called a hash. And that function shuffles bits so well that single-bit change
in input **completely** changes output. And it's **super hard** to tweak input
so it produces the desired output. Super hard.

A hash pointer is simply an object's hash used as a unique identifier of that
object. An important property of hash pointer is that it points to a content of
an object, not to its location. That "points to content" part mean immutability
since hash *(pointer)* changes if content changes. My intuition is that hash
pointer is a "little photograph" (\*) of an object. With those little
photographs, I welcome you in the world of functional programming. Here in order
to change last value of a linked list, we change the whole list. And this is
good for our purposes. If you are not convinced that change of the last element
of an immutable list affects all nodes then here is my analogy. The first
element contains a "photograph" of the second element. That "photograph"
contains a smaller version of 3rd element's "photograph". And further by
induction. So if you have a really good microscope then you can zoom into the
first photograph. And you will find out that it contains the small image of the
last element. So changing the last element really requires changing that first
photograph *(which is part of the 1st element)*.

Let's see, what happens if we build a linked list using hash pointers and use
each node as a container for many items. Think, we **chained** nodes together
and each node essentially is **a block** since it stores several items.. yes! We
just defined a blockchain. We didn't define what is stored inside blocks but
that data structure is the foundation for a blockchain. Also, note that part of
existing list can be used to build a new list. Below is a picture of many
different lists which share the same tail. In the blockchain world, this
situation is called a fork.

![]({{ "/assets/2018-01-09-blockchain-bitcoin/list-fork.png" | absolute_url }})

Merkle tree is a binary tree which uses hash pointers. It has a convenient property:
verifying that element belongs to a tree requires inspecting only O(log n)
nodes. So if one person wants to convince you that some value is in a tree then
he/she can send you all nodes on the path from a root to the node of interest.
That set of nodes is called a Merkle proof.

(\*) *Please note that a small photograph is not 100% good analogy because it's
hard to see 1-pixel change on a small photograph, while a 1-bit change in input
completely changes resulting hash*.

### Accounts, History

Bitcoin uses public/private key cryptography to implement accounts. The idea is
simple: if a transaction withdraws money from an account then it *(the
transaction)* must be signed by the corresponding private key. The public key is
used as the public address of the account. Similarly to traditional banking,
increasing balance doesn't require a signature of a receiver.

Now we have a blockchain, a Merkle tree, accounts, and transactions. With that,
we can build a log of transactions *(aka a ledger)*. Transactions are stored
inside blocks. They are organized using Merkle tree. Blocks form blockchain.
Usage of hash pointers provides immutability so you can add new blocks but can't
change old blocks without changing the root block.

That append-only log of transactions provides storage. Usage of private/public
keys provides accounts security. For complete banking application, we need
initial money supply because before we can move funds we should define which
accounts have money. Let's ignore that problem for a moment. If so, then
discussed data structures and an accounts system are sufficient to implement a
banking application which works on a single computer. Soon we will discuss how
to turn it into a distributed application.

### Mining

Mining helps to solve two problems:
* consensus *(discussed in consensus section)*
* money distribution.

#### Puzzle

A puzzle makes it difficult to produce some digital content but it allows easy
verification of the resulting solution. "Difficult" means that time and
computational power *(i.e. money)* should be spent.

The original purpose of puzzles in computer science was reducing email spam
*(see [Hashcash](https://en.wikipedia.org/wiki/Hashcash))*. One of the reasons
why email spam is so widespread is that sending emails is instantaneous and
almost free. If there would be a price to pay for each email then Internet-scale
spam would be hard *(or even impossible)*. To add this price we can agree *(by
using standards)* that each valid email should contain a valid puzzle solution.

An important note about a puzzle is that it works similarly to a hash function.
It takes a content as input and produces smaller output. Different puzzles exist
but Bitcoin puzzle is, in fact, a hash function with an additional restriction.
It takes two inputs: actual content and nonce field. A nonce field is just a
dummy field with arbitrary content. The Bitcoin puzzle is that: tweak nonce
field until you obtain a hash with N leading zero bits. The important properties
of that puzzle are:
* it's the only solution is brute force;
* if two competitors A and B are solving the same puzzle then their chances of
  winning the race are proportional to their computing power.

Note the N in "N leading zero bits" part of the puzzle definition. N is a
variable hence puzzle difficulty is adjustable.

![]({{ "/assets/2018-01-09-blockchain-bitcoin/puzzle.png" | absolute_url }})

#### New blocks, money distribution

Each block from the ledger contains a puzzle solution. Today it's really hard to
produce new Bitcoin block due to high puzzle difficulty *(discussed in next
section)*. Please note that producing transactions and producing blocks are
separate activities. To produce a transaction you need a private key. To produce
a block you need computational power and time to solve a puzzle. Generally,
block producers take transactions from other bitcoiners.

Bitcoin uses really smart scheme for money distribution. I think it's the
smartest thing in the whole system. Each new block gives a fixed reward to a
person who has produced a block. Also, each transaction gives optional
transaction fee to a block producer. So block producer takes two types of
rewards for a successful new block.

The process of blocks creation is called mining because it produces new
Bitcoins.

A fixed reward for a new block exponentially decays over time. The idea here is
that eventually all money will be distributed and then only existing money will
circulate. Few notes from me:
* People burn electricity *(i.e. real money)* to mine Bitcoins. I think it
  actually increases Bitcoin value. Once you spent real money to gain something,
  you are not going to sell it cheaper than your expenses were.
* Everyone who is willing to invest real money and to accept risks can
  participate in mining. It improves coins distribution.

### Consensus

Finally, it's time to rock and to define what it's all about. We have talked
about a linked list of blocks *(aka ledger, aka blockchain)* which represents a
history. We defined mining as a way to produce new blocks. Let's add few more
pieces and build the whole picture.

Bitcoin nodes form a p2p network. Each node stores its own copy of a
transactions history *(aka ledger)*. The shiny goal of a consensus algorithm is
to make sure that each node has the exact same history. And it works like this:

![]({{ "/assets/2017-12-21-blockchain-intro/POW.png" | absolute_url }})

1.  The system has known puzzle difficulty *(see below (\*))*.
2.  Any node can create a new block. In order to do so, it should solve a puzzle
    *(perform work)* and put the answer in the block header.
3.  New blocks are telegraphed to other nodes of the p2p network.
4.  If there is no other candidate, then all nodes accept a new block to be the
    new true history.
5.  There could be a race condition when two different nodes produce blocks at
    the same time *(approximately same time is enough due to network latency)*.
    At this point, the network can't decide which history is better. There is
    the rule: best history is the longest history, but both versions of history
    have the same length. At this point, there is two version of a history and
    no consensus on the latest block. We call this situation a fork.
6.  At some point, new blocks will be produced on top of either of two forks and
    the longer branch will become the true history. But due to insanely bad
    luck, two new blocks can be produced approximately at the same time on top
    of both branches in the fork. This situation can't last for a long time
    because low probabilities multiply. And hence longer forks have a lower
    probability of existence. Forks with length 3 are much less likely to happen
    than a fork with length 2 than a fork with length 1. Common practice in
    Bitcoin community is to believe that history deeper than 5 blocks will never
    change. In other words: transactions are finalized after 5 confirmations.

(\*) Puzzle difficulty is determined by previous blocks' timestamps which are
written in a blockchain. So existing history determines difficulty for the next
block.

Why this algorithm is called a "Proof of work"? My intuition is that someone who
spent more computation power *(work)* can prove that his version of history is
the right one. In other words, work is used as a proof.

Few more details about confirmations. Amount of confirmations is how deep a
transaction is in the history. Transaction in the latest block has 1
confirmation, in the second block transactions have 2 confirmations and so on.
But there are transactions which are not *(yet)* part of the history. Those are
transactions with zero confirmations. They are telegraphed by nodes and
eventually they reach miners. Miners produce blocks and after some time
transactions become written into history and they receive confirmations. 
### Bonuses

#### Consistency vs Availability

Here is an exercise: what happens if a large part of Bitcoin network becomes
isolated for some period of time?

#### Interesting facts

Set of unconfirmed transactions is called a Memory Pool *(strange terminology)*.

History starts from the genesis block which is special block without a parent.
It's hardcoded into the source code. There are other hardcoded blocks called
checkpoints. 

The first transaction inside each block is called a coinbase transaction. It
specifies an address where mining reward should go and it also contains the
additional nonce field.

#### UTXO

The straightforward implementation of balances allows partial withdraw of money
from an account. This makes light clients hard to implement because a system
should account many transactions to know the exact balance of each account.

Bitcoin improves the situation by storing money inside unspent transaction
outputs (UTxOs). Each transaction has inputs and outputs. Output specifies the
amount of money it has and also specifies a link to its owner's account. Inputs
are just unspent outputs of other transactions. So you own money if you own some
unspent transaction outputs. Bitcoin requires transactions to completely spend
each of its input. This scheme simplifies computing of account's balance and
still makes possible partial withdraw from unspent output *(by producing 2 new
outputs from it where one of the new outputs is owned by your account)*.

![]({{ "/assets/2017-12-21-blockchain-intro/utxo.png" | absolute_url }})

The UTXO scheme greatly reduces the amount of information required by a light
client to compute its balance.

#### Bloom filter

If a light client asks a full node "hey I want all transactions which affect
account XXX" then the full client will notice that the light client is somehow
connected to that account. To protect its privacy the light client can tell
instead "hey, full node, here is a bloom filter, pass me all transactions
selected by that filter". The idea is that a bloom filter will match multiple
accounts and it's hard to understand which of accounts is associated with a
light client.

If you forgot how bloom filter works, here is how:

![]({{ "/assets/2018-01-09-blockchain-bitcoin/bloom-filter.png" | absolute_url
}})

#### Bitcoin scripts

When a full node receives a new transaction it checks transaction validity. To
do so it checks crypto signatures and ensures that inputs are valid *(that they
exist and that they aren't spent)*. There is two flavor of transactions: those
who were written into blockchain history *(confirmed)* and those who weren't
*(unconfirmed)*. In order to validate confirmed transactions node also runs
scripts associated with that transaction. Scripts allow implementing some more
complicated applications other than simple tokens movement. Examples are a
lottery, escrow transactions, a voting. As far as I know, it's almost never used
in practice *(according to [this
lectures](https://www.coursera.org/learn/cryptocurrency))*. There are
initiatives to revive Bitcoin script, for example, [Ivy for
Bitcoin](https://blog.chain.com/ivy-for-bitcoin-a-smart-contract-language-that-compiles-to-bitcoin-script-bec06377141a),
but I am not sure if it will be useful.

The most interesting thing about bitcoin scripts is that they are replayed on
all full nodes. So in case of Bitcoin, both storage and CPU are consumed on each
node to perform same actions.

## Cheatsheet

1.  State.

    History of all transactions. Stored on all nodes. *There are also light
    clients.*

2.  State transformations.

    Transactions. Replayed *(and validated)* on all nodes.

3.  Consensus.

    Proof Of Work. Mining is performed by dedicated hosts.

[4] P2p network, nodes telegraph new transactions and blocks.

## To be continued

Things I intentionally skipped to make the post shorter:
+ history of Bitcoin forks, backward compatibility
+ bitcoind resources consumption
+ limits of PoW
+ how mining pools work
+ peers discovery.

I plan to make a shorter post about Etherium and then, probably, will touch some
of the mentioned topics in the following posts.

Thanks for reading.

{% include disqus.html %}
