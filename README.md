atafuzzer
=========

![Build Status](https://github.com/rcvalle/atafuzzer/workflows/build/badge.svg)

An AT Attachment (ATA) storage interface fuzzer for emulated ATA devices.

It currently supports:

* the general feature set for devices not implementing the PACKET command
  feature set.
* the 48-bit address feature set.


Installation
------------

To build and install the fuzzer:

1. Install Autotools (i.e., Autoconf, Automake, and Libtool):

       sudo apt update
       sudo apt install autoconf automake libtool

2. Instantiate the build system:

       autoreconf -fi

3. Build the fuzzer:

       mkdir build/
       cd build/
       ../configure
       make

4. Install the fuzzer:

       sudo make install


Usage
-----

To use the fuzzer:

1. Create a new virtual machine for the hypervisor/VMM to be tested.

2. Add the device to be tested to the virtual machine.

3. Identify the device to be tested:

       lspci
       00:00.0 Host bridge: Intel Corporation 440FX - 82441FX PMC [Natoma] (rev 02)
       00:01.0 ISA bridge: Intel Corporation 82371SB PIIX3 ISA [Natoma/Triton II]
       00:01.1 IDE interface: Intel Corporation 82371SB PIIX3 IDE [Natoma/Triton II]
       00:01.3 Bridge: Intel Corporation 82371AB/EB/MB PIIX4 ACPI (rev 03)
       00:02.0 VGA compatible controller: Red Hat, Inc. QXL paravirtual graphic card (rev 05)
       00:03.0 Ethernet controller: Red Hat, Inc. Virtio network device
       00:04.0 Audio device: Intel Corporation 82801I (ICH9 Family) HD Audio Controller (rev 03)
       00:05.0 USB controller: Intel Corporation 82801I (ICH9 Family) USB UHCI Controller #1 (rev 03)
       00:05.1 USB controller: Intel Corporation 82801I (ICH9 Family) USB UHCI Controller #2 (rev 03)
       00:05.2 USB controller: Intel Corporation 82801I (ICH9 Family) USB UHCI Controller #3 (rev 03)
       00:05.7 USB controller: Intel Corporation 82801I (ICH9 Family) USB2 EHCI Controller #1 (rev 03)
       00:06.0 SCSI storage controller: Red Hat, Inc. Virtio SCSI
       00:07.0 SATA controller: Intel Corporation 82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller [AHCI mode] (rev 02)
       00:08.0 Communication controller: Red Hat, Inc. Virtio console
       00:09.0 SCSI storage controller: Red Hat, Inc. Virtio block device
       00:0a.0 Unclassified device [00ff]: Red Hat, Inc. Virtio memory balloon
       00:0b.0 Unclassified device [00ff]: Red Hat, Inc. Virtio RNG

   The first column is the PCI logical address of the device as
   [domain:]bus:device.function.

4. Copy to, build, and install the fuzzer in the virtual machine.

5. Run the fuzzer:

       sudo atafuzzer -g -B 0 -D 1 -F 1


The command-line options for the fuzzer are:

**-B**
**--bus=**_num_
  Specify the PCI bus number of the ATA/IDE controller. (The default is 0.)

**-D**
**--device=**_num_
  Specify the PCI device number of the ATA/IDE controller. (The default is 0.)

**-F**
**--function=**_num_
  Specify the PCI function number of the ATA/IDE controller. (The default is 0.)

**--bus-num=**_num_
  Specify the ATA bus number. Use 0 for primary, or 1 for secondary. (The
  default is 0.)

**--device-num=**_num_
  Specify the ATA device number. Use 0 for Device 0, or 1 for Device 1. (The
  default is 0.)

**-d**
**--debug**
  Enable debug mode.

**-g**
**--generate**
  Use the pseudorandom number generator (i.e., random()) for input generation.

**-h**
**--help**
  Display help information and exit.

**-o** _file_
**--output=**_file_
  Specify the output file name.

**-q**
**--quiet**
  Enable quiet mode.

**-s** _num_
**--seed=**_num_
  Specify the seed for the pseudorandom number generator. (The default is 1.)

**-t** _num_
**--timeout=**_num_
  Specify the timeout, in seconds, for each iteration. (The default is 5.)

**-v**
**--verbose**
  Enable verbose mode.

**--version**
  Display version information and exit.


Contributing
------------

See [CONTRIBUTING.md](CONTRIBUTING.md).


License
-------

Licensed under the MIT License. See [LICENSE](LICENSE) for license text and
copyright information.
