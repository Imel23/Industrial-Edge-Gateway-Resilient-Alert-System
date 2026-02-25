# ⚙️ Yocto SDK Guide — Cross-Compiling for Your Target

> **Audience:** Developers who have a working Yocto build and want to compile applications for their target board **without rebuilding the entire image every time**.
>
> **Goal:** By the end of this guide you will know how to generate, install, and use the Yocto SDK to cross-compile, debug, and deploy applications to your embedded target.

---

## Table of Contents

1. [What Is the Yocto SDK?](#1-what-is-the-yocto-sdk)
2. [Standard SDK vs Extensible SDK](#2-standard-sdk-vs-extensible-sdk)
3. [Prerequisites](#3-prerequisites)
4. [Generating the SDK](#4-generating-the-sdk)
5. [Installing the SDK](#5-installing-the-sdk)
6. [Using the SDK to Cross-Compile](#6-using-the-sdk-to-cross-compile)
7. [Cross-Compiling a CMake Project](#7-cross-compiling-a-cmake-project)
8. [Cross-Compiling an Autotools Project](#8-cross-compiling-an-autotools-project)
9. [Deploying to the Target](#9-deploying-to-the-target)
10. [Remote Debugging with GDB](#10-remote-debugging-with-gdb)
11. [Using the Extensible SDK (eSDK)](#11-using-the-extensible-sdk-esdk)
12. [Integrating with IDEs](#12-integrating-with-ides)
13. [Rebuilding / Updating the SDK](#13-rebuilding--updating-the-sdk)
14. [Common Mistakes & Troubleshooting](#14-common-mistakes--troubleshooting)
15. [Quick Reference Cheat Sheet](#15-quick-reference-cheat-sheet)

---

## 1. What Is the Yocto SDK?

The Yocto **Software Development Kit (SDK)** is a self-contained toolchain installer that gives you everything you need to **cross-compile** applications for a target architecture on your development host.

### The Problem It Solves

Without an SDK, every code change requires:

```
Edit code → Add to recipe → bitbake image → Flash → Test     (slow — minutes to hours)
```

With an SDK, the workflow becomes:

```
Edit code → Cross-compile → scp to target → Test             (fast — seconds)
```

### What's Inside the SDK?

| Component                    | Description                                                     |
| ---------------------------- | --------------------------------------------------------------- |
| **Cross-compiler**           | GCC configured for your target architecture (e.g., ARM, MIPS).  |
| **Cross-linker**             | Linker configured with target sysroot paths.                    |
| **Target sysroot**           | Headers and libraries from your image (libc, OpenSSL, etc.).    |
| **Native sysroot**           | Host tools needed for building (pkg-config, cmake, etc.).       |
| **Environment setup script** | A single script that configures all paths and variables.        |

> **💡 Key insight:** The SDK's sysroot mirrors the libraries available on your target image. If a library is in your image, it's available for linking in the SDK.

---

## 2. Standard SDK vs Extensible SDK

Yocto offers two types of SDKs:

| Feature                     | Standard SDK                        | Extensible SDK (eSDK)                |
| --------------------------- | ----------------------------------- | ------------------------------------ |
| **Size**                    | Smaller (~500 MB – 2 GB)           | Larger (~1 GB – 5 GB+)              |
| **Use case**                | Traditional cross-compilation       | Iterative development with BitBake   |
| **Can run `devtool`?**      | ❌ No                              | ✅ Yes                               |
| **Can add new recipes?**    | ❌ No                              | ✅ Yes (via `devtool add`)           |
| **Can modify recipes?**     | ❌ No                              | ✅ Yes (via `devtool modify`)        |
| **Can update the SDK?**     | ❌ Regenerate entirely             | ✅ Incremental updates               |
| **Offline capable?**        | ✅ Fully self-contained            | ⚠️ Some features need network        |
| **Best for**                | Application developers              | BSP/platform developers              |

> **Recommendation:** Start with the **Standard SDK** for application development. Use the **Extensible SDK** if you need to create or modify BitBake recipes outside the full build environment.

---

## 3. Prerequisites

Before generating the SDK, you need:

- ✅ A working Yocto build environment (Poky cloned, `oe-init-build-env` sourced).
- ✅ A **successful image build** — the SDK uses the image to know which libraries to include.
- ✅ Your custom layer added to the build (e.g., `meta-industrial-gateway`).

```bash
# Make sure your image builds first
source oe-init-build-env
bitbake industrial-gateway-image
```

---

## 4. Generating the SDK

### 4.1 Standard SDK

From your **build directory**, run:

```bash
bitbake industrial-gateway-image -c populate_sdk
```

This generates a self-extracting shell script installer at:

```
build/tmp/deploy/sdk/poky-glibc-x86_64-industrial-gateway-image-cortexa8hf-neon-beaglebone-yocto-toolchain-<version>.sh
```

> **⏱️ Note:** SDK generation can take 10–30 minutes depending on your image size and machine.

### 4.2 Extensible SDK (eSDK)

```bash
bitbake industrial-gateway-image -c populate_sdk_ext
```

The eSDK installer will be saved in the same `tmp/deploy/sdk/` directory with an `ext` suffix.

### What the Task Does Behind the Scenes

1. Collects all cross-compilation tools for the target architecture.
2. Copies all headers and libraries from the target image sysroot.
3. Packages host-side native tools (pkg-config, cmake wrappers, etc.).
4. Bundles everything into a portable `.sh` installer.

---

## 5. Installing the SDK

### 5.1 Run the Installer

```bash
# Navigate to the SDK output directory
cd build/tmp/deploy/sdk/

# Run the installer (the exact filename will vary)
./poky-glibc-x86_64-industrial-gateway-image-cortexa8hf-neon-beaglebone-yocto-toolchain-*.sh
```

You will be prompted for an installation directory:

```
Poky (Yocto Project Reference Distro) SDK installer version X.X
================================================================
Enter target directory for SDK (default: /opt/poky/X.X):
```

### 5.2 Choose the Installation Path

| Option                | Path                   | Notes                                  |
| --------------------- | ---------------------- | -------------------------------------- |
| **Default**           | `/opt/poky/<version>/` | Requires `sudo` / root permissions.    |
| **User-local**        | `~/yocto-sdk/`         | No root needed. Recommended for dev.   |
| **Project-specific**  | `./sdk/`               | Keeps the SDK alongside your project.  |

Example with a custom path:

```bash
./poky-glibc-x86_64-industrial-gateway-image-*.sh -d ~/yocto-sdk
```

> **💡 Tip:** Use the `-y` flag to auto-accept the license:
>
> ```bash
> ./poky-glibc-x86_64-industrial-gateway-image-*.sh -d ~/yocto-sdk -y
> ```

### 5.3 Verify the Installation

```bash
ls ~/yocto-sdk/
```

You should see:

```
environment-setup-cortexa8hf-neon-poky-linux-gnueabi   # Setup script
site-config-cortexa8hf-neon-poky-linux-gnueabi         # Site config
sysroots/                                               # Target + native sysroots
version-cortexa8hf-neon-poky-linux-gnueabi              # Version info
```

---

## 6. Using the SDK to Cross-Compile

### 6.1 Source the Environment

**Every time** you open a new terminal, source the SDK environment setup script:

```bash
source ~/yocto-sdk/environment-setup-cortexa8hf-neon-poky-linux-gnueabi
```

This sets up all the critical environment variables:

| Variable       | Value (example)                                                 | Purpose                        |
| -------------- | --------------------------------------------------------------- | ------------------------------ |
| `$CC`          | `arm-poky-linux-gnueabi-gcc --sysroot=...`                     | C cross-compiler               |
| `$CXX`         | `arm-poky-linux-gnueabi-g++ --sysroot=...`                     | C++ cross-compiler             |
| `$LD`          | `arm-poky-linux-gnueabi-ld --sysroot=...`                      | Cross-linker                   |
| `$AR`          | `arm-poky-linux-gnueabi-ar`                                    | Archiver                       |
| `$CFLAGS`      | `-O2 -pipe -g -feliminate-unused-debug-types`                  | C compiler flags               |
| `$LDFLAGS`     | `--sysroot=... -Wl,-O1 -Wl,--hash-style=gnu`                  | Linker flags                   |
| `$SDKTARGETSYSROOT` | `~/yocto-sdk/sysroots/cortexa8hf-neon-poky-linux-gnueabi` | Target sysroot path            |
| `$OECORE_NATIVE_SYSROOT` | `~/yocto-sdk/sysroots/x86_64-pokysdk-linux`          | Native (host) sysroot path     |

### 6.2 Verify the Cross-Compiler

```bash
$CC --version
```

Expected output (example):

```
arm-poky-linux-gnueabi-gcc (GCC) 13.x.x
...
```

To confirm you're targeting the right architecture:

```bash
echo $ARCH
echo $CROSS_COMPILE
```

### 6.3 Cross-Compile a Simple C Program

Create a file called `hello.c`:

```c
#include <stdio.h>

int main() {
    printf("Hello from the Yocto SDK!\n");
    return 0;
}
```

Compile it:

```bash
$CC $CFLAGS $LDFLAGS -o hello hello.c
```

Verify the binary is for the target architecture:

```bash
file hello
```

Expected output:

```
hello: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked,
interpreter /lib/ld-linux-armhf.so.3, ...
```

> **⚠️ Important:** Always use `$CC` (not `gcc`) and pass `$CFLAGS` and `$LDFLAGS`. These variables contain the correct sysroot and architecture flags.

---

## 7. Cross-Compiling a CMake Project

The SDK automatically configures CMake through a **toolchain file**.

### 7.1 Using the SDK's Toolchain File

```bash
# Source the SDK first
source ~/yocto-sdk/environment-setup-cortexa8hf-neon-poky-linux-gnueabi

# Configure with CMake (the SDK's env vars handle the toolchain)
cmake -S . -B build \
      -DCMAKE_INSTALL_PREFIX=/usr

# Build
cmake --build build

# (Optional) Install to a staging directory
DESTDIR=./install cmake --install build
```

### 7.2 Example CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(gateway-monitor C)

add_executable(gateway-monitor
    src/main.c
    src/sensor_reader.c
    src/alert_handler.c
)

target_link_libraries(gateway-monitor PRIVATE pthread)

install(TARGETS gateway-monitor DESTINATION ${CMAKE_INSTALL_BINDIR})
```

### 7.3 What Happens Automatically

When the SDK environment is sourced, CMake automatically picks up:

- `CMAKE_C_COMPILER` → `$CC`
- `CMAKE_CXX_COMPILER` → `$CXX`
- `CMAKE_SYSROOT` → `$SDKTARGETSYSROOT`
- `CMAKE_FIND_ROOT_PATH` → Target sysroot (so `find_package()` finds target libraries)

> **💡 Tip:** You do **not** need to write a custom toolchain file — the SDK sets the `$OE_CMAKE_TOOLCHAIN_FILE` variable that CMake uses automatically through the environment.

---

## 8. Cross-Compiling an Autotools Project

For Autotools-based projects (`./configure && make`):

```bash
# Source the SDK
source ~/yocto-sdk/environment-setup-cortexa8hf-neon-poky-linux-gnueabi

# Configure with the cross-compile host triplet
./configure ${CONFIGURE_FLAGS} --prefix=/usr

# Build
make

# Install to staging directory
make install DESTDIR=$(pwd)/install
```

The SDK sets `$CONFIGURE_FLAGS` to include `--host=arm-poky-linux-gnueabi` and other flags needed for cross-compilation.

---

## 9. Deploying to the Target

Once you've cross-compiled your binary, deploy it to the target board:

### 9.1 Using `scp` (Most Common)

```bash
# Copy the binary to the target board
scp hello root@<target-ip>:/usr/bin/

# SSH into the target and run it
ssh root@<target-ip>
hello
```

### 9.2 Using `rsync` (For Larger Deployments)

```bash
# Sync an entire directory to the target
rsync -avz ./install/ root@<target-ip>:/
```

### 9.3 Using NFS (For Rapid Iteration)

For the fastest development cycle, mount an NFS share on the target:

```bash
# On the host: export a directory via NFS
sudo echo "/home/user/nfs-share *(rw,sync,no_subtree_check,no_root_squash)" >> /etc/exports
sudo exportfs -ra

# On the target: mount the NFS share
mount -t nfs <host-ip>:/home/user/nfs-share /mnt/nfs
```

Then simply compile on the host and run from `/mnt/nfs` on the target — no copying needed!

---

## 10. Remote Debugging with GDB

The SDK includes a cross-architecture GDB for debugging applications running on the target.

### 10.1 On the Target — Start GDB Server

```bash
# Install gdbserver if not already present in your image
# (add "gdbserver" to IMAGE_INSTALL in your image recipe)

# Run your application under gdbserver
gdbserver :9090 /usr/bin/hello
```

### 10.2 On the Host — Connect with Cross-GDB

```bash
# Source the SDK environment
source ~/yocto-sdk/environment-setup-cortexa8hf-neon-poky-linux-gnueabi

# Launch the cross-GDB
$GDB hello

# Inside GDB, connect to the target
(gdb) target remote <target-ip>:9090
(gdb) break main
(gdb) continue
```

### 10.3 Required Image Changes for Debugging

To enable `gdbserver` on your target, add it to your image recipe:

```bitbake
IMAGE_INSTALL:append = " \
    gdbserver \
"
```

For full debug symbols, add to `local.conf`:

```bitbake
# Keep debug symbols in packages
DEBUG_BUILD = "1"
INHIBIT_PACKAGE_STRIP = "1"
```

> **⚠️ Warning:** Debug builds produce significantly larger images. Only enable this during development.

---

## 11. Using the Extensible SDK (eSDK)

The eSDK includes everything from the standard SDK **plus** `devtool`, a powerful command-line tool for recipe development.

### 11.1 Install the eSDK

```bash
cd build/tmp/deploy/sdk/
./poky-glibc-x86_64-industrial-gateway-image-*-ext-*.sh -d ~/yocto-esdk -y
```

### 11.2 Source the eSDK Environment

```bash
source ~/yocto-esdk/environment-setup-cortexa8hf-neon-poky-linux-gnueabi
```

### 11.3 Key `devtool` Commands

#### Add a New Recipe from Source

```bash
# Create a recipe from a Git repository
devtool add myapp https://github.com/user/myapp.git

# Edit the source code
cd workspace/sources/myapp
# ... make changes ...

# Build the recipe
devtool build myapp

# Deploy directly to a running target
devtool deploy-target myapp root@<target-ip>
```

#### Modify an Existing Recipe

```bash
# Extract and patch an existing recipe for editing
devtool modify busybox

# Make your modifications
cd workspace/sources/busybox
# ... edit code ...

# Build and test
devtool build busybox

# When satisfied, create a bbappend in your layer
devtool finish busybox ../meta-industrial-gateway
```

#### Update an Existing Recipe to a New Version

```bash
devtool upgrade myapp --version 2.0
```

### 11.4 `devtool` Workflow Diagram

```
┌──────────────────────────────────────────────────────┐
│                     devtool workflow                  │
│                                                      │
│   devtool add/modify                                 │
│         │                                            │
│         ▼                                            │
│   ┌──────────┐     ┌──────────┐     ┌────────────┐  │
│   │  Source   │────▶│  Build   │────▶│  Deploy    │  │
│   │  (edit)   │     │  (test)  │     │  (target)  │  │
│   └──────────┘     └──────────┘     └────────────┘  │
│         │                                  │         │
│         │            iterate               │         │
│         ◀──────────────────────────────────┘         │
│         │                                            │
│         ▼                                            │
│   devtool finish → bbappend in your meta layer       │
└──────────────────────────────────────────────────────┘
```

---

## 12. Integrating with IDEs

### 12.1 VS Code

1. **Source the SDK** in your terminal before launching VS Code:

   ```bash
   source ~/yocto-sdk/environment-setup-cortexa8hf-neon-poky-linux-gnueabi
   code .
   ```

2. **Configure CMake** in `.vscode/settings.json`:

   ```json
   {
       "cmake.configureEnvironment": {
           "SDKTARGETSYSROOT": "${env:SDKTARGETSYSROOT}"
       },
       "cmake.configureArgs": [
           "-DCMAKE_INSTALL_PREFIX=/usr"
       ]
   }
   ```

3. **Configure IntelliSense** in `.vscode/c_cpp_properties.json`:

   ```json
   {
       "configurations": [
           {
               "name": "Yocto SDK",
               "compilerPath": "${env:CC}",
               "includePath": [
                   "${workspaceFolder}/**",
                   "${env:SDKTARGETSYSROOT}/usr/include/**"
               ],
               "defines": [],
               "cStandard": "c17",
               "intelliSenseMode": "linux-gcc-arm"
           }
       ],
       "version": 4
   }
   ```

### 12.2 CLion

1. Source the SDK environment before launching CLion.
2. Go to **Settings → Build, Execution, Deployment → CMake**.
3. Set the environment variables or point to the SDK's toolchain file:
   ```
   CMAKE_TOOLCHAIN_FILE=$OE_CMAKE_TOOLCHAIN_FILE
   ```

### 12.3 Eclipse (Yocto Plugin)

The Yocto Project provides an official Eclipse plugin:

1. Install the **Eclipse IDE for Embedded C/C++ Developers**.
2. Install the Yocto SDK plugin from the Eclipse Marketplace.
3. Point the plugin to your SDK installation directory.
4. Create a new project using the Yocto SDK template.

---

## 13. Rebuilding / Updating the SDK

### When to Regenerate the SDK

You need to regenerate the SDK when:

- ✅ You **add new libraries** to your image (e.g., adding `openssl-dev` to `IMAGE_INSTALL`).
- ✅ You **change the target machine** (e.g., from `qemuarm` to `beaglebone-yocto`).
- ✅ You **upgrade Yocto / Poky** to a new release.
- ✅ You **update a library version** that your application depends on.

### Regenerate and Reinstall

```bash
# Rebuild the SDK
bitbake industrial-gateway-image -c populate_sdk

# Remove the old SDK
rm -rf ~/yocto-sdk

# Install the new SDK
cd build/tmp/deploy/sdk/
./poky-glibc-x86_64-industrial-gateway-image-*.sh -d ~/yocto-sdk -y
```

### For the eSDK — Incremental Update

```bash
# Source the eSDK environment
source ~/yocto-esdk/environment-setup-*

# Update the eSDK without full reinstall
devtool sdk-update
```

---

## 14. Common Mistakes & Troubleshooting

### ❌ Error: "command not found" when running `$CC`

**Cause:** You forgot to source the SDK environment.

**Fix:**
```bash
source ~/yocto-sdk/environment-setup-cortexa8hf-neon-poky-linux-gnueabi
```

### ❌ Error: "cannot find -lmylibrary"

**Cause:** The library is not in the SDK sysroot. It may be missing from your image.

**Fix:**
1. Add the library's `-dev` package to your image:
   ```bitbake
   IMAGE_INSTALL:append = " mylibrary-dev"
   ```
2. Regenerate the SDK:
   ```bash
   bitbake industrial-gateway-image -c populate_sdk
   ```
3. Reinstall the SDK.

### ❌ Binary runs on host but crashes on target

**Cause:** You compiled with the host `gcc` instead of `$CC`.

**Fix:** Always verify with `file`:
```bash
file mybinary
# Should show ARM / MIPS / etc., NOT x86-64
```

### ❌ Error: "no such file or directory" when running binary on target

**Cause:** The dynamic linker path doesn't match. Usually from mixing SDK versions or stripping sysroot info.

**Fix:** Recompile using the SDK's `$CC` with `$LDFLAGS` (which contains the correct `--sysroot`).

### ❌ CMake ignores the SDK cross-compiler

**Cause:** CMake was configured before sourcing the SDK, or a `CMakeCache.txt` is leftover from a previous host build.

**Fix:**
```bash
# Delete the old build directory
rm -rf build/

# Source the SDK, then re-run cmake
source ~/yocto-sdk/environment-setup-*
cmake -S . -B build
```

### ❌ "Incompatible SDK — sysroot mismatch"

**Cause:** The SDK was generated for a different image or machine than what's on the target.

**Fix:** Regenerate the SDK from the same image you flash:
```bash
bitbake <your-image> -c populate_sdk
```

---

## 15. Quick Reference Cheat Sheet

### SDK Generation & Installation

| Task                          | Command                                                           |
| ----------------------------- | ----------------------------------------------------------------- |
| Generate Standard SDK         | `bitbake <image> -c populate_sdk`                                |
| Generate Extensible SDK       | `bitbake <image> -c populate_sdk_ext`                            |
| Install SDK                   | `./poky-glibc-*.sh -d ~/yocto-sdk -y`                           |
| Source SDK environment        | `source ~/yocto-sdk/environment-setup-*`                         |
| Verify cross-compiler         | `$CC --version`                                                  |
| Check binary architecture     | `file <binary>`                                                  |

### Cross-Compilation

| Task                          | Command                                                           |
| ----------------------------- | ----------------------------------------------------------------- |
| Compile C program             | `$CC $CFLAGS $LDFLAGS -o output source.c`                       |
| Compile C++ program           | `$CXX $CXXFLAGS $LDFLAGS -o output source.cpp`                  |
| CMake configure               | `cmake -S . -B build`                                            |
| CMake build                   | `cmake --build build`                                            |
| Autotools configure           | `./configure ${CONFIGURE_FLAGS}`                                 |
| Check target sysroot          | `echo $SDKTARGETSYSROOT`                                         |
| List available target libs    | `ls $SDKTARGETSYSROOT/usr/lib/`                                  |
| List available target headers | `ls $SDKTARGETSYSROOT/usr/include/`                              |

### Deployment & Debugging

| Task                          | Command                                                           |
| ----------------------------- | ----------------------------------------------------------------- |
| Copy binary to target         | `scp binary root@<ip>:/usr/bin/`                                 |
| Start GDB server (target)     | `gdbserver :9090 /usr/bin/binary`                                |
| Connect cross-GDB (host)      | `$GDB binary` → `target remote <ip>:9090`                       |
| Sync directory to target      | `rsync -avz ./install/ root@<ip>:/`                              |

### devtool (eSDK Only)

| Task                          | Command                                                           |
| ----------------------------- | ----------------------------------------------------------------- |
| Add new recipe from source    | `devtool add <name> <source-url>`                                |
| Modify existing recipe        | `devtool modify <recipe>`                                        |
| Build a recipe                | `devtool build <recipe>`                                         |
| Deploy to target              | `devtool deploy-target <recipe> root@<ip>`                       |
| Finish and create bbappend    | `devtool finish <recipe> <layer-path>`                           |
| Update SDK                    | `devtool sdk-update`                                             |

---

## Summary Workflow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                     SDK Development Workflow                     │
│                                                                 │
│  ┌───────────┐    ┌──────────┐    ┌──────────┐    ┌─────────┐  │
│  │  Generate  │───▶│ Install  │───▶│  Source   │───▶│  Code   │  │
│  │    SDK     │    │   SDK    │    │  env-setup│    │  & Build│  │
│  └───────────┘    └──────────┘    └──────────┘    └────┬────┘  │
│                                                        │        │
│                                                        ▼        │
│                                   ┌──────────┐    ┌─────────┐  │
│                                   │   Test   │◀───│ Deploy  │  │
│                                   │ on target│    │ (scp)   │  │
│                                   └────┬─────┘    └─────────┘  │
│                                        │                        │
│                                        │    iterate             │
│                                        └──────────────▶ Code    │
└─────────────────────────────────────────────────────────────────┘
```

---

> **📝 Note:** This guide is part of the [Industrial Edge Gateway Resilient Alert System](../README.md) project documentation.  
> See also: [Yocto Meta Layer Tutorial](yocto_meta_layer_tutorial.md) · [Fastboot Guide](fastboot_guide.md)
