SUMMARY = "Industrial DHT11 IIO Kernel Module"
DESCRIPTION = "Custom IIO (Industrial I/O) kernel module for the DHT11 \
temperature and humidity sensor. Implements the 1-wire protocol via GPIO \
bit-banging, checksum validation, and exposes data through standard IIO \
channels (in_temp_raw, in_humidityrelative_raw)."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

inherit module

# Source files
SRC_URI = " \
    file://industrial_dht11.c \
    file://Makefile \
    file://COPYING.MIT \
"

# Working directory
S = "${WORKDIR}"

# The inherit module class automatically handles:
# - do_compile: runs make modules
# - do_install: runs make modules_install

# Include the module in the package
FILES:${PN} += "${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/industrial_dht11.ko"

# Auto-load the module at boot
KERNEL_MODULE_AUTOLOAD += "industrial_dht11"
