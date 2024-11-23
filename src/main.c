/** @file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../lib/error.h"
#include "lib/ata_controller.h"
#include "lib/ata_fuzzer.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/io.h>
#include <unistd.h>

#define usage() \
    fprintf(stderr, \
            "Usage: %s [OPTION]... [INPUT]\n" \
            "Options:\n" \
            "  -B, --bus=NUM         Specify the PCI bus number of the ATA/IDE controller.\n" \
            "                        (The default is 0.)\n" \
            "  -D, --device=NUM      Specify the PCI device number of the ATA/IDE controller.\n" \
            "                        (The default is 0.)\n" \
            "  -F, --function=NUM    Specify the PCI function number of the ATA/IDE\n" \
            "                        controller. (The default is 0.)\n" \
            "      --bus-num=NUM     Specify the ATA bus number. Use 0 for primary, or 1 for\n" \
            "                        secondary. (The default is 0.)\n" \
            "      --device-num=NUM  Specify the ATA device number. Use 0 for Device 0, or 1\n" \
            "                        for Device 1. (The default is 0.)\n" \
            "  -d, --debug           Enable debug mode.\n" \
            "  -g, --generate        Use the pseudorandom number generator (i.e., random())\n" \
            "                        for input generation.\n" \
            "  -h, --help            Display help information and exit.\n" \
            "  -o, --output=FILE     Specify the output file name.\n" \
            "  -q, --quiet           Enable quiet mode.\n" \
            "  -s, --seed=NUM        Specify the seed for the pseudorandom number generator.\n" \
            "                        (The default is 1.)\n" \
            "  -t, --timeout=NUM     Specify the timeout, in seconds, for each iteration.\n" \
            "                        (The default is 5.)\n" \
            "  -v, --verbose         Enable verbose mode.\n" \
            "      --version         Display version information and exit.\n", \
            PACKAGE_NAME)

#define version() fprintf(stderr, "%s\n", PACKAGE_STRING)

void
default_error_handler(int status, int error, const char *restrict format, va_list ap)
{
    fflush(stdout);
    vfprintf(stderr, format, ap);
    if (error != 0) {
        fprintf(stderr, ": %s\n", strerror(error));
    }

    fflush(stderr);
    abort();
}

void
default_log_handler(FILE *restrict stream, const char *restrict format, va_list ap)
{
    flockfile(stream);
    fprintf(stream, "{ ");
    fprintf(stream, "\"time\": %d,", (unsigned int)time(NULL));
    for (size_t i = 0; format[i] != '\0'; ++i) {
        if (i > 0) {
            fprintf(stream, ", ");
        }

        fprintf(stream, "\"%s\": ", va_arg(ap, char *));
        switch (format[i]) {
        case 'c':
            fprintf(stream, "\"%c\"", va_arg(ap, int));
            break;

        case 'd':
            fprintf(stream, "%d", va_arg(ap, int));
            break;

        case 'f':
            fprintf(stream, "%f", va_arg(ap, double));
            break;

        case 'o':
            fprintf(stream, "%o", va_arg(ap, unsigned int));
            break;

        case 'p':
            fprintf(stream, "%p", va_arg(ap, void *));
            break;

        case 'q':
            fprintf(stream, "%llu", va_arg(ap, unsigned long long int));
            break;

        case 's':
            fprintf(stream, "\"%s\"", va_arg(ap, char *));
            break;

        case 'u':
            fprintf(stream, "%u", va_arg(ap, unsigned int));
            break;

        case 'x':
            fprintf(stream, "%x", va_arg(ap, unsigned int));
            break;

        case 'z':
            fprintf(stream, "%zu", va_arg(ap, size_t));
            break;

        default:
            abort();
        }
    }

    fprintf(stream, " }\n");
    fflush(stream);
    fsync(fileno(stream));
    funlockfile(stream);
}

void
random_buf(void *buf, size_t size)
{
    uint32_t number = 0;
    for (size_t i = 0; i < size; ++i) {
        if ((i % sizeof(uint16_t)) == 0) {
            number = random();
        }

        ((uint8_t *)buf)[i] = (number >> (8 * (i % sizeof(uint16_t)))) & 0xff;
    }
}

int
main(int argc, char *argv[])
{
    int c = 0;
    enum
    {
        OPT_VERSION = CHAR_MAX + 1,
        OPT_BUS_NUM,
        OPT_DEVICE_NUM,
    };
    /* clang-format off */
    static struct option longopts[] = {
        {"bus",         required_argument, NULL, 'B'             },
        {"device",      required_argument, NULL, 'D'             },
        {"function",    required_argument, NULL, 'F'             },
        {"debug",       no_argument,       NULL, 'd'             },
        {"generate",    no_argument,       NULL, 'g'             },
        {"help",        no_argument,       NULL, 'h'             },
        {"output",      required_argument, NULL, 'o'             },
        {"quiet",       no_argument,       NULL, 'q'             },
        {"seed",        required_argument, NULL, 's'             },
        {"timeout",     required_argument, NULL, 't'             },
        {"verbose",     no_argument,       NULL, 'v'             },
        {"version",     no_argument,       NULL, OPT_VERSION     },
        {NULL,          0,                 NULL, 0               }
    };
    /* clang-format on */
    static int longindex = 0;
    unsigned long bus = 0;
    unsigned long device = 0;
    unsigned long function = 0;
    unsigned long bus_num = 0;
    unsigned long device_num = 0;
    int debug = 0;
    int generate = 0;
    char *input = NULL;
    char *output = NULL;
    int quiet = 0;
    unsigned long seed = 1;
    int timeout = 5;
    int verbose = 0;
    while ((c = getopt_long(argc, argv, "B:D:F:dgho:qs:t:v", longopts, &longindex)) != -1) {
        switch (c) {
        case 'B':
            errno = 0;
            bus = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            if (bus > 255) {
                fprintf(stderr, "%s: Invalid PCI bus number.\n", __func__);
                exit(EXIT_FAILURE);
            }

            break;

        case 'D':
            errno = 0;
            device = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            if (device > 31) {
                fprintf(stderr, "%s: Invalid PCI device number.\n", __func__);
                exit(EXIT_FAILURE);
            }

            break;

        case 'F':
            errno = 0;
            function = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            if (function > 7) {
                fprintf(stderr, "%s: Invalid PCI function number.\n", __func__);
                exit(EXIT_FAILURE);
            }

            break;

        case OPT_BUS_NUM:
            errno = 0;
            bus_num = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            if (bus_num > 1) {
                fprintf(stderr, "%s: Invalid ATA bus number.\n", __func__);
                exit(EXIT_FAILURE);
            }

            break;

        case OPT_DEVICE_NUM:
            errno = 0;
            device_num = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            if (device_num > 1) {
                fprintf(stderr, "%s: Invalid ATA device number.\n", __func__);
                exit(EXIT_FAILURE);
            }

            break;

        case 'd':
            debug = 1;
            break;

        case 'g':
            generate = 1;
            break;

        case 'h':
            usage();
            exit(EXIT_FAILURE);

        case 'o':
            output = optarg;
            break;

        case 'q':
            quiet = 1;
            break;

        case 's':
            errno = 0;
            seed = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            break;

        case 't':
            errno = 0;
            timeout = strtoul(optarg, NULL, 0);
            if (errno != 0) {
                perror("strtoul");
                exit(EXIT_FAILURE);
            }

            break;

        case 'v':
            verbose = 1;
            break;

        case OPT_VERSION:
            version();
            exit(EXIT_FAILURE);

        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    FILE *stream = stdout;
    if (output != NULL) {
        stream = fopen(output, "a+");
        if (stream == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }

    if (iopl(3) == -1) {
        perror("iopl");
        exit(EXIT_FAILURE);
    }

    ata_controller_set_error_handler(default_error_handler);
    ata_controller_t *ata_controller = ata_controller_create(bus, device, function, bus_num, timeout);
    if (ata_controller == NULL) {
        perror("ata_controller_create");
        fclose(stream);
        exit(EXIT_FAILURE);
    }

    ata_fuzzer_set_error_handler(default_error_handler);
    ata_fuzzer_t *ata_fuzzer = ata_fuzzer_create(ata_controller, device_num);
    if (ata_fuzzer == NULL) {
        perror("ata_fuzzer_create");
        goto err;
    }

    ata_fuzzer_set_log_handler(ata_fuzzer, default_log_handler);
    ata_fuzzer_set_log_stream(ata_fuzzer, stream);
    if (generate) {
        srandom(seed);
        for (;;) {
            uint8_t buf[ATA_FUZZER_MAX_INPUT];
            random_buf(buf, sizeof(buf));
            FILE *stream = fmemopen(buf, sizeof(buf), "r");
            if (stream == NULL) {
                perror("fmemopen");
                goto err;
            }

            ata_fuzzer_iterate(ata_fuzzer, stream);
            fclose(stream);
        }
    } else {
        if (argv[optind] != NULL) {
            input = argv[optind];
        }

        FILE *stream = stdin;
        if (input != NULL) {
            FILE *stream = fopen(input, "r");
            if (stream == NULL) {
                perror("fopen");
                exit(EXIT_FAILURE);
            }
        }

        ata_fuzzer_iterate(ata_fuzzer, stream);
        fclose(stream);
    }

    ata_fuzzer_destroy(ata_fuzzer);
    ata_controller_destroy(ata_controller);
    fclose(stream);
    exit(EXIT_SUCCESS);

err:
    ata_fuzzer_destroy(ata_fuzzer);
    ata_controller_destroy(ata_controller);
    fclose(stream);
    exit(EXIT_FAILURE);
}
