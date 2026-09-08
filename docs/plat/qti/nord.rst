Qualcomm Nord platform
=======================

Trusted Firmware-A (TF-A) platform port for the Qualcomm Nord SoC (SA8797P),
part of the Wildcat platform family (``plat/qti/wildcat/nord``). The platform
name is ``nord_generic``.

Nord has three CPU clusters of six Qualcomm Oryon-1 cores each (18 cores
total), with a dedicated in-tree Oryon-1 CPU library
(``lib/cpus/aarch64/oryon.S``).

Unlike the other Qualcomm ports, the Nord port does not link against the
closed ``QTISECLIB`` blob: BL31 is built entirely from in-tree sources. The
PSCI CPU bring-up, GIC, PDC/RSC, cmd_db and other pieces that ``QTISECLIB``
would normally provide are implemented natively in-tree.

Platform features
-----------------

- **Reset entry.** BL2 is the reset entry image (``RESET_TO_BL2=1``) and runs
  at EL3. XBL loads and authenticates BL2 (signed as the TZ image) and the
  FIP, which must be placed where XBL expects it (``PLAT_QTI_FIP_IOBASE``,
  ``0xd8d00000``).

- **CPU library.** ``oryon.S`` handles the Oryon-1 reset path (exclusive
  Non-Cacheable load/store enable and AMU counter setup) and no-op power-down
  hooks (the platform builds ``HW_ASSISTED_COHERENCY=1`` and XBL releases the
  cores already coherent).

- **PSCI.** Native CPU bring-up: BL31 powers the secondary Oryon cores via
  PSCI ``CPU_ON`` by driving the per-cluster NCC_ARCH registers directly, and
  notifies CPUCP of per-core power-state changes. Nord advertises CPU standby
  (WFI) only.

- **GIC.** GIC-700 (GICv3). The distributor resets with ``IGRPMODR`` set, so
  the port explicitly regroups non-secure interrupts to Group-1NS. The APSS
  INTU is brought up to condition peripheral SPIs (edge/level type) before
  they reach the GIC.

- **Drivers.** Secure watchdog, architected timer (qtimer), SMMU
  configuration, PDC (branch-mask driven, no TCS path) and CPUCP host driver
  are all provided in-tree.

Boot flow
---------

XBL loads and authenticates BL2 (signed as the TZ image) and the FIP, then
enters BL2 at EL3. BL2 loads BL31, the BL32 payload (OP-TEE, under
``SPD=opteed``) and BL33 from the FIP, and hands over to BL31. BL31 brings up
the secondary cores via PSCI ``CPU_ON`` and starts the non-secure OS at EL2.

How to build
------------

Build TF-A BL31 and BL2::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=nord_generic bl31 bl2

Build with an OP-TEE BL32 payload and a non-secure BL33 packed into a FIP::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=nord_generic SPD=opteed \
	    BL32=<path-to-optee-bin> BL33=<path-to-os-bootloader-bin> \
	    bl31 bl2 fip all

The ``bl2.elf`` generated here must be signed as the TZ image, and the FIP must
be placed where XBL expects it (``PLAT_QTI_FIP_IOBASE``).

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
