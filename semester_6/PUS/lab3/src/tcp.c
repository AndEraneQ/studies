#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "checksum.h"

#define SOURCE_PORT 5050
#define SOURCE_ADDRESS "192.0.2.1"

struct phdr {
    struct in_addr ip_src, ip_dst;
    unsigned char unused;
    unsigned char protocol;
    unsigned short length;
};

void print_hex(const unsigned char *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

int main(int argc, char** argv) {
    int sockfd, socket_option, retval;
    struct addrinfo hints, *rp, *result;
    unsigned short checksum;
    unsigned char datagram[sizeof(struct ip) + sizeof(struct tcphdr)] = {0};

    struct ip *ip_header = (struct ip *)datagram;
    struct tcphdr *tcp_header = (struct tcphdr *)(datagram + sizeof(struct ip));

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <HOSTNAME OR IP> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_TCP;

    retval = getaddrinfo(argv[1], NULL, &hints, &result);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }

    socket_option = 1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) continue;

        retval = setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &socket_option, sizeof(int));
        if (retval == -1) {
            perror("setsockopt()");
            exit(EXIT_FAILURE);
        } else {
            break;
        }
    }

    if (rp == NULL) {
        fprintf(stderr, "Client failure: could not create socket.\n");
        exit(EXIT_FAILURE);
    }

    ip_header->ip_hl = 5;
    ip_header->ip_v = 4;
    ip_header->ip_tos = 0;
    ip_header->ip_len = htons(sizeof(struct ip) + sizeof(struct tcphdr));
    ip_header->ip_id = htons(0);
    ip_header->ip_off = 0;
    ip_header->ip_ttl = 255;
    ip_header->ip_p = IPPROTO_TCP;
    ip_header->ip_src.s_addr = inet_addr(SOURCE_ADDRESS);
    ip_header->ip_dst.s_addr = ((struct sockaddr_in*)rp->ai_addr)->sin_addr.s_addr;
    ip_header->ip_sum = 0;

    tcp_header->th_sport = htons(SOURCE_PORT);
    tcp_header->th_dport = htons(atoi(argv[2]));
    tcp_header->th_seq = htonl(rand());
    tcp_header->th_ack = 0;
    tcp_header->th_off = 5;
    tcp_header->th_flags = TH_SYN;
    tcp_header->th_win = htons(65535);
    tcp_header->th_sum = 0;
    tcp_header->th_urp = 0;

    fprintf(stdout, "Sending TCP SYN packet...\n");
    print_hex(datagram, ntohs(ip_header->ip_len));

    for (;;) {
        fprintf(stdout, "Packet bytes before send:\n");
        print_hex(datagram, ntohs(ip_header->ip_len));
        
        retval = sendto(sockfd, datagram, ntohs(ip_header->ip_len), 0, rp->ai_addr, rp->ai_addrlen);
        if (retval == -1) {
            perror("sendto()");
        }
        sleep(1);
    }

    freeaddrinfo(result);
    close(sockfd);
    return EXIT_SUCCESS;
}