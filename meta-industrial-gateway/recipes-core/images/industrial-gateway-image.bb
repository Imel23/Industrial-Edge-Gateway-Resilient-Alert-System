SUMMARY = "Industrial Edge Gateway Image"
DESCRIPTION = "A minimal image for the Industrial Edge Gateway with resilient alert system support."
LICENSE = "MIT"

inherit core-image

IMAGE_INSTALL:append = " \
    kernel-modules \
    industrial-led-mod \
"

# Set a root password (change for production!)
EXTRA_IMAGE_FEATURES += "debug-tweaks"
