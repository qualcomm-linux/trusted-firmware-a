# NORD SysINI Library

`libsysini.a` is the prebuilt NORD/Wildcat SysINI library bundled for the
NORD IQ10 FBC workaround branch.

- Source repository: `cpussfirmware/sysini`
- Source branch: `wildcat_llvm`
- Base revision: `b456a0dbc227cae2da7265b01decee9f7048586a`
- SysINI release version: `0x00071800`
- Toolchain: Qualcomm LLVM 20.0.0, target `aarch64-none-elf`
- SHA-256: `e66ce79fde79861e128a78ba1d27cd8e37364b36028c266ae4fd3d2874c0559b`

The source checkout used to supply this archive contained local modifications
to `Makefile`, `src/armlib_bti.inc`, and `src/sysini_stubs.S`; therefore the
base revision alone does not fully reproduce the archive.
