// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd = connection(connectionParameters.serialPort);
    unsigned char buf[BUF_SIZE] = {0};

    if(connectionParameters.role == LlTx){
        int bytes = write(fd, buf, BUF_SIZE);
        printf("%d bytes written\n", bytes);

        int state = 0;
        int a_ua = 0;
        int c_ua = 0;

        switch (state){
    
	        case 0 :{
		        if(buf[0]!=0x7E){
			        state=0;
			        break;
		        }
                state = 1;
                break;
            }
	        case 1: {
		        if(buf[0]!=0x03){
			        state=0;
			        break;
		        }
		        else if(buf[1]==0x7E){
                    state=1;
                    break;
                }
                state=2;
                a_ua = buf[0];
                break;
            }
            case 2: {
                if(buf[0]!=0x03){
                    state = 0;
                    break;
                }
                else if(buf[0]==0x7E){
                    state=1;
                    break;
                }
                state=3;
                c_ua = buf[0];
                break;
            }   
            case 3: {
                if(buf[0]!=a_ua^c_ua){
                    state=0;
                    break;
                }
                else if(buf[0]==0x7E){
                    state=1;
                    break;
                }
                state = 4;
                break;
            }   
            case 4:{
                if(buf[0]!=0x7E){
                    state=0;
                    break;
                }
                state = 5;
            }
            case 5:{
                state = 0;
                break;

                printf(":%s:%d\n", buf, bytes);
            }        

            }
    }
    else if(connectionParameters.role == LlRx){

    }
    else
        return -1;

    return 1;
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
