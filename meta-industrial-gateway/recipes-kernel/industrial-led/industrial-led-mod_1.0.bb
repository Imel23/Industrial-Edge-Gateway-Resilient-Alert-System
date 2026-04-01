SUMMARY = "Industrial LED Kernel Module"
DESCRIPTION = "Custom loadable kernel module for controlling the industrial LED via /dev/industrial_led with hrtimer-based variable-frequency blinking support"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

inherit module

# Source files
SRC_URI = " \
    file://industrial_led.c \
    file://Makefile \
    file://COPYING.MIT \
"

# Working directory
S = "${WORKDIR}"

# The inherit module class automatically handles:
# - do_compile: runs make modules
# - do_install: runs make modules_install

# Include the module in the package
FILES:${PN} += "${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/industrial_led.ko"

# Auto-load the module at boot
KERNEL_MODULE_AUTOLOAD += "industrial_led"