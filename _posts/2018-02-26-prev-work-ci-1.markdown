---
layout: post
title:  "In pursuit of my 2nd CI system. Part 1"
date:   2018-02-26 00:00:00 +1000
categories: ci
---

## Introduction

> The general tendency is to over-design the second system, using all the ideas
> and frills that were cautiously sidetracked on the first one.
>
> \-\- The Mythical Man-Month by Frederick P. Brooks

I have spent about 2 years of my career working on one highly customized
Continuous Integration system. The cool thing about my experience is that word
"continuous" refers not only to non-stop execution of automated tests but also
to never ending development of the system itself. We were providing running
service for QA and Dev teams and where implementing new features in the same
time.

6 month ago I changed my job and abandoned never ending development. Sadly
enough I didn't implement many of my ideas and sometimes I think how I would
implement them in different context *(other company, other projects)*. I also
want to explore better techniques to solve similar problems. It would be cool if
someone will read the article and will say "I know how to do similar things 99%
cheaper in less time and result will be more maintainable" *(and also he/she
will appear to be right)*.

Before we can discuss requirements and implementation I want to provide some
context to convey you the atmosphere of embedded development. It significantly
differs from web/desktop development and hence CI systems for embedded projects
have some additional requirements compared to CI systems from other domains.

## Understanding domain

The main specialization of our company was **embedded development**. This is
broad term which may refer to wide range of computing hardware starting from
cheap and tiny devices with absurdly small performance finishing at high
performance computers which are embedded into ridiculously expensive scientific
or medical equipment. My main experience relates to digital photo/video cameras
so by embedded device I mean something similar.


![]({{ "/assets/2018-02-26-prev-work-ci-1/embedded-world.jpg" | absolute_url }})

### Target device lives it's own life

Embedded development always means that your working station *(e.g. your laptop)*
and target platform are different devices *(usually they even have different
architectures)*. Developers may use Windows or Linux amd64 boxes to write code,
but then they cross-compile firmware to Mips or Arm architecture and burn
binaries into separate development boards. Such workflow differs from web or
desktop development where programmers **can run** their code on the same machine
where they write it.

Aside from development boards there are also development devices. By development
device I mean almost finished device in plastic case *(similar to what you
usually see in a store)* but with soldered debug interfaces. Usually such
devices have some limitations compared to development boards. For example there
will be less debug information for electrical engineer due to absence of probes.
It also can have some obscure method of uploading a new version of a firmware. A
battery could cause testing difficulties because sometimes a battery is a part
of a test. For example, test could require to power device from a battery with
an unplugged cable to check energy safe regime *(but you should ensure battery
is fully charged before the test)*.

![]({{ "/assets/2018-02-26-prev-work-ci-1/device-life.png" | absolute_url }})

It takes time to burn firmware and to reboot a device. Sounds obvious but burn
with reboot can actually take several minutes which is not intuitive. So reboot
after each test or burn of firmware before each test group can significantly
increase tests execution time. Similar thing can happen with device failures.
It's common to dump some debug information in case of device failure. If device
transmits dump via slow interface like UART then transmission will take
significant time. Moreover logic for dump extraction could be non trivial. For
example device can write core dump onto SD card. So you need to reboot device,
ask it to send you latest core dump, or ask it to mound it's SD card as a Mass
Storage, or ask to start ftp server or something else depending of fantasy of
your Dev team.

Testing such devices always requires additional hardware. The simplest version
of a testing setup is an ordinary PC with a single connected device. The PC acts
like a puppet master: it burns firmware and communicates with the target
platform *(usually called Device Under Test)* to run tests.

### Meet chaotic world

Embedded devices often contain sensors. Let's take mobile phone as an extreme
example. Several position sensors, gps module, photo cameras, proximity sensor,
light sensor and others. Automatic testing of all those sensors is a real
challenge! Fair testing requires building some mechanical manipulators and
isolated boxes with controlled light source. Sure there are less fair testing
techniques where application artificially generates some predefined data instead
of using sensors. But choosing right testing approach is an optimization task
where you weight probabilities and estimated money loss in case of missed bug.
Sometimes it's cheaper to programmatically control the environment from tests.
Canonical example is a digital photo camera: how would you check that photo
camera actually made a photo of a scene? It's possible to automate such check by
placing a display in front of a camera and by controlling display
programmatically.

Embedded devices often use some kind of networking. It could be NFC, Bluetooth,
Wifi, Mobile networks, Ethernet and others. Network can be considered as a part
of test environment so it's often a good idea to control or at least to isolate
testing network.

One of the most desired software qualities is determinism. It's easy to test
deterministic behavior. Unfortunately tests for embedded devices rarely behave
completely deterministic. This is because devices themselves depend on external
world *(thanks to sensors and networking)* and hence not deterministic. Because
of this testing requires a lot of repetition and hence long test execution time.
Usual weapon against long tests is parallelism and hence big amount of target
devices in a testing laboratory. Unfortunately even parallelism doesn't help to
solve all problems. For example, I saw stress tests which run several days
*(they should continuously run on one device, hence no parallelism)*.

![]({{ "/assets/2018-02-26-prev-work-ci-1/photo-stand.jpg" | absolute_url }})

### Device increases entropy of the Universe

Aside from gathering physical signals by using sensors, devices also generate
signals by themselves. We already talked about network, but there are also
displays, audio speakers, flashlight, vibrator, and so on. Sometimes electronic
parts interfere between themselves. Sometimes that interference reproduces only
during some specific load. That is why automated testing of peripheral devices
is useful. And in order to test peripherals you need to somehow measure signals.

There are also physical signals which are not part of user interface.
Temperature, electromagnetic interference, voltage levels *(gathered from
different probes)*, power consumption *(actually computed from voltage level)*
and so on. Serious hardware development assumes ability to fight serious
hard-to-reproduce bugs. Signals recorded with high sampling rate quickly consume
hard drives. But hey, we live in the age of big data, so why not to use it to
correlate bugs with overheating or low voltage from a battery?

### Need moar branches

Deployment model for embedded devices is different from modern desktop or cloud
software. Once device has left a factory it's quite problematic to update it's
firmware. So there is big release day when major functionality must work. It's
acceptable to leave some minor bugs in release version with assumption that user
will download and update firmware manually.

Configuration Management model also differs from desktop/cloud software. It is
common to maintain many code branches at the same time. There are two main
reasons for that phenomenon: first is a deployment model. A big release date
means that a product transitions into support phase and a new support branch
appears. This sounds like a intuitive thing to do, but sometimes managers demand
to create support branch long before the release date to ensure that code is
stable. Also there is such thing as spin-off, it's a new product based on old
one which assumes minimum amount of changes. It's tempting to reuse old
maintenance branch to create new spin-off, hence developers may work on ancient
branches. The second reason for creating branches is a large number of hardware
configurations. It is questionable whether one should use branches or a build
system to mange hardware configurations. Linux kernel manages diverse hardware
using build system, so it should be a preferable solution. But not all
developers are kernel developers, there is time and management pressure and a
new branch appears and developer goes into a bar after his/her work to take
liquid painkiller.

### Embedded-land can't into virtualization

Finally, such simple thing as a cable could be a problem. There is very popular
*(due to it's simplicity)* UART interface which is often used to communicate
with developer's working station either in text or in binary format. There could
be several such interfaces per device. Modern PC computers doesn't have serial
ports *(serial port is a is bigger brother of UART)* so one should use UART to
USB converter. Connecting several devices to single PC could be problematic due
to limited amount of USB ports on PC. How about USB hubs? A weird thing is that
for some reason many USB hubs poorly interact with UART-to-USB converters and
reduce their stability.

Also note that UART and USB connections make virtualization usage trickier.
Tests may enable/disable USB connections and reboot a device. In some obscure
cases there could be a custom USB driver. Because of this it's preferable to
give entire USB port control to a guest OS or to a container. So one should use
usb-passthrough or pci-passthrough features which complicates setup. Often
people use least complicated solution and don't use virtualization to run tests.

Of cause, what I told is accidental complexity and one can configure
usb/pci-passthrough features to setup OS which communicates with target device
during testing. Unfortunately one can't replace device itself with a virtual
machine. So virtualization usage is limited by amount of physically installed
target devices.

## Summary

Here is short summary of what is special *(more or less)* about automated
testing of embedded devices:

1. **Separate device under test**
   * separate hardware for building firmware and for running tests
   * firmware should be burned into device and it takes time
   * physical devices can't be replaced by emulated versions.
2. **Determinism and environment**
   * controlled *(physical)* environment helps achieving better determinism
   * but there is no such thing a 100% determinism
   * advanced tests require programmatically controlled environment.
3. **History**
   * different measurements and statistics help to debug advanced issues
   * advanced hardware debugging requires physical measurements.
4. **Diversity and volume**
   * there are many hardware configurations and code branches and each of them
     should be tested
   * parallelism is a must-have feature for such loads
   * there are long running stress tests.

I didn't mention **virtualization difficulties** as a separate point but it's
important consequence of 1st one. Nowadays CI systems like [Travis
CI](https://travis-ci.com) heavily rely on virtualized environment for testing.
Travis spawns machines with fresh OS and installs all required software in order
to run tests. This approach doesn't work smoothly enough for embedded devices
because a newly created virtual machine *(or a container)* should have a
connected device to run tests. So bear in mind that separate target devices is
the biggest difficulty for organizing CI on such projects.

## Te be continued

The post happened to be quite long and I split it into two parts, so stay tuned.

Next part will define informal requirements for CI system, briefly describe one
imperfect implementation and will talk about what system I see in my
imagination.

{% include disqus.html %}
