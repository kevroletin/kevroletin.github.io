---
layout: post
title:  "Blockchain overview: Bitcoin (WIP)"
date:   2017-12-22 00:00:00 +1000
categories: blockchain
---

Work In Progress
================

I plan to improve this post. Topics to cover:
+ history of Bitcoin forks, backward compatibility
+ bitcoin speed, limits of PoW 
+ how light client works, bloom filter (picture of several plates with holes combining into single filter)
+ memory pool
+ mining pools
+ dns seeds

Bitcoin, Proof Of Work 
======================

Before diving into technical details I will describe an ordinary day of an
ordinary Bitcoin user. There are several possible roles:
* a user with a light client
* a user with a full node
* a miner
* a developer

If you don't like stories then skip to a highlights section. 

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
transaction time. Even with a good fee it takes about 50 minus for a transaction
to finish. It seems like today transactions don't commit without a fee anymore.
This is because there is a constant supply of transactions with good fees and
Bitcoin's throughput is limited.

Later, the user sold his car and invested into Bitcoins. To protect his
investment he generated new account, printed private key on a paper and locked
it in his safe. He wiped the private key from the hard drive so there is no way
for hackers to steal his money. Such approach is called cold/hot wallets. The
cold one is in the safe. It's not intended for everyday use and it is secure.
The hot one is in mobile phone. It's for everyday use but it's security suffers
from software bugs, social engineering and other hacking techniques.

#### Highlights

* A light client can work on a "weak" device like a mobile phone.
* There is no bootstrap time for light client. All you need is to install a
  software and to generate an account.
* Account is as security as the private key.

### Full client

#### Story

Aka fully validating client. This user installed client software, configured
firewall to allow incoming connections and started bitcoind background process.
Few days later he/she found that bitcoind has occupied 120 GB of storage and it
constantly communicates over network. But cpu and network usage aren't too bad
and storage consumption doesn't seem to grow too fast. The user tries to run his
computer 24/7 and occasional reboots doesn't seem to affect bitcoind behavior:
after reboot software catches up quickly.

Now the user can do all the same things that light client user can *(and even
more!)*. An additional feature for his investment is the felling of security. He
knows that all transactions from the Bitcoin network go through his/her computer
and his/her Bitcoin client validates all transactions. Also light clients works
only because full clients operate. A single instance of a full client doesn't
make the network secure by itself. But common efforts from the thousands of
individuals is the main idea behind Bitcoin and this is what makes the network
secure *(although it seems to work slightly differently nowdays)*.

#### Highlights

* A full client is a server software.
* It requires long bootstraping time, and consumes >120 GB of hard drive.
* It constantly consumes the CPU and the network bandwidth; but load is not severe.

### Miner

#### Story

The user runs a data center of specialized mining hardware. Mining produces
Bitcoins and huge electricity bills. Likely enough, Bitcoin price is high enough
to compensate bills and to provide revenue. Mining requires separate software
but the user also runs a full client. The user is an active part of Bitcoin
community because his/her wealth directly depends on the Bitcoin ecosystem
health. The user also knows that his/her service is required to "commit"
transactions of other bitcoiners into the blockchain. And his/her honesty is
required to secure the network.

Nowdays solo mining doesn't work. Modern miners form groups to compute together
and to share a reward. This approach is called mining pools and the main idea
here is to smooth rewards over time. Instead of a huge reward once per century
you get a tiny reward daily. So our miner user is also a part of some mining
pool.

#### Highlights

Mining
* is an activity required to "commit" transactions of other users; we can think
  that miners are "servants".
* requires expensive hardware: video cards or even specialized mining computers.
* consumes a lot of energy.
* gives reward to miners.
* is performed *(mostly)* by mining pools.

## How things work

Bitcoin is a cryptocurrency. It is very similar to a banking application.
There are accounts, balances and transactions which move money between accounts.

Recall our classification system from the previous post. State, State
transformations, Consensus. Let's start with state. We need to define what is
stored and how it's stored. First part is "how it is stored". Bitcoin uses
several simple but clever data structures, let's review them.

### Hash pointers, Merkle tree

A hash function collapses arbitrary input into shuffled shorter version which is
called a hash. And that function shuffles bits so well that single-bit change in
input **completely** changes output. And it's **super hard** to tweak input so
it produces desired output. Super hard.

A hash pointer is simply an object's hash used as an unique identifier of that
object. Unique property of hash pointer is that it points to content of an
object not to it's location. That "points to content" part means immutability
because hash *(pointer)* changes if content changes. My intuition is that hash
pointer is a "little photograph" (\*) of an object. With those little photographs
I welcome you in the world of functional programming. Here in order to change
last value of a linked list we change the whole list. And this is good for our
purposes. If you are not convinced that change of the last element of an
immutable list affects all nodes list then here is my analogy. The first element
contains a "photograph" of second element. That "photograph" contains smaller
version of 3rd element's "photograph". And further by induction. So if you have
a really good microscope then you can zoom into the first photograph. And you
will find out that it contains the small image of the last element. So changing
last element really requires changing that first photograph *(which is part of
1st element)*.

What happens if we build a linked list using hash pointers and use each node as
a container for many items. Think, we **chained** nodes together and each node
essentially is **a block** since it stores several items.. yes! We just defined
a blockchain. We didn't define what is stored inside blocks but that data
structures is the foundation for a blockchain. Also note that part of existing
list can be used to build new list. Below is a picture of two different lists
which share same tail. In the blockchain world this situation is called a fork.
We will return to this picture later to discuss a consensus algorithm. 

TODO: fork picture

Merklee tree is a binary tree with hash pointers. It has a convenient property:
verifying that element belongs to a tree requires inspecting only O(log n)
nodes. So if one person wants to convince you that some value is in a tree then
he/she can send you all nodes on the path from root to the node of interest.
That set of nodes is called a merklee proof.

TODO: merklee proof picture 

(\*) *Please note that a small photograph is not 100% good analogy because it's
hard to see 1-pixel change on a small photograph, while 1-bit change in input
completely changes resulting hash*.

### Accounts, a ledger

Bitcoin uses public/private key cryptography to implement accounts. The idea is
simple: if a transactions withdraw money from an account then it *(transaction)*
must be signed by the private key of the account. The public key is used as an
public address of the account. Similarly to traditional banking, increasing
balance doesn't require signature of receiver.

Now we have a blockchain, a merklee tree, accounts and transactions. With that
we can build a log of transactions *(aka a ledger)*. Transactions are stored
inside blocks. They are organized using merklee tree. Blocks form blockchain.
Usage of hash pointers provides immutability so you can add new blocks with
transactions but can't change old blocks without changing the root block.

We have an append only log of transactions. Usage of private/public keys
provides accounts security. For complete banking application we need initial
money supply because before we can move funds we should define which accounts
has money. Let's ignore that problem for a moment. If so then discussed data
structures and an accounts system are sufficient to implement a storage and a
transaction validation. So we have a banking application which works on a single
computer. Now we are going to discuss how to turn it into a distribute
application.

### Puzzle

A puzzle makes it difficult to produce some digital content but it allows easy
verification of the resulting solution. "Difficult" means that time and
computational power *(i.e. money)* should be spent.

The original purpose of puzzles in computer science was reducing email spam
*(see [Hashcash](https://en.wikipedia.org/wiki/Hashcash))*. One of the reasons
why email spam is so widespread is that sending emails is instantaneous and
almost free. If there would be price to pay for each email then Internet-scale
spam would be hard *(or even impossible)*. To add this price we can agree *(by
using standards)* that each valid email contains a valid puzzle solution.

An important note about a puzzle is that is works similar to a hash function. It
takes a content as input and produces smaller output. Different puzzles exist
but Bitcoin puzzle is, in fact, a hash function with an additional restriction.





### Mining

Mining solves two problems:
* consensus
* money distribution.

I can't define which problem is harder but my intuition is that it is the money distribution.

### Full client

### Light client

1.  State.

    History of all transactions. Stored on all nodes.

    Each fully validating Bitcoin node stores all transactions from day
    1 of Bitcoin (\*\*). It keeps track of UTxOs to validate
    new transactions. Today this is about 120gb of data and it
    keeps growing.

    There are also lightweight Bitcoin clients. They don't store the
    whole history and hence unable to validate new transactions.
    Instead, they rely on full nodes. Lightweight node is quite secure
    because it's hard for a full node to cheat by injecting arbitrary
    false data into its responses *(due to excessive usage of hash
    pointers in Bitcoin)*. But lightweight clients doesn't increase
    security or performance of the whole p2p network.

    There is also an option to reduce the hard drive consumption by
    deleting spent transactions from the history. It is not implemented
    but possible.

2.  State transformations.

    Transactions. Replayed on all nodes.

    When a full node receives a new transaction it checks transaction validity.
    To do so it checks crypto signatures and ensures that inputs are valid
    *(that they exist and that they aren't spent)*. There is two flavor of
    transactions: those who were written into blockchain history *(confirmed)*
    and those who weren't *(unconfirmed)*. In order to validate confirmed
    transactions node also runs scripts associated with the transaction. Scripts
    allow implementing some more complicated applications other than simple
    tokens movement. Examples are a lottery, an escrow transactions, a voting.
    As far as I know, it's almost never used in practice *(according to [this
    lectures](https://www.coursera.org/learn/cryptocurrency))*. There are
    initiatives to revive Bitcoin script, for example, [Ivy for
    Bitcoin](https://blog.chain.com/ivy-for-bitcoin-a-smart-contract-language-that-compiles-to-bitcoin-script-bec06377141a),
    but I am not sure if it will be useful.

3.  Consensus.

    Proof Of Work. Mining.

    History of transactions is represented by a linked list of blocks.
    Block is just a set of transactions. The linked list uses hash
    pointers which are simply hash value of the referred object. The
    neat property of linked list with hash pointers is that you can't
    change older values without modifying newer *(if you familiar with
    git, then you know how it works)*. This is why blockchain is called
    blockchain: it is linked list *(or chain)* of blocks.

    Chain of blocks is an append-only data structure which is often
    called a ledger. Nodes use Proof Of Work consensus algorithm to
    choose single true history. The main idea behind PoW is that the
    true history is a history produced by the greatest amount of work
    *(hence the name)*.
    
    ![]({{ "/assets/2017-12-21-blockchain-intro/POW.png" | absolute_url }})

    It works like this:
    1.  The system has known puzzle difficulty (\*).
    2.  Any node can create a new block. In order to do so, it should
        solve puzzle *(perform work)* and put the answer in the
        block header.
    3.  New blocks are telegraphed to other nodes of the p2p network.
    4.  If there is no other candidate, then all nodes accept new block
        as the new true history.
    5.  There could be a race condition when two different nodes produce
        blocks at the same time *(approximately same time is enough due
        to network latency)*. At this point, the network can't decide
        which history is better. We know the rule: best history is the
        longest history, but both versions of history have the
        same length. At this point, there is two version of history and
        no consensus on the latest block. We call this situation a fork.
    6.  At some point, new blocks will be produced on top of either of
        two forks and the longer branch will become true history. But
        due to insanely bad luck, two new blocks can be produced
        approximately at the same time on top of both branches in
        the fork. This situation can't last for long time because low
        probabilities multiply. And hence longer forks have a lower
        probability of existence. Forks with length 3 are much less
        likely to happen than a fork with length 2 than a fork with
        length 1. Common practice in Bitcoin community is to believe
        that history longer than 5 blocks will never change. In other
        words: transactions are finalized after 5 confirmations.

    Mining has social implications we will discuss this issue in the next
    article.

    (\*) Puzzle difficulty is determined by previous blocks timestamps
    written in blockchain.

(\*\*) History starts from genesis block which is special block without
a parent. It's hardcoded into the source code.

# UTXO

The system should track history of all transactions to know the balance
of each account. Bitcoin simplifies this task by storing money inside
unspent transaction outputs (UTxOs). Each transaction has inputs and outputs.
Output specifies the amount of money it has and also specifies a link to
its owner's account. Inputs are just unspent outputs of other
transactions. So you own money if you own some unspent transaction
outputs. Bitcoin requires transactions to completely spend each of its
input. This scheme simplifies computing of account's balance and still
makes possible partial withdraw from unspent output *(by producing 2 new
outputs from it where one of the new outputs is owned by your account)*.

![]({{ "/assets/2017-12-21-blockchain-intro/utxo.png" | absolute_url }})
