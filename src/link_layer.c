// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd = openPort(connectionParameters.serialPort);
    
    if(connectionParameters.role == LlTx)
        llopenTx(fd);
    else if(connectionParameters.role == LlRx)
        llopenRx(fd);
    else
        return -1;

    return 1;
}

int llopenTx(int fd){

    unsigned char SUPFRAME[SUPFRAME_SIZE] = {FLAG, A_SET, C_SET, A_SET^C_SET, FLAG};

    int bytes = write(fd, SUPFRAME, SUPFRAME_SIZE);
    printf("%d bytes written\n", bytes);

    // Wait until all bytes have been written to the serial port
    sleep(1);

    bytes = read(fd, SUPFRAME, 1);

    int state = 0;
    int a_ua = 0;
    int c_ua = 0;

    printf("fds");
    switch (state){
    
        case 0 :{
            if(SUPFRAME[0]!=FLAG){
                state=0;
                break;
            }
            printf("0x%02X\n",SUPFRAME[0]);
            state = 1;
        }
        case 1: {
            if(SUPFRAME[0]!=A_UA){
                state=0;
                break;
            }
            else if(SUPFRAME[1]==FLAG){
                state=1;
                break;
            }
            printf("0x%02X\n",SUPFRAME[0]);
            state=2;
            a_ua = SUPFRAME[0];
        }
        case 2: {
            if(SUPFRAME[0]!=C_UA){
                state = 0;
                break;
            }
            else if(SUPFRAME[0]==FLAG){
                state=1;
                break;
            }
            printf("0x%02X\n",SUPFRAME[0]);
            state=3;
            c_ua = SUPFRAME[0];
        }   
        case 3: {
            if(SUPFRAME[0]!=(a_ua^c_ua)){
                state=0;
                break;
            }
            else if(SUPFRAME[0]==FLAG){
                state=1;
                break;
            }
            printf("0x%02X\n",SUPFRAME[0]);
            state = 4;
        }   
        case 4:{
            if(SUPFRAME[0]!=FLAG){
                state=0;
                break;
            }
            printf("0x%02X\n",SUPFRAME[0]);
            state = 5;
        }
        case 5:{
            state = 0;
            printf("0x%02X\n",SUPFRAME[0]);
            break;
        }
    }        
    return fd;
}

int llopenRx(int fd){


    unsigned char SUPFRAME[SUPFRAME_SIZE] = {0};

    int bytes = read(fd, SUPFRAME, 1);

    int state=0; //1-FLAG_RCV, 2-A_RCV, 3-C_RCV, 4-BCC_OK, 5-STOP

    int a_set = 0;
    int c_set = 0;

    switch (state){
            case 0 :{
                if(SUPFRAME[0]!=FLAG){
                    state=0;
                    break;
                }
                printf("0x%02X\n",SUPFRAME[0]);
                state = 1;
            }
            case 1: {
                if(SUPFRAME[0]!=A_SET){
                    state=0;
                    break;
                }
                else if(SUPFRAME[0]==FLAG){
                    state=1;
                    break;
                }
                printf("0x%02X\n",SUPFRAME[0]);
                state = 2;
                a_set = SUPFRAME[0];
            }
            case 2: {
                if(SUPFRAME[0]!=C_SET){
                    state = 0;
                    break;
                }
                else if(SUPFRAME[0]==FLAG){
                    state=1;
                    break;
                }
                printf("0x%02X\n",SUPFRAME[0]);
                state = 3;
                c_set = SUPFRAME[0];
            }   
            case 3: {
                if(SUPFRAME[0]!=(a_set^c_set)){
                    state=0;
                    break;
                }
                else if(SUPFRAME[0]==FLAG){
                    state=1;
                    break;
                }
                printf("0x%02X\n",SUPFRAME[0]);
                state = 4;
            }   
            case 4:{
                if(SUPFRAME[0]!=FLAG){
                    state=0;
                    break;
                }
                printf("0x%02X\n",SUPFRAME[0]);
                state = 5;
            }
            case 5:{
                state = 0;
                break;
            }        

    }

    unsigned char TxSUPFRAME[SUPFRAME_SIZE] = {FLAG, A_UA, C_UA, A_UA^C_UA, FLAG};

    bytes = write(fd, TxSUPFRAME, SUPFRAME_SIZE);

    printf("%d bytes written\n", bytes);

    return fd;
}


int openPort(const char* serialPort){
    // Open serial port device for reading and writing, and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        perror(serialPort);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    return fd;
}
////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    return 1;
}
