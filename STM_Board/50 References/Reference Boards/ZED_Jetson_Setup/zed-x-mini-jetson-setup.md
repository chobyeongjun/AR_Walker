---
title: "ZED X Mini on Jetson: What Nobody Tells You"
date: 2026-04-05
draft: false
tags: ["jetson", "zed", "gmsl2", "robotics", "embedded"]
description: "1 month of failures across two boards, three JetPack versions, and one CSI cable that should never have existed — here's the working setup."
author: "Byeongjun"
---

I spent over a month trying to get the ZED X Mini working on a Jetson-based system. The documentation looks clean. The SDK is well-maintained. And yet, almost nothing worked the way I expected. This post is everything I wish I'd known before starting.

## 1. The board compatibility trap

My first attempt used a **reComputer J4012** from Seeed Studio — a Jetson Orin NX-based board that, on paper, seemed perfectly fine. It runs Jetson Linux, it has a CSI interface, and the ZED X Mini connects via GMSL2 with a capture card.

> **Hard lesson:** The ZED X Mini and the Stereolabs GMSL2 capture card have implicit hardware dependencies that go beyond "does it run Jetson Linux." The reComputer J4012's CSI implementation did not play nicely with the ZED Link capture card — at all.

The symptom was silent: no error messages, no kernel panics. Just nothing. Running `i2cdetect` on every bus returned empty tables. The camera simply did not exist as far as the system was concerned.

## 2. The CSI cable that goes nowhere

The reComputer J4012 uses a **15-pin CSI connector**, while the standard GMSL2 capture card uses a 22-pin interface. So I ordered a 22-to-15 FPC adapter cable and tried to bridge the gap.

> **Don't do this.** The 22-to-15 CSI adapter cable does not work. It's not a pin-count mismatch you can cable your way around — the signal routing and lane assignments differ between connector generations. Even if the physical connection seats correctly, the I2C control channel for the GMSL2 deserializer won't come up.

I spent considerable time probing the I2C buses with different address scans (`i2cdetect -y -r 9`, `-r 10`, etc.) hoping to see the deserializer enumerate. Nothing appeared. The adapter cable is a dead end.

## 3. The JetPack version roulette

After accepting that the J4012 wasn't going to work, I switched to a board with a proper **22-pin CSI connector** and DisplayPort output — closer to the official NVIDIA Orin NX developer kit layout.

The community consensus at the time was that ZED SDK and ZED Link only support up to JetPack 6.1. So I spent two weeks flashing and reflashing 6.1. It never worked — the flashing process itself kept failing.

> **The myth:** "ZED X Mini only works up to JetPack 6.1" — this is wrong, or at least outdated.

Out of frustration, I tried **JetPack 6.2.2**. It flashed successfully. Then I tried 6.2.0 — it failed. Then **6.2.1 — it worked.** I've been on 6.2.1 since, without issues.

The working combination:

| Component | Version |
|-----------|---------|
| JetPack | 6.2.1 (L4T 36.4.0) |
| ZED SDK | 5.2.1 |
| ZED Link | 1.4.0-L4T36.4.0 |
| Board | Waveshare Jetson Orin NX carrier (22-pin CSI) |

## 4. The moment it works: i2cdetect

After flashing JetPack 6.2.1 and installing ZED Link, I ran `i2cdetect` again. This time, addresses actually appeared on the bus.

```bash
sudo i2cdetect -y -r 9
```

Addresses on the I2C bus = the GMSL2 deserializer is alive. This is the first real confirmation that the capture card and camera are communicating. If you see this, you're past the hardware barrier.

From there, launching ZED Explorer immediately showed the camera feed. No additional configuration needed.

## 5. Final working stack

```bash
sudo apt install zed-link
sudo apt install zed-sdk
zed-explorer  # verify camera feed
```

**Summary of what actually works:**
- Use a board with a **native 22-pin CSI connector** (not adapted)
- Flash **JetPack 6.2.1** via SDK Manager
- Install ZED SDK 5.2.1 + ZED Link 1.4.0-L4T36.4.0
- Skip JetPack 6.1 entirely — the community reports are outdated

The reComputer J4012 and the 22-to-15 CSI adapter are not viable paths. If you're starting fresh, go directly to 6.2.1.

---

*This post documents a real integration attempt on research robotics hardware. Hardware was purchased and tested independently — no affiliation with Stereolabs or NVIDIA.*
