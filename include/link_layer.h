// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

typedef enum
{
    LlTx,
    LlRx,
} LinkLayerRole;

typedef struct
{
    char serialPort[50];
    LinkLayerRole role;
    int baudRate;
    int nRetransmissions;
    int timeout;
} LinkLayer;

typedef enum
{
    C_RR,
    C_REJ,
    NONE,
} FrameStatus;

#define BAUDRATE 38400

// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000

// MISC
#define FALSE 0
#define TRUE 1

#define SUPFRAME_SIZE 5 //

#define FLAG    0x7E
#define A_SET   0x03
#define C_SET   0x03
#define A_UA    0x01
#define C_UA    0x07
#define C_N(n)  (n << 6)
#define C_RR0   0X05
#define C_RR1   0X85
#define C_REJ0  0X01
#define C_REJ1  0X81
#define C_DISC  0x0B

// Open a connection using the "port" parameters defined in struct linkLayer.
// Return "1" on success or "-1" on error.
int llopen(LinkLayer connectionParameters);

int llopenTx(int fd, int retransmissionsCount, int timeout);
int llopenRx(int fd);

int openPort(const char* serialPort);

void alarmHandler(int signal);

// Send data in buf with size bufSize.
// Return number of chars written, or "-1" on error.
int llwrite(const unsigned char *buf, int bufSize);

// Receive data in packet.
// Return number of chars read, or "-1" on error.
int llread(unsigned char *packet);

// Close previously opened connection.
// if showStatistics == TRUE, link layer should print statistics in the console on close.
// Return "1" on success or "-1" on error.
int llclose(int showStatistics);



#endif // _LINK_LAYER_H_
