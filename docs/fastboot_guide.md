# Flashing a Yocto Image on BeagleBone Black via Fastboot

> **Project:** Industrial Edge Gateway & Resilient Alert System  
> **Target:** BeagleBone Black (AM335x)  
> **Yocto Release:** Scarthgap 5.0 LTS  
> **Last Updated:** 2026-02-24

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Understanding the Boot Flow](#understanding-the-boot-flow)
4. [Method 1 — Flash the Full WIC Image via SD Card (Recommended)](#method-1--flash-the-full-wic-image-via-sd-card-recommended)
5. [Method 2 — Flash via U-Boot Fastboot over USB](#method-2--flash-via-u-boot-fastboot-over-usb)
   - [Step 1: Enable Fastboot in U-Boot](#step-1-enable-fastboot-in-u-boot)
   - [Step 2: Build Fastboot-Enabled U-Boot](#step-2-build-fastboot-enabled-u-boot)
   - [Step 3: Prepare the SD Card with Fastboot U-Boot](#step-3-prepare-the-sd-card-with-fastboot-u-boot)
   - [Step 4: Boot into Fastboot Mode](#step-4-boot-into-fastboot-mode)
   - [Step 5: Install Fastboot on the Host](#step-5-install-fastboot-on-the-host)
   - [Step 6: Flash the Yocto Image](#step-6-flash-the-yocto-image)
6. [Method 3 — Partition-Level Fastboot Flashing](#method-3--partition-level-fastboot-flashing)
7. [Verifying the Flash](#verifying-the-flash)
8. [Troubleshooting](#troubleshooting)
9. [Quick Reference](#quick-reference)

---

## Overview

This guide explains how to flash a custom Yocto Linux image onto the **BeagleBone Black's internal eMMC** storage using the **U-Boot fastboot** protocol over USB. Fastboot allows you to push images directly from a host PC to the target board without needing to swap SD cards, making iterative development significantly faster.

### When to Use Fastboot

| Scenario | Recommended Method |
| :-- | :-- |
| First-time setup / production flashing | **SD Card** (Method 1) |
| Iterative development & rapid re-flashing | **Fastboot** (Method 2) |
| Updating individual partitions (kernel, rootfs) | **Partition-level Fastboot** (Method 3) |

### Build Artifacts

After a successful Yocto build, the following artifacts are available in the `output/` directory:

| File | Description |
| :-- | :-- |
| `core-image-minimal-beaglebone-yocto.rootfs-*.wic` | Full disk image (bootloader + kernel + rootfs) |
| `zImage--*.bin` | Linux kernel image |
| `am335x-boneblack.dtb` | Device Tree Blob for BeagleBone Black |

---

## Prerequisites

### Hardware

- **BeagleBone Black** (Rev C recommended)
- **5V / 2A DC power supply** (USB power alone may be insufficient during eMMC writes)
- **MicroSD card** (≥ 4 GB) and SD card reader
- **Mini-USB cable** (for fastboot USB connection to the host)
- **USB-to-Serial/FTDI cable** (for accessing the U-Boot serial console — *highly recommended*)

### Software (Host Machine)

- Linux host (Ubuntu 22.04+ recommended)
- `fastboot` utility (from Android SDK Platform-Tools)
- `dd` or [balenaEtcher](https://etcher.balena.io/) for writing SD cards
- `minicom`, `screen`, or `picocom` for serial console access

### Install Host Dependencies

```bash
# Install fastboot (Ubuntu/Debian)
sudo apt update
sudo apt install -y android-sdk-platform-tools

# Verify installation
fastboot --version

# Install serial console tool
sudo apt install -y minicom
```

> **Note:** On some distributions, the package may be called `android-tools-fastboot` or `fastboot`.

---

## Understanding the Boot Flow

The AM335x boot sequence on the BeagleBone Black is:

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  AM335x ROM  │───▶│  MLO (SPL)   │───▶│   U-Boot     │───▶│ Linux Kernel │
│  Boot Code   │    │  (1st stage) │    │  (2nd stage) │    │   + rootfs   │
└──────────────┘    └──────────────┘    └──────────────┘    └──────────────┘
     eMMC/SD             FAT32              FAT32              ext4
```

- **MLO (SPL):** Minimal loader that initializes DRAM and loads U-Boot.
- **U-Boot:** Full bootloader that loads the kernel, DTB, and passes control to Linux.
- **Fastboot mode** is entered from U-Boot, transforming the board into a USB device that accepts images from the host.

---

## Method 1 — Flash the Full WIC Image via SD Card (Recommended)

This is the simplest and most reliable method. Use this for initial setup.

### 1.1 Write the WIC Image to an SD Card

```bash
# Identify your SD card device (e.g., /dev/sdX or /dev/mmcblkX)
lsblk

# ⚠️  Double-check the device — this will ERASE the target!
sudo dd if=output/core-image-minimal-beaglebone-yocto.rootfs-*.wic \
       of=/dev/sdX \
       bs=4M \
       conv=fsync \
       status=progress
```

### 1.2 Boot from the SD Card

1. **Power off** the BeagleBone Black.
2. Insert the prepared microSD card.
3. **Hold the USER/BOOT button** (S2, near the SD card slot).
4. While holding S2, apply power (connect the 5V supply).
5. Release S2 after the USR LEDs begin to blink (~2 seconds).

### 1.3 Flash to eMMC from the Running System

Once booted from SD, you can write the image to the internal eMMC:

```bash
# SSH into the BeagleBone Black or use serial console
# The eMMC is typically /dev/mmcblk1 when booted from SD

sudo dd if=/dev/mmcblk0 of=/dev/mmcblk1 bs=4M conv=fsync status=progress
```

> **Alternative:** Use the WIC image file directly if it's accessible on the SD card filesystem.

### 1.4 Boot from eMMC

1. Power off the board.
2. Remove the SD card.
3. Power on — the board will now boot from eMMC.

---

## Method 2 — Flash via U-Boot Fastboot over USB

This method uses the **USB fastboot protocol** built into U-Boot, allowing you to push images directly from your host PC over the mini-USB cable. This is ideal for rapid iteration during development.

### Step 1: Enable Fastboot in U-Boot

Your U-Boot build must include fastboot support. Add or verify the following configuration options in your U-Boot defconfig (e.g., `am335x_boneblack_defconfig`):

```kconfig
# Core fastboot support
CONFIG_USB_FUNCTION_FASTBOOT=y
CONFIG_CMD_FASTBOOT=y
CONFIG_FASTBOOT=y
CONFIG_FASTBOOT_FLASH=y

# Target the eMMC (MMC device 1 on BeagleBone Black)
CONFIG_FASTBOOT_FLASH_MMC_DEV=1

# USB gadget support
CONFIG_USB_GADGET=y
CONFIG_USB_GADGET_DOWNLOAD=y
CONFIG_USB_GADGET_VENDOR_NUM=0x0451
CONFIG_USB_GADGET_PRODUCT_NUM=0xd022

# Fastboot buffer configuration
# Buffer in DRAM where images are staged before writing
CONFIG_FASTBOOT_BUF_ADDR=0x82000000
CONFIG_FASTBOOT_BUF_SIZE=0x06000000

# GPT partition support (for partition-level flashing)
CONFIG_FASTBOOT_GPT_NAME="gpt"
CONFIG_EFI_PARTITION=y
```

> **Key Setting:** `CONFIG_FASTBOOT_FLASH_MMC_DEV=1` ensures fastboot writes to the **eMMC** (device 1) rather than the SD card (device 0).

### Step 2: Build Fastboot-Enabled U-Boot

If you're building U-Boot within Yocto, you can add a `.bbappend` for U-Boot in your custom layer. Otherwise, build U-Boot standalone:

#### Option A: Yocto `.bbappend` (Recommended)

Create or edit: `meta-industrial-gateway/recipes-bsp/u-boot/u-boot_%.bbappend`

```bitbake
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://fastboot.cfg"
```

Then create `meta-industrial-gateway/recipes-bsp/u-boot/files/fastboot.cfg`:

```kconfig
CONFIG_USB_FUNCTION_FASTBOOT=y
CONFIG_CMD_FASTBOOT=y
CONFIG_FASTBOOT=y
CONFIG_FASTBOOT_FLASH=y
CONFIG_FASTBOOT_FLASH_MMC_DEV=1
CONFIG_USB_GADGET=y
CONFIG_USB_GADGET_DOWNLOAD=y
CONFIG_FASTBOOT_BUF_ADDR=0x82000000
CONFIG_FASTBOOT_BUF_SIZE=0x06000000
CONFIG_EFI_PARTITION=y
```

Rebuild:

```bash
bitbake u-boot -c cleansstate
bitbake u-boot
```

#### Option B: Standalone U-Boot Build

```bash
# Clone U-Boot
git clone https://source.denx.de/u-boot/u-boot.git
cd u-boot
git checkout v2024.04  # or your preferred version

# Configure for BeagleBone Black
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- am335x_boneblack_defconfig

# Apply fastboot configs (add them to .config or create a fragment)
# Then build
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc)

# Output files:
# - MLO          (SPL first-stage bootloader)
# - u-boot.img   (second-stage bootloader)
```

### Step 3: Prepare the SD Card with Fastboot U-Boot

Create a bootable SD card with the fastboot-enabled U-Boot:

```bash
# Partition the SD card
# Partition 1: FAT32, 64MB, boot flag
# Partition 2: ext4, remaining space (optional)

sudo fdisk /dev/sdX << EOF
o
n
p
1

+64M
t
c
a
w
EOF

# Format the boot partition
sudo mkfs.vfat -F 32 -n BOOT /dev/sdX1

# Mount and copy bootloader files
sudo mount /dev/sdX1 /mnt
sudo cp MLO /mnt/
sudo cp u-boot.img /mnt/
sudo umount /mnt
```

### Step 4: Boot into Fastboot Mode

1. Insert the SD card with the fastboot-enabled U-Boot into the BBB.
2. Connect the **USB-to-Serial cable** to the BBB's J1 header (pins: GND, RX, TX).
3. Open a serial terminal on your host:

   ```bash
   sudo minicom -D /dev/ttyUSB0 -b 115200
   ```

4. Connect the **Mini-USB cable** to the BBB's USB client port (near the barrel jack).
5. **Hold S2** (USER button) and apply power to boot from SD.
6. In the serial console, **interrupt autoboot** by pressing any key when you see:

   ```
   Hit any key to stop autoboot:  3
   ```

7. At the U-Boot prompt, enter fastboot mode:

   ```
   U-Boot# fastboot usb 0
   ```

   You should see:

   ```
   musb-hdrc: peripheral reset irq
   ```

   The board is now waiting for fastboot commands from the host.

### Step 5: Install Fastboot on the Host

If not already installed:

```bash
# Ubuntu/Debian
sudo apt install -y android-sdk-platform-tools

# Fedora
sudo dnf install -y android-tools

# Arch Linux
sudo pacman -S android-tools
```

Verify the board is detected:

```bash
sudo fastboot devices
```

Expected output:

```
<serial-number>    fastboot
```

> **Tip:** If no device appears, check USB connections and try adding a udev rule:
>
> ```bash
> echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="0451", MODE="0666", GROUP="plugdev"' | \
>   sudo tee /etc/udev/rules.d/51-beaglebone-fastboot.rules
> sudo udevadm control --reload-rules
> sudo udevadm trigger
> ```

### Step 6: Flash the Yocto Image

#### 6a. Flash the Full WIC Image (Raw Disk Image)

The WIC image contains the complete disk layout (partition table + boot partition + rootfs). Flash it as a raw image to the entire eMMC:

**For U-Boot v2021.04 and later** (with `flash` to raw device support):

```bash
# Flash the entire WIC image to eMMC device 1, starting at sector 0
sudo fastboot flash 1:0 output/core-image-minimal-beaglebone-yocto.rootfs-*.wic
```

**For older U-Boot versions** (use the `stage` + `mmc write` approach):

On the host:

```bash
# Stage the image into U-Boot's RAM buffer
sudo fastboot stage output/core-image-minimal-beaglebone-yocto.rootfs-*.wic
```

Then, in the U-Boot serial console (press Ctrl+C to exit fastboot mode first):

```
# Calculate the number of 512-byte sectors
# Image size: ~72 MB → 72674304 / 512 = 141942 sectors = 0x22A86

U-Boot# mmc dev 1
U-Boot# mmc write 0x82000000 0x0 0x22A86
```

> ⚠️ **Adjust the sector count** based on your actual image size. Use:  
> `stat --format="%s" <image_file>` to get the byte size, then divide by 512.

#### 6b. Flash Individual Partitions

If your U-Boot has GPT-aware fastboot support:

```bash
# Flash the boot partition (contains MLO, u-boot.img, zImage, DTB)
sudo fastboot flash boot boot_partition.img

# Flash the root filesystem
sudo fastboot flash rootfs rootfs.ext4
```

#### 6c. Flash the Bootloader Only

```bash
# Flash SPL/MLO
sudo fastboot flash spl MLO

# Flash U-Boot
sudo fastboot flash bootloader u-boot.img
```

After flashing, reboot from the U-Boot console:

```
U-Boot# reset
```

Or from the host:

```bash
sudo fastboot reboot
```

---

## Method 3 — Partition-Level Fastboot Flashing

For iterative development, you often only need to update the **kernel** or **rootfs** without re-flashing the entire image. This requires a GPT partition table on the eMMC.

### 3.1 Set Up GPT Partitions

First, define the partition layout in U-Boot. At the U-Boot console:

```
U-Boot# setenv partitions "uuid_disk=${uuid_gpt_disk}; \
  name=boot,start=1MiB,size=64MiB,type=0FC63DAF-8483-4772-8E79-3D69D8477DE4; \
  name=rootfs,start=65MiB,size=-,type=0FC63DAF-8483-4772-8E79-3D69D8477DE4"
U-Boot# gpt write mmc 1 $partitions
```

### 3.2 Flash Individual Components

```bash
# Update the kernel only
sudo fastboot flash boot:zImage output/zImage--*.bin

# Update the device tree only
sudo fastboot flash boot:am335x-boneblack.dtb output/am335x-boneblack.dtb

# Update the root filesystem
sudo fastboot flash rootfs output/core-image-minimal-beaglebone-yocto.rootfs.ext4
```

### 3.3 Using Raw Partition Descriptors

If you don't use GPT, you can define raw partition mappings in U-Boot:

```
U-Boot# setenv fastboot_raw_partition_boot 0x800 0x20000
U-Boot# setenv fastboot_raw_partition_rootfs 0x20800 0x100000
U-Boot# saveenv
```

The format is: `<start_sector> <size_in_sectors>`.

---

## Verifying the Flash

After flashing, verify the image was written correctly:

### From U-Boot

```
U-Boot# mmc dev 1
U-Boot# mmc info
U-Boot# mmc part
```

### After Booting Linux

```bash
# Check eMMC partitions
lsblk

# Verify the root filesystem
df -h

# Check kernel version
uname -a

# Verify device tree
cat /proc/device-tree/model
# Expected: "TI AM335x BeagleBone Black"
```

---

## Troubleshooting

### `fastboot devices` shows no output

| Possible Cause | Solution |
| :-- | :-- |
| USB cable issue | Use a **data-capable** mini-USB cable (not charge-only) |
| Board not in fastboot mode | Verify `fastboot usb 0` was entered in U-Boot |
| Permission issue | Run with `sudo` or add udev rules (see [Step 5](#step-5-install-fastboot-on-the-host)) |
| Wrong USB port | Use the **mini-USB client port** (P4, near the barrel jack), not the USB-A host port |

### `FAILED (remote: unknown command)` during flash

U-Boot's fastboot implementation may not support all standard fastboot commands. Solutions:

- Use the **`stage` + `mmc write`** workaround (see [Step 6a](#6a-flash-the-full-wic-image-raw-disk-image))
- Upgrade to U-Boot **v2021.04 or later** for improved fastboot support

### Transfer stalls or fails midway

- Use a **5V DC power supply** (not just USB power)
- Reduce `CONFIG_FASTBOOT_BUF_SIZE` if your board has limited DRAM
- Try a shorter USB cable for reliability

### Board doesn't boot after flashing

1. **Re-flash via SD card** (Method 1) as recovery
2. Verify the WIC image integrity:
   ```bash
   md5sum output/core-image-minimal-beaglebone-yocto.rootfs-*.wic
   ```
3. Ensure the eMMC is being targeted (device 1, not device 0)
4. Check U-Boot environment:
   ```
   U-Boot# printenv bootcmd
   U-Boot# printenv bootargs
   ```

### Serial console shows garbled text

- Ensure baud rate is set to **115200**
- Connection: **GND → Pin 1**, **RX → Pin 4**, **TX → Pin 5** on J1 header
- Do **not** connect the VCC/3.3V pin from the FTDI cable

---

## Quick Reference

### Essential Commands Cheat Sheet

```bash
# ─── Host Side ───────────────────────────────────────────

# Check if board is detected
sudo fastboot devices

# Flash full WIC image (modern U-Boot)
sudo fastboot flash 1:0 <image>.wic

# Flash full WIC image (legacy U-Boot)
sudo fastboot stage <image>.wic
# → then in U-Boot: mmc dev 1; mmc write 0x82000000 0x0 <sectors>

# Flash bootloader components
sudo fastboot flash spl MLO
sudo fastboot flash bootloader u-boot.img

# Reboot the board
sudo fastboot reboot

# ─── U-Boot Side (Serial Console) ───────────────────────

# Enter fastboot mode
fastboot usb 0

# Check eMMC
mmc dev 1
mmc info
mmc part

# Write staged image to eMMC
mmc write <buf_addr> <start_sector> <sector_count>

# Reset the board
reset
```

### Connection Diagram

```
 Host PC                          BeagleBone Black
┌──────────┐                     ┌────────────────────┐
│          │  Mini-USB Cable     │ P4 (USB Client)    │
│  USB  ◄──┼─────────────────────┼──►  AM335x         │
│          │                     │                    │
│          │  FTDI/Serial Cable  │ J1 (Serial Header) │
│  USB  ◄──┼─────────────────────┼──►  UART0          │
│          │                     │                    │
│ fastboot │                     │ U-Boot (fastboot)  │
│ minicom  │                     │                    │
└──────────┘                     └────────────────────┘
                                       │
                                   5V DC Power
                                   (P1 Barrel Jack)
```

---

## References

- [U-Boot Fastboot Documentation](https://docs.u-boot.org/en/latest/android/fastboot.html)
- [BeagleBone Black System Reference Manual](https://docs.beagleboard.org/latest/boards/beaglebone/black/)
- [Yocto Project Documentation — WIC Image Creator](https://docs.yoctoproject.org/ref-manual/kickstart.html)
- [AM335x Technical Reference Manual (TI)](https://www.ti.com/lit/ug/spruh73q/spruh73q.pdf)

---

*This guide is part of the [Industrial Edge Gateway & Resilient Alert System](https://github.com/Imel23/Industrial-Edge-Gateway-Resilient-Alert-System) project documentation.*
