---
layout: post
title:  "Blockchain overview: Ethereum (WIP)"
date:   2017-12-22 00:00:00 +1000
categories: blockchain
---

Work In Progress
================

I plan to improve this post. Topics to cover:
+ more technical innovations from Etherium
+ history of Etherium forks
+ examples of smart contracts

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

