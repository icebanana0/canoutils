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
        perror("gethostname");
    }
}

static void printDNSDomainName(void) {
    char hostname[256];
    char *domain;
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct hostent *host = gethostbyname(hostname);
        if (host && host->h_name) {
            domain = strchr(host->h_name, '.');
            if (domain) {
                printf("DNS Domain Name: %s\n", domain + 1);
            } else {
                printf("DNS Domain Name: (none)\n");
            }
        } else {
            perror("gethostbyname");
        }
    } else {
        perror("gethostname");
    }
}

static void printIPAddresses(void) {
    char hostname[256];
    struct addrinfo hints, *res, *p;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname");
        return;
    }

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return;
    }

    printf("IP Addresses:\n");
    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        char ipstr[INET6_ADDRSTRLEN];

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        printf("  %s\n", ipstr);
    }

    freeaddrinfo(res);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        printShortHostname();
        return EXIT_SUCCESS;
    } else if (argc == 2) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printVersion();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printHelp();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--dnsdomainname") == 0 || strcmp(argv[1], "-d") == 0) {
            printDNSDomainName();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--ip-address") == 0 || strcmp(argv[1], "-i") == 0) {
            printIPAddresses();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Error: Unrecognized option '%s'\n", argv[1]);
            fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "Error: Too many arguments\n");
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return EXIT_FAILURE;
    }
}
