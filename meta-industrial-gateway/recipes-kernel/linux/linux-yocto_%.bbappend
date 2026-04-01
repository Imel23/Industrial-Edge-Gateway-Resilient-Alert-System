FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://bbb-gpio-dht11.dtsi \
            file://gpio-features.cfg"

do_configure:append() {
    # Copy our dtsi into the kernel device-tree source directory
    cp ${WORKDIR}/bbb-gpio-dht11.dtsi ${S}/arch/arm/boot/dts/ti/omap/

    # Include our dtsi directly into the BeagleBone Black base device tree
    # so our nodes are compiled into am335x-boneblack.dtb (no overlay needed)
    if ! grep -q 'bbb-gpio-dht11.dtsi' ${S}/arch/arm/boot/dts/ti/omap/am335x-boneblack.dts; then
        echo '#include "bbb-gpio-dht11.dtsi"' >> ${S}/arch/arm/boot/dts/ti/omap/am335x-boneblack.dts
    fi
}