#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> /* socket() */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h>  /* inet_pton() */
#include <unistd.h>     /* close() */
#include <string.h>
#include <errno.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

int main(int argc, char** argv) {

    int             sockfd;                 /* Desktryptor gniazda. */
    int             retval;                 /* Wartosc zwracana przez funkcje. */
    struct          sockaddr_in remote_addr;/* Gniazdowa struktura adresowa. */
    socklen_t       addr_len;               /* Rozmiar struktury w bajtach. */
    FILE            *file;
    EVP_PKEY        *pkey = NULL;
    size_t    message_len, sig_len;

    unsigned char buff[1024] = "Laboratorium PUS.";


    if (argc != 3) {
        fprintf(stderr, "Invocation: %s <IPv4 ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /******************************************************************************/
    /*                            RSA SIGNING (patr 1)                            */
    /******************************************************************************/

    /* Otworzenie pliku do odczytu: */
    file = fopen("private.key", "r");
    if (file == NULL) {
        perror("fopen()");
        exit(EXIT_FAILURE);
    }
    /******************************************************************************/

    /* Utworzenie gniazda dla protokolu UDP: */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    /* Wyzerowanie struktury adresowej dla adresu zdalnego (serwera): */
    memset(&remote_addr, 0, sizeof(remote_addr));
    /* Domena komunikacyjna (rodzina protokolow): */
    remote_addr.sin_family = AF_INET;

    /* Konwersja adresu IP z postaci kropkowo-dziesietnej: */
    retval = inet_pton(AF_INET, argv[1], &remote_addr.sin_addr);
    if (retval == 0) {
        fprintf(stderr, "inet_pton(): invalid network address!\n");
        exit(EXIT_FAILURE);
    } else if (retval == -1) {
        perror("inet_pton()");
        exit(EXIT_FAILURE);
    }

    remote_addr.sin_port = htons(atoi(argv[2])); /* Numer portu. */
    addr_len = sizeof(remote_addr); /* Rozmiar struktury adresowej w bajtach. */

    /******************************************************************************/
    /*                            RSA SIGNING (patr 2)                            */
    /******************************************************************************/
    fprintf(stdout, "Signing message...\n");

    /* Zaladowanie tekstowych opisow bledow: */
    ERR_load_crypto_strings();

    /* Zaladowanie nazw algorytmow do pamieci: */
    OpenSSL_add_all_ciphers();

    /* Wczytanie klucza prywatnego z pliku: */
    pkey = PEM_read_PrivateKey(file, &pkey, NULL, NULL);
    if (!pkey) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    /* Zamkniecie pliku: */
    fclose(file);

    /* Rozmiar wiadomosci w buforze: */
    message_len = strlen(buff);

    /*
     * Utworzenie podpisu cyfrowego. Podpis jest umieszczany w buforze 'buff' za
     * wiadomoscia:
     */
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) {
        fprintf(stderr,"EVP_MD_CTX_new failed");
        ERR_print_errors_fp(stderr);
    }

    // Inicjalizacja podpisywania
    if (EVP_DigestSignInit(md_ctx, NULL, EVP_sha256(), NULL, pkey) <= 0) {
        fprintf(stderr,"EVP_DigestSignInit failed");
        ERR_print_errors_fp(stderr);
    }

    // Wprowadzenie danych do podpisania
    if (EVP_DigestSignUpdate(md_ctx, buff, message_len) <= 0) {
        fprintf(stderr,"EVP_DigestSignUpdate failed");
        ERR_print_errors_fp(stderr);
    }

    // Uzyskanie długości podpisu
    if (EVP_DigestSignFinal(md_ctx, NULL, &sig_len) <= 0) {
        fprintf(stderr,"EVP_DigestSignFinal (get length) failed");
        ERR_print_errors_fp(stderr);
    }

    // Wygenerowanie podpisu
    if (EVP_DigestSignFinal(md_ctx, buff + message_len, &sig_len) <= 0) {
        fprintf(stderr,"EVP_DigestSignFinal failed");
        ERR_print_errors_fp(stderr);
    }

    /* Usuniecie struktury i klucza z pamieci: */
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(pkey);

    /* Usuniecie nazw algorytmow z pamieci: */
    EVP_cleanup();

    /* Zwolnienie tekstowych opisow bledow: */
    ERR_free_strings();

    fprintf(stdout, "Signature size: %lu bytes.\n", sig_len);
    /******************************************************************************/

    fprintf(stdout, "Sending signed message to %s.\n", argv[1]);

    /* sendto() wysyla dane na adres okreslony przez strukture 'remote_addr': */
    retval = sendto(
                 sockfd,
                 buff, message_len + sig_len,
                 0,
                 (struct sockaddr*)&remote_addr, addr_len
             );

    if (retval == -1) {
        perror("sendto()");
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    exit(EXIT_SUCCESS);
}
