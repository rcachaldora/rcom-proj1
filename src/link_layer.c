// Link layer protocol implementation

#include "../include/link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd = openPort(connectionParameters.serialPort);
    
    if(connectionParameters.role == LlTx)
        return llopenTx(fd, connectionParameters.nRetransmissions);
    else if(connectionParameters.role == LlRx)
        return llopenRx(fd);
    else
        return -1;

    return fd;
}

int llopenTx(int fd, int retransmissionsCount, int timeout){

    unsigned char SUPFRAME[SUPFRAME_SIZE] = {FLAG, A_SET, C_SET, A_SET^C_SET, FLAG} // the supervision frame

    int bytes = write(fd, SUPFRAME, SUPFRAME_SIZE); // write the frame
    //printf("%d bytes written\n", bytes);

    (void) signal(SIGALRM, alarmHandler); // set the alarm to keep track of retransmissions and timeouts
    alarm(timeout);
    alarmTriggered = FALSE;
 
    while(retransmissionsCount > 0 && state != -1 ){ 

        bytes = read(fd, SUPFRAME, 1);  //read the frame sent by receiver 

        //we should check if fd is not equal to 0

        int state = 0; //setting the starting state
        int a_ua_check = 0; //to store the a_ua and check the bcc1
        int c_ua_check = 0; //to store the c_ua and check the bcc1

        switch (state){
        
            case 0 :{
                if(SUPFRAME[0]!=FLAG){  //flag 
                    state=0;
                    break;
                }
                state = 1;
            }
            case 1: {
                if(SUPFRAME[0]!=A_UA){ //a_ua
                    state=0;
                    break;
                }
                else if(SUPFRAME[0]==FLAG){//flag
                    state=1;
                    break;
                }
                state=2;
                a_ua_check = SUPFRAME[0];
            }
            case 2: {
                if(SUPFRAME[0]!=C_UA){//c_ua
                    state = 0;
                    break;
                }
                else if(SUPFRAME[0]==FLAG){//flag
                    state=1;
                    break;
                }
                state=3;
                c_ua_check = SUPFRAME[0];
            }   
            case 3: {
                if(SUPFRAME[0]!=a_ua_check^c_ua_check){ //bcc1
                    state=0;
                    break;
                }
                else if(SUPFRAME[0]==FLAG){ //flag
                    state=1;
                    break;
                }
                state = 4;
            }   
            case 4:{
                if(SUPFRAME[0]!=FLAG){ //flag again
                    state=0;
                    break;
                }
                state = 5;
            }
            case 5:{   //finished
                state = 0;
                break;
                //printf(":%s:%d\n", buf, bytes);
            }
        }

        retransmissionsCount--;
    }        

}

int llopenRx(int fd){

    int bytes = read(fd, SUPFRAME, 1); //aqui é bytes?

    int state=0; //1-FLAG_RCV, 2-A_RCV, 3-C_RCV, 4-BCC_OK, 5-STOP

    switch (state){
            case 0 :{
                if(SUPFRAME[0]!=FLAG){
                    state=0;
                    break;
                }
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
                state = 3;
                c_set = SUPFRAME[0];
            }   
            case 3: {
                if(SUPFRAME[0]!=a_set^c_set){
                    state=0;
                    break;
                }
                else if(SUPFRAME[0]==FLAG){
                    state=1;
                    break;
                }
                state = 4;
            }   
            case 4:{
                if(SUPFRAME[0]!=FLAG){
                    state=0;
                    break;
                }
                state = 5;
            }
            case 5:{
                state = 0;
                break;

                //printf(":%s:%d\n", buf, bytes);
            }        

        }

    unsigned char SUPFRAME[SUPFRAME_SIZE] = {FLAG, A_UA, C_UA, A_UA^C_UA, FLAG} //e se passarmos o supframe a uma variavel global?

    int bytes = write(fd, SUPFRAME, SUPFRAME_SIZE);
    //printf("%d bytes written\n", bytes);
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

void alarmHandler(int signal) {
    alarmTriggered = TRUE;
    alarmCount++;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    unsigned char header[4] = {FLAG, A_SET, (linkLayer.sequenceNumber == 0? C_I0 : C_I1), (header[1]^header[2])};
    unsigned char *dataBuf = (unsigned char*) malloc(bufSize);

    unsigned char BCC2 = buf[0];
    unsigned char *trailer = (unsigned char*)malloc(2); 

    //bcc2
    for (int i = 1; i<bufSize; i++){
        BCC2 = BCC2 ^ buffer[i];
    }

    //byte stuffing bcc2
    if(BCC2 == 0x7E || BCC2 == 0x7D){
        trailer = (unsigned char*)realloc(trailer, 3);
        trailer[0] = 0x7D;
        trailer[1] = BCC2 ^0x20;
        trailer[2] = FLAG;
    }
    else{
        trailer[0] = BCC2;
        trailer[1] = FLAG;
    }

    //byte stuffin data payload
    int currentSize = bufSize;
    for (int i = 1; i<bufSize; i++){
        if(buffer[i] == 0x7E || buffer[i] == 0x7D){
            currentSize++; //when we byte stuff the data we need to allocate one more byte
            trailer = (unsigned char*)realloc(trailer, currentSize);
            trailer[0] = 0x7D;
            trailer[1] = BCC2 ^0x20;
            trailer[2] = FLAG;
        }
        else{
            trailer[0] = BCC2;
            trailer[1] = FLAG;
        }        
    }

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
