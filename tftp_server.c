#include "tftp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
int Data_size = 512;
int mode_flag = 1 ;
void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, tftp_packet *packet);

int main()
{
    system("clear");
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    tftp_packet packet;
    char buff[10];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("Error : Could not create the Socket\n");
        return 0;
    }

    // Set socket timeout option
    // TODO Use setsockopt() to set timeout option

    // Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind the socket

    if ((bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
    {
        perror("bind");
        exit(1);
    }
    printf("TFTP Server listening on port %d...\n", PORT);

    // Main loop to handle incoming requests
    while (1)
    {
        Data_size = 512;
        mode_flag=1 ;
        printf("\nServer is waiting for the Request....\n");
        int n = recvfrom(sockfd, (void *)&packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0)
        {
            perror("Receive failed or timeout occurred");
            continue;
        }

        handle_client(sockfd, client_addr, client_len, &packet);
    }

    close(sockfd);
    return 0;
}

void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, tftp_packet *packet)
{
    // Extract the TFTP operation (read or write) from the received packet
    // and call send_file or receive_file accordingly

    printf("Request type :%d\n", packet->opcode);
    printf("filename : %s\n", packet->body.request.filename);
    printf("Mode :%s\n",packet->body.request.mode);
    char filename[30];
    strcpy(filename, packet->body.request.filename);
    if (!strcmp(packet->body.request.mode, "default"))
    {
        mode_flag = 1 ;
        Data_size = 512;   
    }
    else if (!strcmp(packet->body.request.mode, "octet")) // To check data transmission mode
    {
        // printf("octet\n");
        mode_flag = 2 ;
        Data_size = 1;
    }
     else if(!strcmp(packet->body.request.mode, "net-ascii"))
    {
        mode_flag = 3 ;
        Data_size = 512;
    }
    else
    {
        printf("Error : Invalid Mode\n");
        return ;
    }

    socklen_t sz_of_sock_addr = sizeof(struct sockaddr_in);
    tftp_packet Ack_packet;
    switch (packet->opcode)
    {
    case RRQ:
    {
        tftp_packet Ack_packet;
        memset(&Ack_packet, 0, sizeof(Ack_packet));
        int r_ret, Send_data_bytes, data_pack_count = 0;
        int fd = open(filename, O_RDONLY, 0777);
        if (fd < 0)
        {
            perror("open");
            //  char buff[200];
            //  fread(buff,sizeof(buff),1,stderr)`
            Ack_packet.opcode =htons(ERROR);
            Ack_packet.body.error_packet.error_code = htons(1); // File not found
            strcpy(Ack_packet.body.error_packet.error_msg, strerror(errno));  // to get error string
             int ret = sendto(sockfd, &Ack_packet, sizeof(Ack_packet), 0, (struct sockaddr *)&client_addr, client_len);
             printf("n_ack sent ->%d\n",ret);
            printf("Error packet sent successfully\n");
            return;
        }
        close(fd);
        Ack_packet.opcode = ACK;
        Ack_packet.body.ack_packet.block_number = 0; // file open suucessfully
        sendto(sockfd, &Ack_packet, sizeof(Ack_packet), 0, (struct sockaddr *)&client_addr, client_len);
        send_file(sockfd, client_addr, client_len, filename);
        break;
    }
    case WRQ:
    {
        // Implement file receiving logic here

        memset(&Ack_packet, 0, sizeof(Ack_packet));
        int data_pkt_count = 0, w_ret;
        int Rec_data_byts;
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
        if (fd < 0)
        {
            perror("open");
            Ack_packet.opcode =htons(ERROR);
            Ack_packet.body.error_packet.error_code = htons(1); // File not found
            strcpy(Ack_packet.body.error_packet.error_msg, strerror(errno));
            int ret = sendto(sockfd, &Ack_packet, sizeof(Ack_packet), 0, (struct sockaddr *)&client_addr, client_len);
             printf("n_ack sent ->%d\n",ret);
             printf("Error packet sent successfully\n");
            return;
        }
        close(fd);

        Ack_packet.opcode = ACK;
        Ack_packet.body.ack_packet.block_number = 0; // file open suucessfully

        sendto(sockfd, &Ack_packet, sizeof(Ack_packet), 0, (struct sockaddr *)&client_addr, client_len);
        printf("Ack send ->file opned successfully\n");
        receive_file(sockfd, client_addr, client_len, filename);
        break;
    }
    }
}
