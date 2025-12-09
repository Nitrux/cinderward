# Cinderward | [![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

A simple, no-nonsense, init-free, Wayland-friendly GUI for **[firewalld](https://firewalld.org/)**.

![Cinderward](https://nxos.org/wp-content/uploads/2025/12/image_2025-12-09_02-27-09-1.jpg)
> Cinderward, a GUI for firewalld.

# Introduction

Cinderwuard is a simple utility built with MauiKit that provides an intuitive interface for managing day-to-day firewall rules without the complexity of firewalld's command-line tooling.

## Features

- View and edit firewall rules by zone  
- Enable or disable:
  - Panic Mode (Lockdown)
  - Masquerading (NAT)
  - Logging of denied packets
  - ICMP reconnaissance blocking
- Add or remove:
  - Services  
  - Ports (TCP/UDP)  
  - Port forwarding rules  
- Real-time synchronization with firewalld via D-Bus  
- Wayland-compatible
- ***Does not require Systemd*** (or other inits to function)
- Built with **Qt 6**, **MauiKit**, and **KF6**
- Optimized for x86-64-v3

Cinderward intentionally does **not** expose advanced or dangerous firewalld features. Its design goal is to remain approachable and safe for everyday users while still being powerful enough for typical workstation and laptop use cases.


## Requirements

- Nitrux 5.1.0 and newer.

### Runtime Requirements

```
firewalld (>= 2.3.1)
mauikit (>= 4.0.2)
qt6 (>= 6.8.2)
kf6-windowsystem (>= 6.13.0)
kf6-i18n (>= 6.13.0)
kf6-coreaddons (>= 6.13.0)
```

# Usage

To use Cinderward, launch it from the applications menu.

# Licensing

The license for this repository and its contents is **BSD-3-Clause**.

# Issues

If you find problems with the contents of this repository, please create an issue and use the **🐞 Bug report** template.

## Submitting a bug report

Before submitting a bug, you should look at the [existing bug reports]([url](https://github.com/Nitrux/cinderward/issues)) to verify that no one has reported the bug already.

©2025 Nitrux Latinoamericana S.C.

