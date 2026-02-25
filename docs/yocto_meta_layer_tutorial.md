# 🧱 Beginner's Guide: Creating & Adding a Meta Layer to Yocto

> **Audience:** Beginners with basic Linux knowledge who are getting started with the Yocto Project.
>
> **Goal:** By the end of this tutorial, you will understand what a meta layer is, how to create one from scratch, and how to integrate it into your Yocto build.

---

## Table of Contents

1. [What Is a Meta Layer?](#1-what-is-a-meta-layer)
2. [Prerequisites](#2-prerequisites)
3. [Understanding the Layer Structure](#3-understanding-the-layer-structure)
4. [Step 1 — Create a New Layer](#4-step-1--create-a-new-layer)
5. [Step 2 — Understand the Generated Files](#5-step-2--understand-the-generated-files)
6. [Step 3 — Add the Layer to Your Build](#6-step-3--add-the-layer-to-your-build)
7. [Step 4 — Create a Simple Recipe](#7-step-4--create-a-simple-recipe)
8. [Step 5 — Create a Custom Image Recipe](#8-step-5--create-a-custom-image-recipe)
9. [Step 6 — Build Your Image](#9-step-6--build-your-image)
10. [Real-World Example: `meta-industrial-gateway`](#10-real-world-example-meta-industrial-gateway)
11. [Common Mistakes & Troubleshooting](#11-common-mistakes--troubleshooting)
12. [Quick Reference Cheat Sheet](#12-quick-reference-cheat-sheet)
13. [Next Steps](#13-next-steps)

---

## 1. What Is a Meta Layer?

In the Yocto Project, a **meta layer** is a directory that contains a set of instructions (called **recipes**) telling the build system **how to build software packages and images**.

Think of layers like _modular building blocks_:

| Concept     | Analogy                                                                 |
| ----------- | ----------------------------------------------------------------------- |
| **Poky**    | The base Yocto distribution — like an operating system starter kit.     |
| **Layer**   | A plugin that adds or modifies functionality on top of the base system. |
| **Recipe**  | A single instruction file (`.bb`) that builds one software component.   |
| **Image**   | A recipe that combines packages into a final flashable OS image.        |

### Why Use Custom Layers?

- ✅ **Separation of concerns** — Keep your customizations separate from the base system.
- ✅ **Reusability** — Reuse your layer across different projects and machines.
- ✅ **Maintainability** — Easily upgrade the base Yocto version without losing your changes.
- ✅ **Collaboration** — Share layers with your team or the open-source community.

> **⚠️ Golden Rule:** Never modify the `poky/` base layer directly. Always create your own layer.

---

## 2. Prerequisites

Before starting, make sure you have:

- **A Linux machine** (Ubuntu 20.04/22.04 recommended).
- **Yocto build dependencies** installed:

```bash
sudo apt update
sudo apt install -y gawk wget git diffstat unzip texinfo gcc build-essential \
  chrpath socat cpio python3 python3-pip python3-pexpect xz-utils debianutils \
  iputils-ping python3-git python3-jinja2 python3-subunit zstd liblz4-tool \
  file locales libacl1
sudo locale-gen en_US.UTF-8
```

- **Poky cloned** (the Yocto reference distribution):

```bash
git clone git://git.yoctoproject.org/poky.git
cd poky
git checkout scarthgap   # or your target release branch
```

- **Build environment initialized**:

```bash
source oe-init-build-env
```

> **Note:** Running `source oe-init-build-env` creates a `build/` directory and drops you into it. You only need to run this once per terminal session.

---

## 3. Understanding the Layer Structure

Every Yocto meta layer follows a standard directory structure:

```
meta-<your-layer-name>/
├── COPYING.MIT              # License file
├── README                   # Layer description
├── conf/
│   └── layer.conf           # ⭐ Layer configuration (REQUIRED)
├── recipes-core/            # Category folder for recipes
│   └── images/
│       └── my-image.bb      # Custom image recipe
├── recipes-apps/            # Another category folder
│   └── my-app/
│       ├── my-app_1.0.bb    # Application recipe
│       └── files/
│           └── my-app.c     # Source files
└── recipes-bsp/             # Board Support Package recipes
    └── ...
```

### Key Points

- The layer **must** have a `conf/layer.conf` file — this is what makes it a valid layer.
- Recipe folders follow the naming convention `recipes-<category>/`.
- Recipe files use the `.bb` extension.
- Append files (`.bbappend`) modify recipes from other layers.

---

## 4. Step 1 — Create a New Layer

Yocto provides a built-in tool to scaffold a new layer. From your **build directory**, run:

```bash
bitbake-layers create-layer ../meta-mylayer
```

This creates a new layer one level up from `build/`, alongside `poky/`.

### What the Command Generates

```
meta-mylayer/
├── COPYING.MIT
├── README
├── conf/
│   └── layer.conf
└── recipes-example/
    └── example/
        └── example_0.1.bb
```

> **💡 Tip:** You can also create the layer manually by creating the directory and files yourself. The tool just saves time.

---

## 5. Step 2 — Understand the Generated Files

### 5.1 `conf/layer.conf` — The Heart of Your Layer

This is the most important file. Let's break it down line by line:

```bitbake
# We have a conf and classes directory, add to BBPATH
BBPATH .= ":${LAYERDIR}"

# We have recipes-* directories, add to BBFILES
BBFILES += "${LAYERDIR}/recipes-*/*/*.bb \
            ${LAYERDIR}/recipes-*/*/*.bbappend"

BBFILE_COLLECTIONS += "meta-mylayer"
BBFILE_PATTERN_meta-mylayer = "^${LAYERDIR}/"
BBFILE_PRIORITY_meta-mylayer = "6"

LAYERDEPENDS_meta-mylayer = "core"
LAYERSERIES_COMPAT_meta-mylayer = "scarthgap"
```

| Variable                       | Purpose                                                                                     |
| ------------------------------ | ------------------------------------------------------------------------------------------- |
| `BBPATH`                       | Adds your layer to BitBake's search path.                                                   |
| `BBFILES`                      | Tells BitBake where to find your `.bb` and `.bbappend` recipe files.                        |
| `BBFILE_COLLECTIONS`           | Registers a unique name for your layer.                                                     |
| `BBFILE_PATTERN_meta-mylayer`  | A regex pattern that maps recipes to this layer.                                            |
| `BBFILE_PRIORITY_meta-mylayer` | Priority level (higher = overrides others). Default `6` is fine for most custom layers.     |
| `LAYERDEPENDS_meta-mylayer`    | Declares dependencies on other layers (e.g., `core` for the base OE-Core layer).           |
| `LAYERSERIES_COMPAT_meta-mylayer` | Which Yocto release series this layer is compatible with (e.g., `scarthgap`, `kirkstone`). |

> **⚠️ Important:** The `LAYERSERIES_COMPAT` value **must match** your Poky branch, or BitBake will warn you about compatibility.

### 5.2 `COPYING.MIT` — License

Contains the MIT license text. You can change this to match your project's licensing.

### 5.3 `README` — Documentation

A plain-text file describing your layer, its dependencies, and how to use it. Keep this updated!

### 5.4 `recipes-example/example/example_0.1.bb` — Sample Recipe

A minimal example recipe to show the structure:

```bitbake
SUMMARY = "bitbake-layers recipe"
DESCRIPTION = "Recipe created by bitbake-layers"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*  Example recipe created by bitbake-layers   *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

addtask display_banner before do_build
```

---

## 6. Step 3 — Add the Layer to Your Build

There are **two ways** to add your layer to the build:

### Option A: Using `bitbake-layers` (Recommended)

From your **build directory**, run:

```bash
bitbake-layers add-layer ../meta-mylayer
```

This automatically edits the `conf/bblayers.conf` file for you.

### Option B: Manual Edit

Open `build/conf/bblayers.conf` and add your layer path to the `BBLAYERS` variable:

```bitbake
BBLAYERS ?= " \
  /home/user/poky/meta \
  /home/user/poky/meta-poky \
  /home/user/poky/meta-yocto-bsp \
  /home/user/meta-mylayer \
"
```

### Verify the Layer Is Registered

```bash
bitbake-layers show-layers
```

You should see your layer listed in the output:

```
layer                 path                                      priority
==========================================================================
meta                  /home/user/poky/meta                      5
meta-poky             /home/user/poky/meta-poky                 5
meta-yocto-bsp        /home/user/poky/meta-yocto-bsp            5
meta-mylayer          /home/user/meta-mylayer                    6
```

---

## 7. Step 4 — Create a Simple Recipe

Let's create a recipe for a simple "Hello World" C application.

### 7.1 Create the Directory Structure

```bash
mkdir -p meta-mylayer/recipes-apps/hello-world/files
```

### 7.2 Write the Source Code

Create `meta-mylayer/recipes-apps/hello-world/files/hello.c`:

```c
#include <stdio.h>

int main() {
    printf("Hello from my custom Yocto layer!\n");
    return 0;
}
```

### 7.3 Write the Recipe

Create `meta-mylayer/recipes-apps/hello-world/hello-world_1.0.bb`:

```bitbake
SUMMARY = "Hello World Application"
DESCRIPTION = "A simple hello world app to demonstrate custom Yocto recipes."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Where to find the source files
SRC_URI = "file://hello.c"

# Source and build directory setup
S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

# Compile the application
do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o hello ${S}/hello.c
}

# Install the binary into the image
do_install() {
    install -d ${D}${bindir}
    install -m 0755 hello ${D}${bindir}/hello
}
```

### Understanding the Recipe Variables

| Variable              | Purpose                                                          |
| --------------------- | ---------------------------------------------------------------- |
| `SUMMARY`             | One-line description of the package.                             |
| `LICENSE`             | The license type (e.g., `MIT`, `GPL-2.0-only`).                 |
| `LIC_FILES_CHKSUM`   | Checksum of the license file for verification.                   |
| `SRC_URI`             | Where to get the source files (`file://` = local, `git://` = repo, `https://` = remote). |
| `S`                   | The source directory where BitBake unpacks/finds source code.    |
| `do_compile()`        | Shell function that compiles the source code.                    |
| `do_install()`        | Shell function that installs files into the staging directory.   |
| `${CC}`               | The cross-compiler (automatically set by Yocto).                 |
| `${D}`                | The destination directory (staging root filesystem).             |
| `${bindir}`           | Expands to `/usr/bin`.                                           |

### 7.4 Test Build the Recipe

```bash
bitbake hello-world
```

If the build succeeds, your recipe is working! 🎉

---

## 8. Step 5 — Create a Custom Image Recipe

An image recipe bundles multiple packages into a complete Linux image.

### 8.1 Create the Directory

```bash
mkdir -p meta-mylayer/recipes-core/images
```

### 8.2 Write the Image Recipe

Create `meta-mylayer/recipes-core/images/my-custom-image.bb`:

```bitbake
SUMMARY = "My Custom Yocto Image"
LICENSE = "MIT"

# Inherit the core-image class (provides base image functionality)
inherit core-image

# Packages to include in the image
IMAGE_INSTALL:append = " \
    packagegroup-core-boot \
    packagegroup-core-ssh-openssh \
    hello-world \
"

# Allow root login without password (development only!)
EXTRA_IMAGE_FEATURES += "debug-tweaks"
```

### Key Image Recipe Concepts

| Concept                  | Explanation                                                        |
| ------------------------ | ------------------------------------------------------------------ |
| `inherit core-image`    | Pulls in all the base functionality for building a Linux image.    |
| `IMAGE_INSTALL:append`  | Adds packages to the image. **Note the leading space** after `"`.  |
| `EXTRA_IMAGE_FEATURES`  | Adds image-level features like `debug-tweaks` or `package-management`. |

> **⚠️ Important Syntax:** When using `:append`, you **must** include a leading space before the first package name. Otherwise, the package name will be concatenated with the previous value.

### 8.3 Build the Image

```bash
bitbake my-custom-image
```

The output image will be in `build/tmp/deploy/images/<machine>/`.

---

## 9. Step 6 — Build Your Image

### Full Build Workflow Summary

```bash
# 1. Source the build environment (once per terminal session)
source oe-init-build-env

# 2. (Optional) Set your target machine in conf/local.conf
#    e.g., MACHINE = "beaglebone-yocto"

# 3. Add your layer (first time only)
bitbake-layers add-layer ../meta-mylayer

# 4. Build the image
bitbake my-custom-image
```

### Build Output Location

```
build/tmp/deploy/images/<machine>/
├── my-custom-image-<machine>.rootfs.wic
├── my-custom-image-<machine>.rootfs.tar.bz2
├── my-custom-image-<machine>.rootfs.ext4
└── ...
```

---

## 10. Real-World Example: `meta-industrial-gateway`

This project includes a real custom layer called **`meta-industrial-gateway`**. Here's how it maps to what you just learned:

### Layer Structure

```
meta-industrial-gateway/
├── COPYING.MIT
├── README
├── conf/
│   └── layer.conf
└── recipes-core/
    └── images/
        └── industrial-gateway-image.bb
```

### `conf/layer.conf`

```bitbake
# We have a conf and classes directory, add to BBPATH
BBPATH .= ":${LAYERDIR}"

# We have recipes-* directories, add to BBFILES
BBFILES += "${LAYERDIR}/recipes-*/*/*.bb \
            ${LAYERDIR}/recipes-*/*/*.bbappend"

BBFILE_COLLECTIONS += "meta-industrial-gateway"
BBFILE_PATTERN_meta-industrial-gateway = "^${LAYERDIR}/"
BBFILE_PRIORITY_meta-industrial-gateway = "6"

LAYERDEPENDS_meta-industrial-gateway = "core"
LAYERSERIES_COMPAT_meta-industrial-gateway = "scarthgap"
```

### `recipes-core/images/industrial-gateway-image.bb`

```bitbake
SUMMARY = "Industrial Gateway OS Image"
LICENSE = "MIT"
inherit core-image

IMAGE_INSTALL:append = " \
    packagegroup-core-boot \
    packagegroup-core-ssh-openssh \
    kernel-modules \
    i2c-tools \
    gpio-utils \
    libstdc++ \
"
# Allows root login without password for development
EXTRA_IMAGE_FEATURES += "debug-tweaks"
```

**Notice how it follows the exact same patterns** from this tutorial — a `layer.conf`, an image recipe inheriting `core-image`, and packages added via `IMAGE_INSTALL:append`.

---

## 11. Common Mistakes & Troubleshooting

### ❌ Error: "Layer is not compatible with the current series"

```
WARNING: Layer meta-mylayer is not compatible with the current set of configured layers
```

**Fix:** Make sure `LAYERSERIES_COMPAT` in your `layer.conf` matches your Poky branch:

```bitbake
# If you're on the scarthgap branch:
LAYERSERIES_COMPAT_meta-mylayer = "scarthgap"
```

### ❌ Error: "Nothing PROVIDES 'my-recipe'"

```
ERROR: Nothing PROVIDES 'hello-world'
```

**Fix:** Check that:
1. Your layer is added to `bblayers.conf` (`bitbake-layers show-layers`).
2. The recipe file is in a `recipes-*/` directory.
3. The `BBFILES` glob in `layer.conf` matches your recipe's path.

### ❌ Error: "QA Issue: No packages were created"

**Fix:** Make sure your `do_install()` function copies files to `${D}`. BitBake won't create packages if nothing is installed.

### ❌ Build fails with "do_compile" error

**Fix:** Check:
- Your source code compiles correctly.
- You're using `${CC}` (cross-compiler), not `gcc` (host compiler).
- `${CFLAGS}` and `${LDFLAGS}` are passed to the compiler.

### ❌ Forgot the leading space in `:append`

```bitbake
# ❌ WRONG — "packagegroup-core-boot" gets concatenated
IMAGE_INSTALL:append = "packagegroup-core-boot"

# ✅ CORRECT — note the space after the opening quote
IMAGE_INSTALL:append = " packagegroup-core-boot"
```

---

## 12. Quick Reference Cheat Sheet

| Task                          | Command                                          |
| ----------------------------- | ------------------------------------------------ |
| Create a new layer            | `bitbake-layers create-layer ../meta-mylayer`    |
| Add a layer to the build      | `bitbake-layers add-layer ../meta-mylayer`        |
| Remove a layer from the build | `bitbake-layers remove-layer ../meta-mylayer`     |
| List all registered layers    | `bitbake-layers show-layers`                      |
| Build a single recipe         | `bitbake <recipe-name>`                           |
| Build an image                | `bitbake <image-name>`                            |
| Clean a recipe                | `bitbake -c clean <recipe-name>`                  |
| Rebuild from scratch          | `bitbake -c cleansstate <recipe-name>`            |
| Show recipe environment       | `bitbake -e <recipe-name> \| less`                |
| Find a recipe                 | `bitbake-layers show-recipes "*<name>*"`          |

---

## 13. Next Steps

Now that you know how to create and add a meta layer, here are some areas to explore:

- 📦 **Adding more recipes** — Package existing open-source software or your own applications.
- 🔧 **Using `.bbappend` files** — Modify recipes from other layers without editing them directly.
- 🖥️ **Machine configuration** — Create custom machine definitions for your hardware.
- 📋 **Package groups** — Organize related packages into logical groups.
- 🔒 **Security hardening** — Remove `debug-tweaks` and configure proper authentication.
- 📖 **Yocto Mega Manual** — [https://docs.yoctoproject.org/](https://docs.yoctoproject.org/)

---

> **📝 Note:** This tutorial is part of the [Industrial Edge Gateway Resilient Alert System](../README.md) project documentation.
