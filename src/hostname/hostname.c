#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <getopt.h>

#define NAME "hostname (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Isaac Song"

static void printVersion(void) {
    printf("%s version %s\n", NAME, VERSION);
}

static void printHelp(void) {
    printf("Usage: %s [OPTION]...\n", NAME);
    printf("Display or set the system's hostname.\n");
    printf("\n");
    printf("  -h, --help           display this help message and exit\n");
    printf("  -v, --version        output version information and exit\n");
    printf("  -i, --ip-address     display IP address for the hostname\n");
    printf("  -d, --dnsdomainname  display DNS domain name\n");
    printf("\n");
    printf("Report bugs to %s\n", AUTHOR);
}

static void printShortHostname(void) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("Hostname: %s\n", hostname);
    } else {
        fprintf(stderr, "Error getting hostname: %s\n", strerror(errno));
    }
}

static void printDNSDomainName(void) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        fprintf(stderr, "Error getting hostname: %s\n", strerror(errno));
        return;
    }

    struct hostent *host = gethostbyname(hostname);
    if (!host) {
        fprintf(stderr, "Error getting DNS domain name: %s\n", hstrerror(h_errno));
        return;
    }

    char *domain = strchr(host->h_name, '.');
    if (domain) {
        printf("DNS Domain Name: %s\n", domain + 1);
    } else {
        printf("DNS Domain Name: (none)\n");
    }
}

static void printIPAddresses(void) {
    char hostname[256];
    struct addrinfo hints, *res, *p;
    int status;

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        fprintf(stderr, "Error getting hostname: %s\n", strerror(errno));
        return;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Use AF_INET for IPv4, AF_INET6 for IPv6, AF_UNSPEC for both
    hints.ai_socktype = SOCK_STREAM; // Can be set to 0 if socket type is irrelevant

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "Error getting address info: %s\n", gai_strerror(status));
        return;
    }

    printf("IP Addresses for %s:\n", hostname);
    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        char ipstr[INET6_ADDRSTRLEN];

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            inet_ntop(AF_INET, addr, ipstr, sizeof(ipstr));
            printf("  IPv4: %s\n", ipstr);
        } else if (p->ai_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            inet_ntop(AF_INET6, addr, ipstr, sizeof(ipstr));
            printf("  IPv6: %s\n", ipstr);
        }
    }

    freeaddrinfo(res);
}

int main(int argc, char **argv) {
    int opt;
    int option_index = 0;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"ip-address", no_argument, 0, 'i'},
        {"dnsdomainname", no_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    // If no arguments are given, display the hostname
    if (argc == 1) {
        printShortHostname();
        return EXIT_SUCCESS;
    }

    // Parse command-line options
    while ((opt = getopt_long(argc, argv, "hvid", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                printHelp();
                return EXIT_SUCCESS;
            case 'v':
                printVersion();
                return EXIT_SUCCESS;
            case 'i':
                printIPAddresses();
                return EXIT_SUCCESS;
            case 'd':
                printDNSDomainName();
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Unrecognized option\n");
                printHelp();
                return EXIT_FAILURE;
        }
    }

    return EXIT_FAILURE;
}
