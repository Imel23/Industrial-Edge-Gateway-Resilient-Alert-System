# Industrial Edge Gateway & Resilient Alert System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Issues](https://img.shields.io/github/issues/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System)](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/issues)
[![Build and Test](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/actions/workflows/build.yml/badge.svg)](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/actions/workflows/build.yml)

This repository contains the final project for the Advanced Embedded Linux Development course.

## 🎯 Project Vision

Build an industrial-grade IIoT gateway for critical environment monitoring, offering **high reliability**, **resilience to failures**, and a **modern control interface**.

## 📋 Project Architecture

This project implements a complete IIoT (Industrial IoT) gateway on the **BeagleBone Black** using **Yocto Scarthgap**. It features:

- **Custom Linux Kernel Modules (LKM)** for hardware control
- **Multithreaded C++ system daemon** for orchestration
- **MQTT-based telemetry interface** for cloud connectivity
- **React dashboard** for real-time monitoring and control

## 🏗️ System Architecture

The system follows a **layered architecture**:

```
┌─────────────────────────────────────────────────────────┐
│                    User Space                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ React UI     │  │ Python API   │  │ MQTT Broker  │  │
│  │ (Dashboard)  │◄─┤ (FastAPI)    │◄─┤ (Telemetry)  │  │
│  └──────────────┘  └──────┬───────┘  └──────────────┘  │
│                           │                             │
│                    ┌──────▼───────┐                     │
│                    │ C++ Daemon   │                     │
│                    │ (Multithread)│                     │
│                    └──────┬───────┘                     │
│                           │ Unix Domain Socket          │
├───────────────────────────┼─────────────────────────────┤
│                    Kernel Space                         │
│  ┌──────────────┐  ┌──────▼───────┐  ┌──────────────┐  │
│  │ LED Driver   │  │ Button Driver│  │ DHT11 Driver │  │
│  │ (LKM)        │  │ (IRQ/LKM)    │  │ (IIO)        │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │
├─────────┼──────────────────┼──────────────────┼─────────┤
│                    Hardware Layer                       │
│  ┌──────▼───────┐  ┌──────▼───────┐  ┌──────▼───────┐  │
│  │ LED (GPIO)   │  │ Button (GPIO)│  │ DHT11 Sensor │  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
│              BeagleBone Black (AM335x)                  │
└─────────────────────────────────────────────────────────┘
```

## 🚀 Quick Links

➡️ [**Project Overview Wiki Page**](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/wiki/Project-Overview-%E2%80%90-Industrial-Edge-Gateway-&-Resilient-Alert-System)

➡️ [**Project Roadmap & Schedule**](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/wiki/Schedule)

➡️ [**GitHub Issues (User Stories)**](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/issues)

## 📦 Key Features

### Hardware Layer
- **BeagleBone Black** (AM335x ARM Cortex-A8)
- **DHT11** temperature and humidity sensor
- **LED** for visual alerts
- **Button** for user input

### Kernel Space
- Custom **Device Tree Overlay** for hardware configuration
- **LED Driver** (Loadable Kernel Module with timer support)
- **Button Driver** (IRQ-based with debouncing)
- **DHT11 Driver** (IIO subsystem integration)

### User Space
- **C++ System Daemon** (multithreaded, Unix Domain Socket)
- **Python API** (FastAPI with REST endpoints)
- **MQTT Bridge** (telemetry publishing)
- **React Dashboard** (real-time monitoring and control)

### DevOps
- **GitHub Actions CI/CD** (automated builds and tests)
- **QEMU Support** (for rapid development and testing)
- **Yocto Project** (custom Linux distribution)

## 🗂️ Repository Structure

```
.
├── meta-industrial-gateway/    # Custom Yocto layer
│   ├── recipes-kernel/         # Kernel module recipes
│   ├── recipes-core/           # Core system recipes
│   └── recipes-app/            # Application recipes
├── src/
│   ├── drivers/                # Kernel drivers (LED, Button, DHT11)
│   ├── daemon/                 # C++ system daemon
│   ├── api/                    # Python FastAPI application
│   └── ui/                     # React dashboard
├── .github/
│   └── workflows/              # CI/CD pipelines
├── docs/                       # Additional documentation
├── LICENSE                     # MIT License
└── README.md                   # This file
```

## 🛠️ Technology Stack

| Component | Technology |
| :-- | :-- |
| **OS** | Yocto Project (Scarthgap 5.0 LTS) |
| **Hardware** | BeagleBone Black (AM335x) |
| **Kernel** | Linux Kernel (LKM, IIO Subsystem) |
| **Daemon** | C++ (Multithreading, POSIX) |
| **API** | Python (FastAPI) |
| **Frontend** | React (Vite, TailwindCSS) |
| **Communication** | MQTT (Paho), Unix Domain Socket |
| **CI/CD** | GitHub Actions, Docker |
| **Testing** | QEMU, pytest, gtest |

## 📊 Project Roadmap

The project is organized into **4 Epics** across **3 Sprints**:

| Epic | Description | Sprint |
| :-- | :-- | :-- |
| **E01: System Foundation & Hardware** | Yocto image, Device Tree, kernel drivers | Sprint 1 |
| **E02: Embedded Intelligence & Daemon** | IRQ handling, timers, C++ daemon | Sprints 2-3 |
| **E03: Connectivity & User Interface** | API, MQTT, React dashboard | Sprints 2-3 |
| **E04: Validation, Security & CI/CD** | Testing, security, automation | All sprints |

**Current Status:** Sprint 1 - System Foundation & Hardware

See the [**detailed roadmap**](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/wiki/Schedule) for more information.

## 🎓 Learning Objectives

This project demonstrates advanced embedded Linux development skills:

- ✅ Custom Linux distribution creation with Yocto
- ✅ Kernel module development (character drivers, platform drivers)
- ✅ Hardware interrupt handling (IRQ)
- ✅ Kernel timers (hrtimer)
- ✅ IIO subsystem integration
- ✅ Device Tree Overlay creation
- ✅ Multithreaded system programming (C++)
- ✅ Inter-process communication (Unix Domain Socket)
- ✅ REST API development (FastAPI)
- ✅ MQTT protocol implementation
- ✅ Modern web development (React)
- ✅ CI/CD pipeline setup (GitHub Actions)

## 🚦 Getting Started

### Prerequisites

- Linux development machine (Ubuntu 22.04 recommended)
- BeagleBone Black (or QEMU for emulation)
- Yocto build dependencies
- Git, Python 3.11+, Node.js 18+

### Build Instructions

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System.git
   cd Industrial-Edge-Gateway-Resilient-Alert-System
   ```

2. **Set up Yocto environment:**
   ```bash
   # Follow instructions in docs/yocto-setup.md
   ```

3. **Build the image:**
   ```bash
   bitbake core-image-industrial-gateway
   ```

4. **Generate SDK:**
   ```bash
   bitbake core-image-industrial-gateway -c populate_sdk
   ```

5. **Flash to BeagleBone Black or run on QEMU:**
   ```bash
   runqemu qemuarm
   ```

See the [**Wiki**](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/wiki) for detailed build and deployment instructions.

## 🤝 Contributing

This is an educational project. Contributions, issues, and feature requests are welcome!

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Imel23**

- GitHub: [@Imel23](https://github.com/Imel23)

## 🙏 Acknowledgments

- **Yocto Project** for the build system
- **BeagleBoard.org** for the hardware platform
- **Open Source Community** for the amazing tools and libraries

---

⭐ **Star this repository** if you find it useful!

📚 **Check the [Wiki](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/wiki)** for detailed documentation.

🐛 **Report issues** on the [Issues page](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System/issues).
