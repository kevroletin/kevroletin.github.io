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

Bitcoin, Proof Of Work
======================

Bitcoin is a cryptocurrency. It is very similar to banking application. There
are accounts, balances and transactions which move money between accounts.

Bitcoin uses public/private key cryptography to implement accounts. You as user
generate public/private keys pair. Your public key is your account. A private
key is a permission to use your assets. You use the private key to sign
transactions to take money from the corresponding account. Due to cryptographic
asymmetry only one who knows private key can move corresponding money but
everyone can validate that transaction is rightful. This approach with
asymmetric cryptography is widely used and I am not aware of any other scheme to
implement accounts ownership in a system where all data is visible to everyone.

![]({{ "/assets/2017-12-21-blockchain-intro/utxo.png" | absolute_url }})

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
    but I am not sure if it will be useful.

3.  Consensus.

    Proof Of Work. Mining.

    History of transactions is represented by a linked list of blocks.
    Block is just a set of transactions. The linked list uses hash
    references which are simply hash value of the referred object. The
    neat property of linked list with hash references is that you can't
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

