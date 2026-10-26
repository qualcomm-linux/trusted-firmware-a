Qualcomm Hermosa (IPQ52xx) platform
===================================

Trusted Firmware-A (TF-A) platform port for the Qualcomm IPQ52xx SoC, based on
the Bobcat architecture, also known as Hermosa as the marketing name.

The SoC has a single cluster of four Cortex-A53 cores and a GICv2 (QGIC)
interrupt controller.

Boot flow
---------

U-Boot SPL loads BL31 and jumps to it at EL3. BL31 receives the image entry
point information from U-Boot SPL through the ``bl_params`` structure,
initializes the platform and OP-TEE OS (BL32) through the OP-TEE dispatcher,
and then exits to BL33 in the normal world.

::

	U-Boot SPL --> BL31 --> BL32 (OP-TEE OS) init --> BL31 --> U-Boot Proper

Current limitations
-------------------

- Only the primary CPU is brought up. ``PSCI CPU_ON`` for secondary cores is
  not supported yet and returns ``PSCI_E_INTERN_FAIL``.
- The only idle state advertised is CPU standby (WFI retention). CPU and
  cluster power collapse are not supported.
- System off and system reset are done by de-asserting PS_HOLD.
- The stack protector is disabled until an RNG source is ported.

The BL31 log is written both to the UART console and to a ring buffer in
shared IMEM, whose address is published at ``TFA_BL31_RING_LOG_BASE`` so that
dump tools can find it.

How to build
------------

Steps to build TF-A BL31 with the OP-TEE dispatcher::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=ipq52xx SPD=opteed bl31

Boot Trace
--------------

Boot logs on ipq52xx::

        U-Boot SPL, End
        NOTICE:  BL31: v2.15.0(debug):qcom-next-2.15-20260727-852-gce0177e46e1c
        NOTICE:  BL31: Built : 11:53:07, Oct 26 2026
        INFO:    ARM GICv2 driver initialized
        INFO:    BL31: Initializing runtime services
        INFO:    BL31: Initializing BL32
        I/TC:
        I/TC: OP-TEE version: 4.10.0-438-g9b756a1a1-dev (gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04.1)) #1 Wed Oct 14 07:11:30 UTC 2026 aarch64
        I/TC: WARNING: This OP-TEE configuration might be insecure!
        I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
        I/TC: Primary CPU initializing
        I/TC: [WDT] bark handler registered on IRQ 35, base=0x83b17000
        I/TC: Platform Qualcomm: Flavor ipq52xx
        I/TC: Primary CPU switching to normal world boot
        INFO:    BL31: Preparing for EL3 exit to normal world
        INFO:    Entry point address = 0x80280000
        INFO:    SPSR = 0x3c9
