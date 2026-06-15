//  /* Common file for server & client */

#include "tftp.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
extern int Data_size;
extern int mode_flag;
char prev = 0;
int prev_nline = 0;

void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
    // Implement file sending logic here
    tftp_packet send_packet;
    tftp_packet rec_packet;
    int r_ret, Send_data_bytes, data_pack_count = 0;
    int fd = open(filename, O_RDONLY, 0777);
    if (fd < 0)
    {
        perror("open");
        //  char buff[200];
        //  fread(buff,sizeof(buff),1,stderr)
        send_packet.opcode = htons(ERROR);
        send_packet.body.error_packet.error_code = htons(1);              // File not found
        strcpy(send_packet.body.error_packet.error_msg, strerror(errno)); // to get error string
        sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&client_addr, client_len);
        return;
    }

    do
    {

        // Clear send packet and receive packet before filling
        memset(&send_packet, 0, sizeof(send_packet));
        memset(&rec_packet, 0, sizeof(rec_packet));

        // If netascii mode, convert before sending
        if (mode_flag == 3)
            r_ret = convert_netascii(fd, send_packet.body.data_packet.data);

        // Otherwise read normally
        else
            r_ret = read(fd, send_packet.body.data_packet.data, Data_size);

        printf("%d bytes readed from the file\n", r_ret);
        data_pack_count++;
        send_packet.body.data_packet.block_number = data_pack_count;
        send_packet.body.data_packet.nof_data_bytes = r_ret; // how many bytes sended(to client)
        Send_data_bytes = sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&client_addr, client_len);
        printf("%d  bytes sent \n", Send_data_bytes);
        recvfrom(sockfd, &rec_packet, sizeof(rec_packet), 0, NULL, NULL);
        if (rec_packet.opcode == ERROR)
        {
            printf("Error occurred  ->%s\n", rec_packet.body.error_packet.error_msg);
            lseek(fd, r_ret, SEEK_CUR); // TO RESEND SAME PACKET AGAIN ;
            data_pack_count--;
        }
        else if (rec_packet.opcode == ACK)
        {
            printf("Ack received for the data packet %d\n", rec_packet.body.ack_packet.block_number);
        }

        printf("Data size = %d\n", Data_size);
    } while (r_ret == Data_size);
    close(fd);
    printf("File sent  Successfully\n");
}

void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
    // Implement file receiving logic here
    tftp_packet send_packet = {0};
    tftp_packet rec_packet = {0};
    int data_pkt_count = 0, w_ret;
    int Rec_data_byts;
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd < 0)
    {
        perror("open");
        printf("file is not open  for writing\n ");

        send_packet.opcode = ERROR;
        strcpy(send_packet.body.error_packet.error_msg, "Server File Don't have Write permission");
        sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&client_addr, client_len);
        return;
    }

    do
    {
        int Rec_data_byts = recvfrom(sockfd, &rec_packet, sizeof(rec_packet), 0, NULL, NULL);

        printf("%d bytes data received \n", Rec_data_byts);

        data_pkt_count++;

        if (rec_packet.body.data_packet.block_number == data_pkt_count) // write only if data packet matches
        {
            if (rec_packet.opcode == DATA)
                w_ret = write(fd, rec_packet.body.data_packet.data, rec_packet.body.data_packet.nof_data_bytes);
            printf("%d bytes data written in file\n", w_ret);

            send_packet.opcode = ACK;
            send_packet.body.ack_packet.block_number = data_pkt_count;
        }
        else
        {
            send_packet.opcode = ERROR;
            strcpy(send_packet.body.error_packet.error_msg, "Data Packet block number mismatch");
            data_pkt_count--;
        }

        sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&client_addr, client_len);

        printf("Data size = %d\n", Data_size);

    } while (w_ret == Data_size);
    printf("Loop breaked\n");
    close(fd);
}

int convert_netascii(int fd, char buff[]) // it will  return how many bytes readed
{
    int i = 0, j = 0;
    int ret = 1;
    char ch;
    if (prev_nline)
    {
        buff[j++] = '\n';
        prev_nline = 0;
    }
    while (j < 512 && ret != 0)
    {
        ret = read(fd, &ch, 1);
         printf("in the  net ascii function ->char ->%c readed from the file\n",ch) ;
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
                else
                    prev_nline = 1; // To add '\n'  in next data  packet
            }
        }
        else
            buff[j++] = ch;

        prev = ch;
    }
    return j;
}
