---
layout: post
title:  "Blockchain overview: Ethereum"
date:   2018-02-17 00:00:00 +1000
categories: blockchain
---

## Introduction

From sufficiently long distance Ethereum and Bitcoin look very similar. They
both:
+ are cryptocurrencies
+ use Proof Of Work consensus algorithm *(and hence mining)*.

Look closer and you will find the first and the biggest difference: the attitude
of core developers. Bitcoiners are very conservative and they don't break
backward compatibility. Ethereum developers are more open-minded and risky
persons. They are in constant search for new features and they deploy new stuff
even if it requires their users to update client software.

Without considering human factor *(or developers factor, don't sure if all
developers are ordinary humans)*, the biggest difference is that Ethereum
implements 2 applications in the single system. Remember my point of view on
Bitcoin? Bitcoin is a distributed banking-like application *(a cryptocurrency)*.
Ethereum implements
+ a banking-like application (a cryptocurrency)
+ a programming language interpreter.

### Interpreter

The traditional financial system is more complex than just moving money between
accounts. There are regulations and each business involves relationships with
companies and people. People sign contracts like "We will pay you XXX if you do
YYY but the government will punish you if you fail to do YYY". Contracts are
enforced by 3rd parties like government institutions. In the spirit of
cypherpunk, Ethereum provides a solution for implementing contracts and
regulations in purely electronic form without government institutions.

There are several possibilities how we can implement contracts using blockchain.
1. We *(blockchain developers)* can guess which kinds of contracts are useful
   and implement them directly inside blockchain code. In this case transactions
   format should provide fields to represent usage of available contracts.
2. We can make blockchain scriptable and let users define their own contracts.
   Scripts could be installed separately from transactions or listed inside each
   transaction.

Ethereum follows the 2nd path. In contrast with Bitcoin, Ethereum scripts are
Turing complete, which means they can compute everything which can be computed
*(no offense for such definition)* and one can write a program which never
terminates. Also, Ethereum script can be associated with an account, or in other
words, it can be "installed" into blockchain. After installation scripts live
their own digital life. Script execution can be triggered directly by a
transaction from a "real" user, or by a message from another script.

These Blockchain-based contracts are called smart contracts, and smart contracts
are the main innovation of Ethereum. My intuition for smart contracts is that
there are independent actors *(or agents)* stored in state. Users can wake up
actor by transaction and then actor can read state, send messages to other
actors and write data into the state. This is a cool idea which allows
implementing some canonical behavior in code so that no one can affect that
behavior by threats or bribes. This direction of thoughts leads to the ideology
"Code is a law".

![]({{ "/assets/2017-12-21-blockchain-intro/ether-smart-contracts.png" |
absolute_url }})

### Distributed virtual machine

In a broader sense, Ethereum is a distributed virtual machine build on top of
the blockchain. "Virtual machine" here means the same thing as Java Virtual
Machine. It's just a program which executes other programs. Blockchain means
resilience to Byzantine failures, immutable history, and so on.

I think distributed general purpose virtual machine on top of blockchain is an
absolutely beautiful idea. Just think: you can ask the system to run your code
and you don't worry about infrastructure and probably you don't worry about
costs. Just let your customers pay for computations required to serve their
needs. Don't think about CDNs: your code automatically deploys to every region
in the World. Don't think about scaling, the system can automatically leverage
thousands of machines. Brilliant idea. With such technology, you will get rid of
your inner system administrator and you will become the pure developer. This
technology will purify you. Just imagine\... Pure programmer. Clean.

Is Ethereum general purpose enough to satisfy my dreams? From one side, Ethereum
virtual machine is Turing complete. From the other side, set of exposed APIs and
performance of Ethereum virtual machine dramatically limit its applications.
It's not general purpose enough even for everyday programming. But I like to
think about blockchain from that angle. Modern blockchains are limited but I
hope that after several iterations people will figure out how to implement
distributed resilient general-purpose high-performance virtual machine. And pure
programmer will be born.

One important remark. Blockchain nature also applies to scripts. This means:
* malicious node can't forge wrong execution results, there is guarantee that
  each script will be executed properly;
* to satisfy previous property, each full node executes all scripts *(there are
  ideas to optimize this, but nothing was deployed so far)*.

### Cryptocurrency

One of the main ideas of Bitcoin is that most users will obey rules because
there are incentives to obey the rules. A user who plays by the rules will
obtain more money than the one who breaks the rules. The same applies to
Ethereum but there are few more details which arise from Turing completeness of
its scripting language.

Similarly to Bitcoin, each transaction is "replayed" *(validated)* on each full
node. But validation of Ethereum transaction means the execution of arbitrary
scripts which can eat a lot of CPU. One even can write an infinite loop in
Ethereum scripts, how Ethereum ensures validation process will terminate? The
issue is solved by right incentives. Execution of each instruction in Ethereum
script costs money *(it's a transaction creator who pays)*. So infinite loop
costs an infinite amount of money and hence impossible. Clearly, with such
incentive, programmers are motivated to write short programs and it also
prevents abuse of Ethereum scripts.

This pattern of combining cryptocurrency with other application is common in
blockchain world. For example, there is a social network where you pay to create
posts. Usually to use an application coupled with cryptocurrency user spends
that currency. This is a neat way to pay for the service and to prevent abuse.
Kind of digital symbiosis.

### Other innovations

#### State organization

Ethereum organizes history differently compared to Bitcoin. It explicitly stores
state in each block. To reduce storage consumption Ethereum represents the state
as a tree data structure *(actually as a trie)*. The main idea here is that
state transformation usually changes only small part of the state which is a
small subset of nodes in the tree. The new state can use almost all nodes from
old one except for root node and several other nodes which locate on the path
from the root to changed nodes. In fact, the tree is a base for many immutable
functional data structures. For example, if you explore immutable Vector from
Scala standard library you will find that it is actually a tree where each node
is a small immutable array.

![]({{ "/assets/2017-12-21-blockchain-intro/ether-state.png" | absolute_url }})

#### Ghost

Bitcoin automatically adjusts difficulty based on previous history. The goal of
this adjustment is a constant block creation rate. For Bitcoin, the target rate
is a one new block per 10 minutes. Why 10 minutes? It is an empirically chosen
trade-off between latency and security.

Latency characterizes time from request creation to its completion. Or, in other
words, a time interval from the moment a user made a payment to the time when
other person confirmed that he/she received the payment and he/she is absolutely
sure about that fact *(at that moment transaction became immutable, or
finalized)*. Generally, low latency is a good thing, but in the blockchain
world, it is hard to achieve low latency. For example, common Bitcoin practice
is to wait for 5 confirmations before acknowledging payment which is pretty bad
latency for buying a coffee *(it's 50 minutes!)*.

Security was mentioned because high blocks creation rate increases forks
probability and forks can reduce security. In order to understand why blocks
creation rate affects forks probability, imagine how new blocks propagate
through the p2p network:
1. A full node receives a new block, validates it and sends to other nodes.
   There is a transmission delay. And there is processing which happens between
   receiving and sending of a block.
2. There is a distance between nodes. It's both a geographical distance and a
   distance in context of the p2p network. It's common for blocks to travel
   through intermediate nodes in order to go from A to B.

Points 1 and 2 multiply and you obtain time required to propagate a new block.
Two new blocks created during this time interval will cause a fork. How likely
it is for miners to independently produce two blocks within fixed time interval
and to create a fork? It depends on blocks creation rate. The shorter interval
between new blocks, the higher probability to mine block at any point in time.
Block propagation time is fixed because it includes networking latency and can't
be reduced. So reducing the time between new blocks increases the probability of
forks.

Why are forks bad? First of all, forks make history unstable and Bitcoin's "5
confirmations" rule exists to tolerate this fact. Also, forks cause orphaned
blocks which are blocks rejected in favor of the longest branch, or in other
words, unlucky forks. Orphaned blocks don't contribute to the network security
because computational power used to create orphaned blocks is literally wasted.
This fact increases chances for an attacker to concentrate enough computational
power and to create the fork longer than 5 blocks *(and to fool users who
thought that 5 confirmations are secure enough for everyone)*. To clarify this
point: imagine that 40% of computational power is wasted to create orphaned
blocks. Then an attacker needs only 20+% of computational resources to create
1/3 of useful computational power and to compete with honest miners. 1/3 is
because honest miners doesn't coordinate and there is the analysis which shows
that 1/3 portion of computational power and some luck is enough to create useful
attacks on Bitcoin.

Ethereum produces new block every 20 seconds *(approximately)*. As a result of
that short block creation interval, there is a bigger probability of short
living forks. To solve this issue:
* there is "12 confirmations" rule which means transaction finalization time is
  approximately 3 minutes;
* there is Ghost scheme to increase the security of the network.

Ghost scheme is a simple idea: use orphaned blocks to secure the network. Block
creators can reference orphaned blocks in order to increase their own reward and
to give some reward to orphaned blocks creators. Orphaned blocks are not used to
include new transactions in the blockchain instead they are used to calculate
the longest branch *(the longest subtree in this case)*. Hence computational
power used to create orphaned blocks is not wasted. And hence short block
creation time doesn't cause the problem described earlier.

#### Mining puzzle

ASIC is an approach of making specialized hardware to perform a single task
which is faster and/or cheaper than general purpose solutions. Mining hardware
is a good example of ASICs: specialized hardware computes hashes faster using
less energy. It sounds like a good thing: people figured out how to mine new
blocks faster using less power. Well, not exactly. Automatic difficulty
adjustment makes intervals between new blocks approximately constant regardless
of total computational power. So ASICs doesn't speed up Bitcoin, they
* secure network by increasing computational power
* allow ASICs' owners to dominate over miners who don't use ASICs.

Many cryptocurrency theorists consider ASICs to be harmful because ASICs shift
mining from individuals to companies. And companies are something that
cypherpunk considers to be a bad thing. Ok, not all cryptocurrency users are
cypherpunk fans but anyway decentralization is commonly considered as a good
thing. However, ASICs cause centralization of mining hence are evil.

To be truly decentralized Ethereum tries to be ASIC resistant and
ordinary-user-friendly. From Ethereum wiki: "We try to make it as easy as
possible to mine with GPUs". To do so, they use memory hard puzzle which means
solving puzzle requires a lot of fast RAM memory. The key idea is that memory
can't be further specialized to work faster so ASICs should use ordinary memory.
That kills the idea of ASICs because commodity hardware with the same amount of
RAM will be cheaper than ASIC due to production volumes.

## Planned changes

### Casper

There are pros and cons for Proof Of Work consensus algorithm. Pros are:
* well studied by scientists and practitioners;
* distributes money *(in some sense is achieves fair distribution)*;
* easy to implement.

Cons:
* has latency limits *(i.e. long finalization time)*;
* consumes a significant amount of electricity.

Latency is a real problem. It's impractical to buy coffee using Bitcoins because
your barista will get tired waiting for transaction completion *(not to mention
transaction fees)*. There are several solutions to reduce latency, including
alternative consensus algorithms. One of such algorithms is a Proof Of Stake. We
will discuss different Proof Of XXX algorithms later, but the key point is that
Ethereum authors talk about replacing Proof Of Work with their variation of
Proof Of Stake called Casper.

### Sharding


![]({{ "/assets/2017-12-21-blockchain-intro/ether-sharding.png" | absolute_url }})

Sharding and replication are bread and butter of a distributed system's
programmer. Replication increases redundancy and it is useful mostly to increase
fault tolerance *(it also can increase the performance of certain operations)*.
Sharding splits load between different nodes and it is useful mostly to improve
the performance of a system. There is no sharding in Ethereum or Bitcoin. And
both systems are as redundant as possible. Each full node performs the same
tasks:
* stores the whole state;
* validates the whole state;
* telegraphs all transactions and blocks.

Redundancy imposes certain limits on Ethereum and Bitcoin performance. Note that
transactions validation in Ethereum requires more computational power than
validation in Bitcoin. The throughput of Ethereum is bigger than the throughput
of Bitcoin hence its state grows faster. So redundancy is a blessing *(it helps
to achieve resilience)* and a curse *(it limits performance)* at the same time.

Sharding is a clever solution for reducing redundancy. Just split work into
parts and assign different nodes to work on different shards *(parts)*. Sharding
raises few important questions:
* which parts of the system to split *(state, validation, networking)*?
* wherever to make sharding transparent or expose sharding in end-user APIs?
* how to make communication between shard possible?
* how to ensure that attackers can't leverage sharding for attacks?

Those are hard questions and Ethereum didn't answer them yet. The only thing I
understood from their wiki is that some sort of randomized shuffling can be used
to prevent attacks on individual shards. There are also ideas how to organize
shards, but no final solution yet *(I mean no solution from Ethereum)*.

## Cheatsheet

1.  State.

    Snapshot of the current state. Stored on all nodes.

2.  State transformations.

    Transactions, smart contracts. Replayed *(and validated)* on all nodes.

3.  Consensus.

    Proof Of Work *(with memory hard puzzle and 20s blocks)*.

## Interesting facts

### Colorful papers

Each cryptocurrency publishes its white paper which is a formal description of
its new algorithms and ideas. Ethereum published the document called the yellow
paper instead of a white paper. This is obviously a joke but several other
cryptocurrencies did the same thing, so there are a blue paper and a polka dot
paper.

### Ethereum classic

In 2016 Ethereum team deployed buggy smart contract which was exploited to steal
a huge amount of money. As a response to the attack, Ethereum developers made
incompatible changes in Ethereum code, fixed buggy smart contract and rolled
back transaction which stole money. Technically, incompatible code change and a
history revert caused the creation of a new currency which was agreed to be the
"true" Ethereum. The old currency with the old history survived and was renamed
to Ethereum classic. It's main idea is that code is a law and what was done
shouldn't be reverted. The event is called the DAO.

## To be continued

Finally, I came to a review of ideas from several popular altcoins. Ethereum is
most popular altcoin but there are many others! I want to cover several
alternative Proof Of XXX algorithms, performance issues, and scaling solutions.

Thanks for reading.

{% include disqus.html %}
