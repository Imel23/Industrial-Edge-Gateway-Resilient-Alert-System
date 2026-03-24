SUMMARY = "Industrial LED Kernel Module"
DESCRIPTION = "Custom loadable kernel module for controlling the industrial LED via /dev/industrial_led"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

inherit module

# Définition des sources (fichiers locaux dans le dossier files/)
SRC_URI = " \
    file://industrial_led.c \
    file://Makefile \
    file://COPYING.MIT \
"

# Le répertoire de travail de la recette
S = "${WORKDIR}"

# The inherit module class automatically handles:
# - do_compile: runs make modules
# - do_install: runs make modules_install

# Ajouter le module au package principal pour qu'il soit installé
FILES:${PN} += "${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/industrial_led.ko"

# Optionnel : charger le module automatiquement au démarrage
KERNEL_MODULE_AUTOLOAD += "industrial_led"