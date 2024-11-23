SUBDIRS = lib
bin_PROGRAMS = atafuzzer
atafuzzer_SOURCES = main.c
atafuzzer_LDADD = lib/libata_controller.a lib/libata_device.a lib/libata_fuzzer.a lib/libdma_buffer.a lib/libinput.a lib/libpci_device.a ../lib/liberror.a -lm
