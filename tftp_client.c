/*
Name: Shreya yadav (25028_049)
Project: TFTP File Transfer using UDP in C
Language Used: C Programming
Platform: Linux

Project Overview

This project implements a TFTP (Trivial File Transfer Protocol) Client–Server application using the C programming language and UDP sockets. The system allows file transfer between a client and server over a network.
The client sends requests to the server to either download a file (RRQ – Read Request) or upload a file (WRQ – Write Request). The server processes these requests and transfers data in small blocks using UDP communication.

Features

File upload from client to server using put command
File download from server to client using get command
Supports multiple transfer modes (default, octet, net-ascii)
Uses UDP socket communication
Implements TFTP packet structure
Includes error handling and acknowledgment packets

Working

The client connects to the server using the server IP address.
The client sends a request packet containing the filename and transfer mode.
If the client sends a get request, the server reads the file and sends it in data packets.
If the client sends a put request, the server receives the file and writes it to disk.
Each data packet is acknowledged using ACK packets to ensure reliable transfer.

Commands Used
connect <server_ip>
put <filename>
get <filename>
mode <default | octet | net-ascii>
Conclusion

This project demonstrates the implementation of a TFTP protocol using UDP sockets in C. It helps in understanding network communication, file transfer mechanisms, packet handling, and client–server architecture in embedded and network programming.
*/

#include "tftp.h"
#include "tftp_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
tftp_packet Req_packet;
int connect_flag = 0;
int mode_flag = 1;
int Data_size = 512; //  To know in  different modes  how many bits data we have to send
int mode_enable = 0;
// char prev = 0;
int main()
{
    char command[256];

    tftp_client_t client;

    // Initialize client structure with zero
    memset(&client, 0, sizeof(client));

    // Store size of server socket structure
    client.server_len = sizeof(struct sockaddr_in);

    // Initialize global request packet
    memset(&Req_packet, 0, sizeof(Req_packet));

    // Clear terminal screen
    system("clear");

    while (1)
    {
        if (mode_enable == 0)
        {
            mode_flag = 1;
            Data_size = 512;
            strcpy(Req_packet.body.request.mode, "default");
        }
        printf("\ntftp> ");
        // if(mode_flag==1)
        // strcpy(Req_packet.body.request.mode ,"default");
        // Data_size = 512 ;
        // mode_flag = 1 ;
        // Read full command from user
        fgets(command, sizeof(command), stdin);

        // Remove newline character from input
        command[strcspn(command, "\n")] = 0;

        // Process user command
        process_command(&client, command);
    }

    return 0;
}

// Function to process commands
void process_command(tftp_client_t *client, char command[])
{

    char *cmd = NULL;
    char *IP_Addr;

    // Extract first token (main command)
    cmd = strtok(command, " ");

    printf("command : %s\n", cmd);

    // If command is "connect"
    if (!strcmp(cmd, "connect"))
    {
        // Extract IP address
        IP_Addr = strtok(NULL, " ");
        if (IP_Addr == NULL)
        {
            printf("Please Enter the IP address\n");
            return;
        }

        printf("IP address : %s\n", IP_Addr);

        // Validate IP address format
        int ip_valid = Ip_validation(IP_Addr);

        if (ip_valid)
        {
            // Create UDP socket and prepare server address
            connect_to_server(client, IP_Addr, PORT);
        }
        else
        {
            return;
        }
    }

    // If command is "put"
    else if (!strcmp(cmd, "put"))
    {
        // Extract filename
        char *filename = strtok(NULL, " ");

        printf("filename : %s\n", filename);

        // Check if file exists
        int fd = open(filename, O_RDONLY);

        if (fd < 0)
        {
            perror("File ");
            return;
        }

        close(fd);

        // Start write request
        put_file(client, filename);
    }

    // If command is "get"
    else if (!strcmp(cmd, "get"))
    {
        char *filename;

        // Extract filename
        filename = strtok(NULL, " ");

        printf("filename :%s\n", filename);

        // Create or truncate file before receiving
        int fd = open(filename, O_TRUNC | O_CREAT | O_WRONLY, 0777);

        if (fd < 0)
        {
            perror("File ");
            return;
        }

        close(fd);

        // Start read request
        get_file(client, filename);
    }

    // If command is "mode"
    else if (!strcmp(cmd, "mode"))
    {
        // Allocate memory for mode string
        char *mode = malloc(20 * sizeof(char));

        // Extract mode argument
        mode = strtok(NULL, " ");

        printf("Mode : %s\n", mode);

        // Default mode selection
        if (!strcmp(mode, "default"))
        {
            mode_flag = 1;
            Data_size = 512;
        }

        // Octet (binary) mode
        else if (!strcmp(mode, "octet"))
        {
            mode_flag = 2;
            Data_size = 1;
        }

        // Netascii mode
        else if (!strcmp(mode, "net-ascii"))
        {
            mode_flag = 3;
            Data_size = 512;
        }

        else
        {
            printf("Error : Invalid mode selection\n");
            return;
        }
        // copy mode in request packet
        strcpy(Req_packet.body.request.mode, mode);
        mode_enable = 1;
    }
}

// This function is to initialize socket with given server IP, no packets sent to server in this function
void connect_to_server(tftp_client_t *client, char *ip, int port)
{
    int sock_fd;

    // Create UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock_fd < 0)
    {
        printf("Error :Could not create the Socket\n");
        return;
    }

    // Store socket descriptor
    client->sockfd = sock_fd;

    // Set address family
    client->server_addr.sin_family = AF_INET;

    // Convert IP string to network format
    client->server_addr.sin_addr.s_addr = inet_addr(ip);

    // Set port number in network byte order
    client->server_addr.sin_port = htons(PORT);

    // Mark client as connected
    connect_flag = 1;

    printf("Connected to the server.\n");
}

void put_file(tftp_client_t *client, char *filename)
{
    // Check if client is connected
    if (connect_flag == 0)
    {
        printf("Error : Client is not ready for put operation \n");
        return;
    }

    // Set opcode as Write Request
    Req_packet.opcode = WRQ;

    // Send WRQ packet
    send_request(client->sockfd, client->server_addr, filename, WRQ);

    // Wait for ACK
    receive_request(client->sockfd, client->server_addr, filename, WRQ);
}

void get_file(tftp_client_t *client, char *filename)
{
    // Check if client is connected
    if (connect_flag == 0)
    {
        printf("Error : Client is not ready to send the Data \n");
        return;
    }

    // Set opcode as Read Request
    Req_packet.opcode = RRQ;

    // Send RRQ packet
    send_request(client->sockfd, client->server_addr, filename, RRQ);

    // Wait for response
    receive_request(client->sockfd, client->server_addr, filename, RRQ);
}

void disconnect(tftp_client_t *client)
{
    // close sockfd
    close(client->sockfd);
    connect_flag = 0;
}
void send_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode)
{
    // Set opcode in packet
    Req_packet.opcode = opcode;

    // Copy filename into request body
    strcpy(Req_packet.body.request.filename, filename);

    // Send packet to server
    int ret = sendto(sockfd, &Req_packet, sizeof(Req_packet), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));

    if (ret)
        printf("Request sent\n");
    else
        printf("Error : Unable to send request\n");
}

void receive_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode)
{
    tftp_packet Req_packet;
    memset(&Req_packet, 0, sizeof(Req_packet));

    // Receive response from server
    printf("Waiting for the Ack\n");
    recvfrom(sockfd, &Req_packet, sizeof(Req_packet), 0, NULL, NULL);
    printf("ACK received\n");
    int R_ACK = ntohs(Req_packet.opcode);
    // Check if server sent error
    if (R_ACK == ERROR)
    {
        printf("Error at Server side-> %s\n", Req_packet.body.error_packet.error_msg);
        return;
    }

    // If ACK received
    else if (R_ACK == ACK)
        printf("Ack received -> Server is ready for Communication\n");

    // Call appropriate function based on request type
    if (opcode == RRQ)
        receive_file(sockfd, server_addr, sizeof(server_addr), filename);
    else
        send_file(sockfd, server_addr, sizeof(server_addr), filename);
}

int Ip_validation(char ip_addr[20])
{
    char ip_octet[6][10];
    char buff[10];
    int i = 0, j = 0, k = 0, octet = 0, dot_count = 0;
    // if (port < 1023)
    // {
    //     printf("Error : Port number is not valid\n");
    //     return 0;
    // }

    while (ip_addr[i] != '\0')
    {
        if (ip_addr[i] == '.')
        {
            dot_count++;
            buff[j] = '\0';
            //  printf("%s \n", buff);
            strcpy(ip_octet[k++], buff);
            j = 0;
            i++;
        }
        else
            buff[j++] = ip_addr[i++];
    }

    buff[j] = '\0'; // for last octete
    strcpy(ip_octet[k++], buff);

    if (dot_count != 3 && k != 4) // Number of dots -> validate and no of octetes should be 4
    {
        printf("Error : Invalid IP address\n");
        return 0;
    }

    // to validate the each octet of IP address
    for (int l = 0; l < k; l++)
    {
        if (strlen(ip_octet[l]) == 0)
        {
            printf("Error : Invalid IP address\n");
            return 0;
        }
        octet = atoi(ip_octet[l]);
        if (l == 0 && octet > 127)
        {
            printf("Error : Invalid IP address\n");
            return 0;
        }
        else if (octet > 255)
        {
            printf("Error : Invalid IP address\n");
            return 0;
        }
        // printf(" ip octet %d -> %d\n",l , octet);
    }
}

int convert_netascii(int fd, char buff[]) // it will  return how many bytes readed
{
    int i = 0, j = 0;
    int ret = 1;
    char ch;
    if ()
        while (j < 512 && ret != 0)
        {
            ret = read(fd, &ch, 1);
            if (ret == 0)
                break;
            if (ch == '\n')
            {
                if (prev == '\r')
                {
                    if (j > 0)
                        buff[--j] = ch;
                    else
                    {
                        buff[j] = ch;
                    }
                }
                else
                {
                    buff[j++] = '\r';
                    if (j < 512)
                        buff[j++] = '\n'; // to check  if last char is '/r' in previous buffer then how to add '\n' at  0 th index
                }
            }
            else
                buff[j++] = ch;

            prev = ch;
        }
    return j;
}
