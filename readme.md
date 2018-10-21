# Craft Services - An air-to-ground-to-air buddy flight system for iNav

## Description

Craft Services is a command line application that connects to multiple iNav crafts over wireless serial links and exchanges position data between them. A related set of patches to iNav enables the radar display of these craft along with distance and altitude information. It is intended to aid buddies flying together FPV. It currently requires a bi-directional serial link for both crafts. This is because each craft must not only report their current position, but be told other crafts positions. This rules out LTM as it currently works in the iNav ecosystem.

Craft Services is written in portable C++. It is currently Windows-only, but Android, Mac and Linux ports are absolutely possible. There is a very small amount of Windows specific code, but all output and serial processing is done with async cross-platform C++ libraries so none of this should present serious issues for porting.

Craft Services is released under GPL V3 (Gnu Public Library Version 3) or later.

## Current Status

As of October 20, 2018, Craft Services comes heartbreakingly close to being useful, but is still hamstrung by low-reliability links and/or Craft Services inexpert handling of them. I am currently unsure if it can be made to work properly with existing commercial hardware (Crossfire, Dragonlink). Some days I'm quite discouraged about the whole thing, and others I thiink it's just one long-range radio firmware upgrade / magic serial port C++ handling trick away from being useful. It seems close enough to working that I am publicizing it early to hopefully get some help with things, and to make others aware that the work exists, even if it isn't immediately completely useful. This project needs help, not hype, but if there are people out there interested enough to participate in some way I want to reach them.

## Current Status Details (10/20/2018)

* With hardwired serial links - i.e. normal USB cables connected directly to the flight controller - Craft Services works very well, with no errors or halting.
* With TBS Crossfire's RX<-> bluetooth bi-directional serial link, performance is decent at close range, but fails completely at anything over 10-30 meters. I am not alone in finding the link to be completely useless at range. (Note that we are specifically talking about the bi-directional Crossfire serial link, NOT emulated Mavlink or anything else)
* Dragonlink's Rx<->bluetooth bi-directional serial link works quite a bit better than TBS Crossfire's. With a single radio involved, Craft Services can work quite well. Here's a demo of the "Phantom Wingman" mode of CraftServices. 

[Command line string]

[Video]

[Diagram]

In this mode, a single craft is flying with a single radio transmitting. The craft sends its current position, altitude, and course down to the CraftServices application, which then uses that information to relay back up to the craft a phantom wingman that tracks the real craft's current position, altitude, and course at pre-defined offsets. In the video above the phantom wingman craft is intended to be 100 meters directly in front of the current craft, at the same altitude. Since there is some latency, you can see that the phantom is always at least a few meters off in altitude, and about 80 meters ahead instead of 100. These ~20 meters difference correspond to the delay in transmitting the packet down to the ground, and then back up to the craft (and, yes, a relatively minscule amount of CPU processing time as well at both ends).

In my opinion this test shows great promise.

* With two Dragonlink radios running to two Dragonlink equiped crafts, results are mixed and highly frustrating. On the ground/in the lab I find that sometimes Dragonlink will run nearly flawlessly for 40+ minutes, until finally I have to disconnect batteries from the test crafts because they are running low. At other times, results are shaky. The typical failure mode is we stop hearing back from typically ONE of the test craft after a time (30 seconds to 3 minutes), and no matter how many times we request the current GPS position, we never hear it again. Without a GPS position from one of the crafts CraftServices, CraftServices currently will restart and try again. This is surely not the best strategy but this is where I am today.

The failures seem capricious and episodic. Sometimes things go great for a long while, and then abruptly a useful connection can't even be started, even at close range, even after rebooting everything repeatedly. I have carefully kept all variables the same (same parameters, gear, physical location, etc, etc.) and yet results vary widely.

In the absence of real understanding the mind resorts to voodoo mysticism, and the scientific method breaks down. That said, my overall impression is that the radios are interfering with each other. My best guess is that we have two radios both shouting at the same time at distant crafts, and then failing to hear the relatively weak reply signal of one of them over the shouts of the other radio. I would like to hear what others think.

## What about Mavlink? 

Mavlink 

Both TBS Crossfire and Dragonlink offer versions of an emulated Mavlink. [ more on why emulated is good ]

In the medium term, I can see a role for Mavlink being a way to get position information from another craft. In fact, I think it would be possible for there to be a asymetric relationship where one craft was running Mavlink -> Craft Services on the ground, and another was running Craft Services <-> iNav. Only the iNav side would see the position of the other craft, but if it proved reliable that might be worth doing.

[ More to say here ]

## What's new on the iNav side?

[ Much more to say here ]
