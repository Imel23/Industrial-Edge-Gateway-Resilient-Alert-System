SUMMARY = "Industrial Button IRQ Kernel Module"
DESCRIPTION = "Custom loadable kernel module for handling the industrial button via hardware interrupts (IRQ) with software debouncing"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

inherit module

# Définition des sources (fichiers locaux dans le dossier files/)
SRC_URI = " \
    file://industrial_button.c \
    file://Makefile \
    file://COPYING.MIT \
"

# Le répertoire de travail de la recette
S = "${WORKDIR}"

# The inherit module class automatically handles:
# - do_compile: runs make modules
# - do_install: runs make modules_install

# Ajouter le module au package principal pour qu'il soit installé
FILES:${PN} += "${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/industrial_button.ko"

# Optionnel : charger le module automatiquement au démarrage
KERNEL_MODULE_AUTOLOAD += "industrial_button"
