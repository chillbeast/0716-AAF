# IGNITE Linux (Buildroot)

Buildroot tree for the BE 0716-AAF Linux image.

For a source-based overview of the processor/OS configuration, boot services,
partitions, hardware interfaces, build gaps and device inspection commands, see
the community [Linux OS guide and source audit](../docs/LINUX_OS_GUIDE.md).
It distinguishes the supplied recipes from a verified, installable device image.

## Building

On a Linux host:

```sh
./builder.sh Ignite      # installs host packages, then: make Ignite_defconfig && make
```

This is the original build entry point. Review the [known build and installation
gaps](../docs/LINUX_OS_GUIDE.md#known-build-and-installation-gaps) first: the released
pipeline still references omitted payloads and an older application repository
layout, so it is not a verified one-command device installer.

Board files are in `buildroot/board/freescale/imx8mm_var_som/ignite/`. The configurations are
`buildroot/configs/Ignite_defconfig`, `Ignite-rel_defconfig` and `Ignite_uuu_defconfig`.

The post-image scripts clone and build the application. Set `IGNITE_SOURCE_GIT_URL` to the repository
that holds `Source/Application` before building. Release images also need the MCU firmware in
`board/freescale/imx8mm_var_som/ignite/mcu-image/Ignite_MCU.hex`, which is not part of this release.

The downloaded source tarballs (`buildroot/dl/`) are not included. Buildroot downloads them on the
first build. Some NXP packages (GPU, multimedia codecs, firmware) are downloaded under NXP's own
license terms, which Buildroot will ask you to accept. The NXP/Marvell SD8987 Wi-Fi firmware
(`sd8987_uapsta.bin`) is not included. Get it from
[linux-firmware](https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git)
(`nxp/sd8987_uapsta.bin`) under NXP's license and add it to `rootfs_overlay/usr/lib/firmware/mrvl/`.

The prebuilt bootloader and the `pcmode` USB transfer service (started by
`create-usb-gadgets.sh`) were removed because they cannot be redistributed. Buildroot builds U-Boot
from source.

## Accounts

The `root` and `ignite` accounts are locked in `rootfs_overlay/etc/shadow`, and no Wi-Fi network is
preconfigured. To log in on your own build, set a password hash (for example from
`mkpasswd -m sha-512`) in that file. Dropbear SSH is enabled, so do not use an empty password on a
networked device.

## Updating a device

`scripts/swUpdateImage.sh` produces an SWUpdate package (`.swu`) described by `fullimage/sw-description`.
The package is not signed.

For the device's USB firmware update, `scripts/createEncryptedImage.sh` packs the `.swu`, the MCU
firmware and `version.txt` into an ext4 image and encrypts it with
`openssl enc -aes-256-cbc -pbkdf2 -salt` using the key in that script. The application decrypts the
update with the same key (`decrptKey` in
`Source/Application/modules/Firmware/Application/Model/FrontPannelManager.h`). The encryption only
packages the update. It is not a security boundary, and the key is published so that you can install
modified software on your own device.

## License

Buildroot is licensed under GPL-2.0, see `buildroot/COPYING`. Packages built by Buildroot keep their
own licenses.
