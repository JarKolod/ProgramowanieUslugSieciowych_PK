/*
 * Uruchamianie:        $ ./client1 <adres IP> <numer portu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>




int main(int argc, char** argv) {

    int sockfd;
    int retval;                 
    struct sockaddr universal_server_addr;
    struct sockaddr_storage universal_server_addr_storage;
    struct addrinfo* universal_addr_info;
    socklen_t addr_len;
    char buff[256];
    char serv_buff[256];
    char host_buff[256];

    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4 or IPv6 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Zidentyfikowanie rodzaju adresu podanego przez uzytkownika
    if(getaddrinfo(argv[1], NULL, NULL, &universal_addr_info) != 0)
    {
        perror("getaddrinfo()");
        exit(EXIT_FAILURE);
    }

    if(universal_addr_info->ai_family == AF_INET)
    {
        printf("Given address is in IPv4 family\n");

        struct sockaddr_in ipv4_addr;
        memset(&ipv4_addr, 0, sizeof(ipv4_addr));
        ipv4_addr.sin_family = AF_INET;
        retval = inet_pton(AF_INET, argv[1], &ipv4_addr.sin_addr);
        if (retval == 0) {
            fprintf(stderr, "inet_pton(): invalid network address!\n");
            exit(EXIT_FAILURE);
        } else if (retval == -1) {
            perror("inet_pton()");
            exit(EXIT_FAILURE);
        }
        ipv4_addr.sin_port = htons(atoi(argv[2]));
        addr_len = sizeof(ipv4_addr);

        memcpy (&universal_server_addr_storage, &ipv4_addr, sizeof (ipv4_addr));
    }
    else if(universal_addr_info->ai_family == AF_INET6)
    {
        printf("Given address is in IPv6 family\n");

        struct sockaddr_in6 ipv6_addr;
        memset(&ipv6_addr, 0, sizeof(ipv6_addr));
        ipv6_addr.sin6_family = AF_INET6;
        retval = inet_pton(AF_INET6, argv[1], &ipv6_addr.sin6_addr);
        if (retval == 0)
        {
            fprintf(stderr, "inet_pton(): invalid network address!\n");
            exit(EXIT_FAILURE);
        }
        else if (retval == -1)
        {
            perror("inet_pton()");
            exit(EXIT_FAILURE);
        }

        memcpy (&universal_server_addr_storage, &ipv6_addr, sizeof (ipv6_addr));
    }
    else
    {
        printf("Not an IP address!\n");
        exit(EXIT_FAILURE);
    }
    
    // Utworzenie gniazda odpowiedniego typu
    sockfd = socket(universal_addr_info->ai_family, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    // Rozpoczencie poloczenia: 
    if (connect(sockfd, (const struct sockaddr *)&universal_server_addr_storage, addr_len) == -1) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }

    //Wypisanie danych
    memset(serv_buff, 0, 256);
    memset(host_buff, 0, 256);
    socklen_t storage_size;
    getsockname(sockfd, (struct sockaddr*)&universal_server_addr_storage, &storage_size);
    getnameinfo(
        (struct sockaddr*)&universal_server_addr_storage, storage_size,
        host_buff, 256,
        serv_buff, 256,
        NI_NUMERICHOST|NI_NUMERICSERV
    );

    printf(
        "Host IP: %s\nServer IP: %s\n",
        host_buff,
        serv_buff
    );

    // Odebranie danych:
    memset(buff, 0, 256);
    retval = read(sockfd, buff, sizeof(buff));
    printf("Received server response: %s\n", buff);

    // Zamkniecie clienta:
    printf("Closing socket (sending FIN to server)...\n");
    close(sockfd);
    printf("Terminating application. After receiving FIN from server, "
            "TCP connection will go into TIME_WAIT state.\n");
    exit(EXIT_SUCCESS);
}
