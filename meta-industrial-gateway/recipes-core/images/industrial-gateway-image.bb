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