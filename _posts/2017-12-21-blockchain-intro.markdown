---
layout: post
title:  "Blockchain overview: introduction"
date:   2017-12-21 00:00:00 +1000
categories: blockchain
---

Structure
=========

For many people, blockchain and Bitcoin are interchangeable synonyms. In this
series of posts, I argue that world of blockchains is much more complicated than
that. I plan to describe features of several popular blockchains with emphasis
on performance and scaling solutions. This article is an introduction to the
subject.

Motivation
==========

My recent involvement in local startup gave me great opportunity to dig into
blockchain technology. Nowadays blockchain is buzz-word and Internet generate a
huge amount of content about cryptocurrencies, smart contracts, and a blockchain
scalability problem. People talk a lot about the usage of blockchains and
challenges of its wide adoption. Sadly enough, I don't see enough technical
content which compares existing and emerging solutions from the technical point
of view. Such article would be useful to me 3 months ago and I am going to
approach this gap.

When I started in a blockchain developer role I didn't know much about altcoins
_(alternatives to Bitcoin)_. Job description clearly stated that their main
requirement was Bitcoin knowledge so I easily passed an interview. It appeared
that my new coworkers weren't interested in altcoins at all. They believed that
all altcoins are Bitcoin forks and forks don't bring anything useful into this
world *(which is strange because they were developing altcoin by themselves)*.

I wasn't sharing the same view on altcoins so I decided to dig by myself
and to understand main innovations of most popular platforms. It appears
to be a time-consuming task because there are a lot of solutions and
many of them differentiate from others by some technical innovation.
After glancing at a dozen of whitepapers, and googling I developed
a small blockchain cheat sheet for myself. I decided to present it in a
readable form with the hope that it will be useful for others.

Blockchain overview
===================

What is blockchain? Instead of a scientific definition, I use my own intuitive
version. The blockchain is a distributed application which works in the huge
untrusted network where some parties can act maliciously for their own sake.
This definition is not self-sufficient because "application" is vague term but
we will discuss it later. For now, really important parts here are "distributed
in the huge untrusted network" and "act maliciously". I believe these two
phrases describe innovation of blockchain.

Who will "act maliciously"
--------------------------

Blockchain should tolerate Byzantine failures. Byzantine failure is when
part of distributed system misbehaves in some way other than just
stopping working *(probably in some intelligent way)*. Below is more
friendly decryption of what is a Byzantine failure.

Let's consider few examples of systems distributed across several
network nodes.

First one is a database with sharding. You install the same instance of
the database server on several nodes and split dataset between them
*(here we assuming no redundancy, all nodes have different not
intersecting parts of data)*. If one node fails then your whole
distributed service becomes malfunctioned since it can't serve data from
the failed node. This is distributed system which can't tolerate any
node failures.

![]({{ "/assets/2017-12-21-blockchain-intro/distributed-no-resilience.png" | absolute_url }})


The second example is a database with replication. You install the same
instance of the database server on several nodes and copy *(replicate)*
same data on those nodes. If one node stops serving requests: no
problem, other nodes will serve same content. This distributed system
can survive node failures when a node stops functioning *(this is stop
failure)*. But what happens if due to some weird data corruption or due
to hacker activity one of the nodes starts serving wrong data. In this
case, node didn't stop serving requests but failed by giving wrong
results or by violating communication protocols. Literature calls such
fails byzantine failures. Obviously, in case of Byzantine failure, some
of your replicated database clients will obtain wrong results so the
whole system becomes malfunctioned. And we can conclude that such
database with replication is an example of a distributed system which
can tolerate stop *(simple)* failures but can't tolerate Byzantine
*(complicated)* failures.

![]({{ "/assets/2017-12-21-blockchain-intro/distributed-stop-resilience.png" | absolute_url }})

The third example is from mission-critical embedded systems. You design
a crash detector for a car. And you know that in some very very rare
cases crash sensors can misbehave by producing wrong results. Sensors
don't stop working, they just give wrong answers. So you decided to
install three sensors instead of one and run them in parallel. With 3
sensors working in parallel you can detect failure of the single sensor
and still can figure out right answer by a using most popular among
sensors answer. This system can tolerate Byzantine failures *(it will
work correctly if up to 1/3 of nodes fail)*.

![]({{ "/assets/2017-12-21-blockchain-intro/distributed-with-resilience.png" | absolute_url }})

Algorithms for Byzantine fault tolerance where invented long before the
invention of bitcoin. Byzantine fault tolerance is not something completely new
in computer science and in the industry
([DLS](http://groups.csail.mit.edu/tds/papers/Lynch/jacm88.pdf),
[PFBT](http://pmg.csail.mit.edu/papers/osdi99.pdf)).

Why "distributed in huge untrusted network" part matters
--------------------------------------------------------

Here for simplicity, we talk about Bitcoin. It runs on the Internet and
there are the huge amount of Bitcoin nodes in different parts of the
World *([tens of thousands](https://bitnodes.earn.com/))*. The core feature
of Bitcoin is that absolutely anyone can join its network. And no one
can guarantee that arbitrary person who joins network will run the
trusted software. Most people run the client from Bitcoin developers,
but some of them run alternative client written in golang, and some of
them can run their own version of a client. And their custom version of
a client can misbehave in some intelligent way. Moreover, malicious
users can try to deploy a number of malicious nodes and coordinate their
behavior.

Bitcoin can tolerate coordinated attacks unless a majority of the network is
healthy *(see [Ghost protocol
introduction](https://www.youtube.com/watch?v=iSq-emtyx5g) for simple
explanation)*. There were algorithms which can tolerate Byzantine failures in
the untrusted network prior to Bitcoin invention. But they don't scale to a
massive network. So Byzantine fault tolerance in Internet scale is true
innovation from Bitcoin. Nowadays people figured out how to apply those
"classic" byzantine fail tolerance algorithms to build blockchain, but 10 years
ago Bitcoin was true innovation in this problem.

Model for blockchain
====================

I use a simple model to talk about blockchain. It is mostly inspired by
Ethereum wiki which talks a lot about state and state transformations.

Blockchain consists of 3 parts:

1.  State.
2.  State transformations.
3.  Consensus

Also, there is 4rth hidden part in this list: (4) communication between
nodes. Nodes communicate to gain consensus and to change state. I didn't
add it explicitly because think that distributed nature of blockchain
assumes p2p communication between nodes.

Let's talk more about this model using digital currency as an example.
Digital currency should have accounts and each account should have its
balance. List of all accounts and corresponding balances is a current
state of the currency.

State transformations are transactions. A transaction is a transfer of
money from one account to another which changes balances and state.

![]({{ "/assets/2017-12-21-blockchain-intro/state-transformation.png" | absolute_url }})

Here I want to make one important remark. State + State Transformation
is a useful model for many applications. If we also add "Representation"
to this pair then we will end up with State + Representation + State
Transformation which is essentially Model View Controller. Model View
Controlled is a model useful to talk about almost any application and it
has been dominating paradigm for web applications for a long time. By
using only State + State Transformation part you obtain Model +
Controller. With these two you can easily talk about any server-side
application. Think that State is your database. State transformations
are your business logic. I gave this remark to give you feel that
blockchain is useful not only for digital currency but also for many
other applications.

![]({{ "/assets/2017-12-21-blockchain-intro/mvc.png" | absolute_url }})

The consensus is what turns ordinary Database + Business Logic
application into popular Rocket Ship bleeding edge science sexy buzzy
hipster application. It works like this: users of the p2p network send
requests for transforming state to their peers *(they send transactions
in case of digital currency)*. Those requests spread across the network
and at some point consensus algorithm comes into play. Nodes have to
decide which requests are valid. They also need to decide in which order
to apply requests *(true for a system where order matters)*. The
ultimate goal of consensus algorithms is to make sure that at any point
in time whole p2p network has the same view *(copy)* of the state. This
is too hard *(or even impossible)* to achieve in practice so nodes in
the p2p network maintain history of state transitions. Most recent
history can differ among different nodes but older history always same
between all nodes. History stabilizes with time and consensus algorithm
needs time to reach reliable consensus.

Later we will explore blockchains where All 3 components of discussed
model differ.

Notes about distributed communication
=====================================

Every respectful talk about distributes systems refers to

1.  The CAP theorem.
2.  The FLP impossibility result.

Chapter 2 of [Distributed systems for fun and
profit](http://book.mixu.net/distsys/index.html) book talks about these
theorems in very friendly language.

My intuition for these theorems is following:

1.  There is design trade-off: in case of networking disaster system
    stays either consistent or available. Alternatively, it chooses
    behavior which is somewhere in the middle between two. The complete
    description below.

    If p2p network divides into isolated parts *(aka partitions)* then
    system have two choices. 
    
    * It can forbid state transformations to prevent state divergence between
    isolated parts of the p2p system. Such systems prefer consistency over
    availability. They stop the usual way of functioning or in other words, they
    become unavailable.
    
    * System can continues functioning which means each isolated part continue
    working on its own. Systems stay available but it is no longer consistent:
    isolated parts have no chance to coordinate state transformations and hence
    they maintain different versions of reality. Such systems prefer
    availability over consistency.
    
    ![]({{ "/assets/2017-12-21-blockchain-intro/network-partition.png" | absolute_url }})
    
2.  The Byzantine generals problem(\*) have no solution under
    strict assumptions. However, it has a practical solution which works
    well but not ideally. Proof Of Work is an example of a practical
    solution which works well in practice but not perfectly. Forks in
    Bitcoin network is an example of such imperfection, we will discuss
    it soon.

(\*) The Byzantine generals problem - several parties try to reliably
agree on some fact over an unreliable network. For example, 3 computers
try to choose a single number by using the network. This should be
solved in such way that each computer knows for sure that other 2
computers chose the same number. The most difficult part here is "you
know that I know that you know that I know \..." reasoning which doesn't
terminate unless we ignore some probability of failure. In practice,
however, networks can be described statistically and they have a low
probability of failure. So at some point, we can decide to ignore
0.0000\...0001% chance of failure. And also we can use the notion of time
and introduce timeouts which don't change anything from the theoretical
point of view but work in practice.

Bitcoin, Proof Of Work
======================

Bitcoin is a cryptocurrency. It is very similar to banking application.
There are accounts and balances and transactions which move money
between accounts.

Bitcoin uses public/private key cryptography to implement accounts. You
as user generate public/private keys pair. Your public key is your
account. A private key is a permission to use this account. You use the
private key to sign transactions to take money from the corresponding
account. Due to cryptographic asymmetry only one who knows private key
can move money from the account but everyone can validate that
transaction is rightful. This approach with asymmetric cryptography is
widely used and I am not aware of any other scheme to implement accounts
ownership in a system where all data is visible to everyone.

The system should track history of all transactions to know the balance
of each account. Bitcoin simplifies this task by storing money inside
unspent transaction outputs. Each transaction has inputs and outputs.
Output specifies the amount of money it has and also specifies a link to
its owner's account. Inputs are just unspent outputs of other
transactions. So you own money if you own some unspent transaction
outputs. Bitcoin requires transactions to completely spend each of its
input. This scheme simplifies computing of account's balance and still
makes possible partial withdraw from unspent output *(by producing 2 new
outputs from it where one of the new outputs is owned by your account)*.

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
    references in Bitcoin)*. But lightweight clients doesn't increase
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
    initiatives to review Bitcoin script, for example, [Ivy for
    Bitcoin](https://blog.chain.com/ivy-for-bitcoin-a-smart-contract-language-that-compiles-to-bitcoin-script-bec06377141a),
    but I don't sure if it will be useful.

3.  Consensus.

    Proof Of Work. Mining.

    History of transactions is represented by a linked list of blocks.
    Block is just a set of transactions. The linked list uses hash
    references which are simply hash value of the referred object. The
    neat property of linked list with hash references is that you can't
    change older values without modifying newer *(if you familiar with
    git, then you know how it works)*. This is why blockchain is called
    blockchain: it is linked list *(or chain)* of blocks.

    Chain of blocks is and an append-only data structure which is often
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

Ethereum, smart contracts
=========================

1.  State.

    Snapshot of the current state. Stored on all nodes *(there are plans
    to optimize that by sharding)*.

    Ethereum organizes history differently compared to Bitcoin. It explicitly
    stores state in each block. To reduce storage consumption Ethereum
    represents the state as a tree data structure *(actually as a trie)*. The
    main idea here is that state transformation usually changes only small part
    of the state which is a small subset of nodes in the tree. The new state can
    use almost all nodes from old one except for root node and several other
    nodes which locate on the path from the root to changed nodes. This is
    approach used in many Functional data structures. For example, if you
    explore immutable Vector from Scala standard library you will find that it
    is actually a tree where each node is a small immutable array.

    ![]({{ "/assets/2017-12-21-blockchain-intro/ether-state.png" | absolute_url }})

2.  State transformations.

    Smart contracts. Executed on all nodes *(there are plans to optimize
    that by sharding)*.

    This is a main innovation of Ethereum. Accounts not only contain
    money balances but also may contain scripts. Scripts can be
    triggered explicitly by the transaction from the user or by a
    message from another script. Scripts are executed on a virtual
    machine by each full node. Execution is not free: each instruction
    costs a tiny amount of money for an account which activated
    the script.

    My intuition for smart contracts is that there are independent
    actors *(or agents)* stored in state. Users can wake up actor by
    transaction and then actor can read state, send messages to other
    actors and write data into the state. This is a cool idea which
    allows implementing some canonical behavior in code so that no one
    can affect this behavior by threats or bribes. This kind of
    reasoning leads to ideology "Code is a law".

    ![]({{ "/assets/2017-12-21-blockchain-intro/ether-smart-contracts.png" | absolute_url }})

3.  Consensus.

    Proof Of Work. Plans to move from PoW to some form of Proof Of Stake
    *(discussed later)*.

    Ethereum slightly extends PoW with the Ghost scheme. We will mention this
    extension in scalability section in the following articles. Ghost improves
    security of PoW without directly improving throughput.
