# Angstrom pda image 
#
# This image provides the bare minimum to run Pd. The aim is to make it boot
# straight into Pd, with nothing else running.

# this image is aimed at running in 16MB of RAM

XSERVER ?= "xserver-kdrive-fbdev"

export IMAGE_BASENAME = "pda-16mb-image"

PR = "r2"

DEPENDS = "task-boot"
IMAGE_INSTALL = "\
    ${XSERVER} \
    task-boot \
    task-base-kernel26 \
    ${@base_contains("MACHINE_FEATURES", "acpi", "task-base-acpi", "",d)} \
    ${@base_contains("MACHINE_FEATURES", "alsa", "task-base-alsa", "", d)} \
    ${@base_contains("MACHINE_FEATURES", "apm", "task-base-apm", "", d)} \
    ${@base_contains("MACHINE_FEATURES", "ext2", "task-base-ext2", "", d)} \
    ${@base_contains("MACHINE_FEATURES", "vfat", "task-base-vfat", "", d)} \
    ${@base_contains("MACHINE_FEATURES", "irda", "task-base-irda", "",d)} \
    ${@base_contains("MACHINE_FEATURES", "keyboard", "task-base-keyboard", "", d)} \
    ${@base_contains("MACHINE_FEATURES", "pci", "task-base-pci", "",d)} \
    ${@base_contains("MACHINE_FEATURES", "pcmcia", "task-base-pcmcia", "", d)} \
    ${@base_contains("MACHINE_FEATURES", "screen", "task-base-screen", "", d)} \
    ${@base_contains("MACHINE_FEATURES", "serial", "task-base-serial", "", d)} \
    ${@base_contains("MACHINE_FEATURES", "touchscreen", "task-base-touchscreen", "", d)} \
    gpe-dm gpe-session-scripts \
    matchbox-wm \
    esound gpe-soundserver gpe-soundbite \
    gpe-terminal gpe-mixer \
    angstrom-feed-configs \
    tcl tk pda \
    opkg-nogpg opkg-collateral \
    "


#    kernel-module-snd kernel-module-soundcore \
#    kernel-module-snd-mixer kernel-module-snd-pcm kernel-module-snd-timer \
#    kernel-module-snd-mixer-oss kernel-module-snd-pcm-oss \
#    kernel-module-snd-ac97 kernel-module-snd-pxa2xx-ac97 \
#    kernel-module-snd-pxa2xx-i2sound kernel-module-snd-pxa2xx-pcm \


# set up default settings for Pd
pda_make_pdrc() {
    curdir=$PWD
    cd ${IMAGE_ROOTFS}
	touch  ./home/root/.pdrc
	echo "-nomidi" >> ./home/root/.pdrc
	echo "-open /home/root/palmte2.pd" >> ./home/root/.pdrc
    cd $curdir
}
ROOTFS_POSTPROCESS_COMMAND += "pda_make_pdrc"

# this should hopefully mount the VFAT partition once things are booted
pda_fstab_postprocess() {
    curdir=$PWD
    cd ${IMAGE_ROOTFS}
    echo >>./etc/fstab
    echo "# SD card FAT" >>./etc/fstab
    echo "/dev/mmcblk0p1 /media/card vfat defaults,ro,sync 0 0" >>./etc/fstab
    cd $curdir
}
#ROOTFS_POSTPROCESS_COMMAND += "pda_fstab_postprocess"

#zap root password for release images
ROOTFS_POSTPROCESS_COMMAND += "zap_root_password; "

# I think this should add a user and skip the login screen
ROOTFS_POSTPROCESS_COMMAND += "set_image_autologin"

# set the time using the build time to start with a reasonable guess
ROOTFS_POSTPROCESS_COMMAND += "create_etc_timestamp"


inherit image
