// Link layer protocol implementation

#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

int alarmEnabled = FALSE;
int retransmitions = 0;
int alarmCount = 0;
int timeout = 0;
unsigned char frameTx = 0;
unsigned char frameRx = 0;
LinkLayerRole ROLE; 


////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    int fd = openPort(connectionParameters.serialPort, connectionParameters.baudRate);
    if(fd < 0) return -1;
    
    timeout = connectionParameters.timeout;
    retransmitions =  connectionParameters.nRetransmissions;
    ROLE = connectionParameters.role;
    
    printf("picking role\n");
    if(ROLE == LlTx)
        llopenTx(fd);
    else if(ROLE == LlRx)
        llopenRx(fd);
    else
        return -1;
    printf("successful llopen\n");

    retransmitions =  connectionParameters.nRetransmissions;
    return fd;
}

int llopenTx(int fd){
    //printf("entrou llopentx\n");
    (void) signal(SIGALRM, alarmHandler); // set the alarm to keep track of retransmissions and timeouts

    unsigned char SUPFRAME[SUPFRAME_SIZE] = {FLAG, A_SET, C_SET, A_SET^C_SET, FLAG};

    
    int state = 0;
    int a_ua_check = 0; //to store the a_ua and check the bcc1
    int c_ua_check = 0; //to store the c_ua and check the bcc1

    while(alarmCount < retransmitions && state != 5 ){ 
        int bytes = write(fd, SUPFRAME, SUPFRAME_SIZE);
        printf("%d bytes for SET\n", bytes);

        alarm(timeout);
        alarmEnabled = TRUE;

        unsigned char byte;
        while(alarmEnabled && state != 5){
            //bytes = read(fd, &byte, 1);  //read the frame sent by receiver
            /*printf("0x%02X\n", byte[0]);
            printf("0x%02X\n", byte[1]);
            printf("0x%02X\n", byte[2]);
            printf("0x%02X\n", byte[3]);
            printf("0x%02X\n", byte[4]);*/
            
            /*if(bytes == 0){
                continue;
            }  */          
            if (read(fd, &byte, 1) > 0 ){
                printf("current 0x%02X, state = %d\n",byte, state);
                
                switch (state){

                    case 0 :{
                        if(byte!=FLAG){
                            state=0;
                            printf("nao e a flag tx\n");
                            break;
                        }
                        
                        state = 1;
                        break;
                    }
                    case 1: {
                        if(byte!=A_UA){
                            state=0;
                            printf("0x%02X\n",byte);
                            break;
                        }
                        else if(byte==FLAG){
                            state=1;
                            break;
                        }
                        printf("0x%02X\n",byte);
                        state=2;
                        a_ua_check = byte;
                        //printf("a_ua_check = 0x%02X\n",a_ua_check);
                        break;
                    }
                    case 2: {
                        if(byte!=C_UA){
                            state = 0;
                            break;
                        }
                        else if(byte==FLAG){
                            state=1;
                            break;
                        }
                        printf("0x%02X\n",byte);
                        state=3;
                        c_ua_check = byte;
                        //printf("c_ua_check = 0x%02X\n",c_ua_check);
                        break;
                    }   
                    case 3: {
                        if(byte!=(a_ua_check^c_ua_check)){
                            state=0;
                            break;
                        }
                        else if(byte==FLAG){
                            state=1;
                            break;
                        }
                        printf("0x%02X\n",byte);
                        state = 4;
                        break;
                    }   
                    case 4:{
                        if(byte!=FLAG){
                            state=0;
                            break;
                        }
                        printf("0x%02X\n",byte);
                        state = 5;
                        break;
                    }
                    default:{
                        state = -1;
                        break;
                    }
                }
            }
        }
    }
    return fd;
}

int llopenRx(int fd){
    //printf("entrou llopenrx\n");


    unsigned char SUPFRAME;


    int state=0; //1-FLAG_RCV, 2-A_RCV, 3-C_RCV, 4-BCC_OK, 5-STOP

    int a_set = 0;
    int c_set = 0;

    while(state != -1 && state != 5){ 

        if(read(fd, &SUPFRAME, 1) > 0) {
        
            switch (state){
                
                case 0 :{
                    if(SUPFRAME!=FLAG){
                        state=0;
                        printf("0x%02X, state = %d\n",SUPFRAME, state);

                        printf("nao e a flag rx\n");
                        state = -1; //tirar depois
                        break;
                    }
                    printf("0x%02X, state = %d\n",SUPFRAME, state);

                    state = 1;
                    break;
                }
                case 1: {
                    if(SUPFRAME!=A_SET){
                        state=0;
                        printf("0x%02X, state = %d\n",SUPFRAME, state);

                        break;
                    }
                    else if(SUPFRAME==FLAG){
                        state=1;
                        printf("0x%02X, state = %d\n",SUPFRAME, state);

                        break;
                    }
                    printf("0x%02X, state = %d\n",SUPFRAME, state);

                    state = 2;
                    a_set = SUPFRAME;
                    break;
                }
                case 2: {
                    if(SUPFRAME!=C_SET){
                        state = 0;
                        break;
                    }
                    else if(SUPFRAME==FLAG){
                        state=1;
                        break;
                    }
                    printf("0x%02X, state = %d\n",SUPFRAME, state);

                    state = 3;
                    c_set = SUPFRAME;
                    break;
                }   
                case 3: {
                    if(SUPFRAME!=(a_set^c_set)){
                        state=0;
                        break;
                    }
                    else if(SUPFRAME==FLAG){
                        state=1;
                        break;
                    }
                    printf("0x%02X, state = %d\n",SUPFRAME, state);

                    state = 4;
                    break;
                }   
                case 4:{
                    if(SUPFRAME!=FLAG){
                        state=0;
                        break;
                    }
                    printf("0x%02X, state = %d\n",SUPFRAME, state);

                    state = 5;
                    break;
                }
                case 5:{
                    state = -1;
                    break;
                }        
            }
        }
    }
    unsigned char TxSUPFRAME[SUPFRAME_SIZE] = {FLAG, A_UA, C_UA, A_UA^C_UA, FLAG};

    /*
    printf("0x%02X\n",TxSUPFRAME[0]);
    printf("0x%02X\n",TxSUPFRAME[1]);
    printf("0x%02X\n",TxSUPFRAME[2]);
    printf("0x%02X\n",TxSUPFRAME[3]);
    printf("0x%02X\n",TxSUPFRAME[4]);
    */
    int bytes = write(fd, TxSUPFRAME, SUPFRAME_SIZE);

    printf("%d bytes written\n", bytes);

    return fd;
}


int openPort(const char* serialPort, const int BAUDRATE){
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
        return -1;
    }
    //printf("%d fd\n", fd);

    return fd;
}

void alarmHandler(int signal) {
    alarmEnabled = FALSE;
    alarmCount++;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(int fd,const unsigned char *buf, int bufSize)
{
    unsigned char header[4] = {FLAG, A_SET, C_N(frameTx), (header[1]^header[2])};
    unsigned char *dataBuf = (unsigned char*) malloc(bufSize);
    unsigned char BCC2 = buf[0];
    unsigned char *trailer = (unsigned char*)malloc(2); 


  //  printf("bcc2 = 0x%02X\n",BCC2);
    //bcc2
    for (int i = 1; i<bufSize; i++){
        BCC2 = BCC2 ^ buf[i];
    }
    printf("bcc2 = 0x%02X\n",BCC2);

    //byte stuffing bcc2
    int trailerSize = 2;
    if(BCC2 == 0x7E || BCC2 == 0x7D){
        trailer = (unsigned char*)realloc(trailer, 3);
        trailer[0] = 0x7D;
        trailer[1] = BCC2 ^ 0x20;
        trailer[2] = FLAG;
        trailerSize++;
     //   printf("byte stuffed bcc2\n");
    }
    else{
        trailer[0] = BCC2;
        trailer[1] = FLAG;
     //   printf("bcc2 not stuffed\n");
    }

   // printf("byte stuffing data payload\n");

    //byte stuffing data payload
    int currentSize = bufSize;
    for (int i = 0, k = 0; i<bufSize; i++,k++){
        if(buf[i] == 0x7E || buf[i] == 0x7D){
            currentSize++; //when we byte stuff the data we need to allocate one more byte
            dataBuf = (unsigned char*)realloc(dataBuf, currentSize);
            dataBuf[k] = 0x7D;
            dataBuf[k+1] = buf[i] ^0x20;
            k++;
           // printf("byte stuffed data\n");
           // printf("0x%02X\n",buf[i]);
           // printf("0x%02X\n",dataBuf[k]);
           // printf("0x%02X\n",dataBuf[k+1]);
        }
        else{
            dataBuf[k] = buf[i];
            //printf("data not stuffed\n");
            //printf("0x%02X\n",dataBuf[k]);
        }        
    }

    int infoframeSize = currentSize + 4 + trailerSize;
    unsigned char *infoframe[infoframeSize];

    int j = 0;
    printf("HEADER: \n");

    //header
    for(int i=0; i<4; i++,j++){
        infoframe[j] = header[i];
        printf("0x%02X ",infoframe[j]);
    }
    printf("\n---------------\nDATA with size %d:\n", currentSize);
    //data
    for(int k=0; k<currentSize; k++,j++){
        infoframe[j] = dataBuf[k];
        printf("0x%02X ",infoframe[j]);
    }

    
    printf("\n---------------\nTRAILER: ");
    //trailer
    for(int l=0; l<trailerSize; l++,j++){
        infoframe[j] = trailer[l];
        printf("0x%02X ",infoframe[j]);
    }

    FrameStatus status = NONE;
    int state=0; //1-FLAG_RCV, 2-A_RCV, 3-C_RCV, 4-BCC_OK, 5-STOP
    alarmCount = 0;
   
    printf("\n---------------\nstarting to write\n");
    while(alarmCount < retransmitions && state != 5 ){
        alarm(timeout);
        alarmEnabled = TRUE;
        
        int bytes = write(fd, header, 4);
        bytes = write(fd, dataBuf, currentSize);     
        bytes = write(fd, trailer, trailerSize);     
        
        /* for (int i = 0; i < infoframeSize; i++){
             printf("0x%02X ",infoframe[i]);
        
        }*/
        

        //printf("%d bytes written\n", infoframeSize);

        int a_set = 0;
        int c_set = 0;

        unsigned char SUPFRAME;
        state = 0;
        //printf("entering while to read\n");
        while(alarmEnabled && state != 5){
            //printf("entered while to read\n");
            bytes = read(fd, &SUPFRAME, 1);
            //printf("after read statement\n");
            if(bytes == 0) continue;

            printf("0x%02X, state = %d\n",SUPFRAME, state);
            
            switch (state){
                case 0 :{
                    if(SUPFRAME!=FLAG){
                        state=0;
                        break;
                    }

                    state = 1;
                    break;
                }
                case 1: {
                    if(SUPFRAME!=A_SET){
                        state=0;
                        break;
                    }
                    else if(SUPFRAME==FLAG){
                        state=1;
                        break;
                    }

                    state = 2;
                    a_set = SUPFRAME;
                     break;
                }
                case 2: {
                    if(SUPFRAME==C_RR0 || SUPFRAME==C_RR1 || SUPFRAME==C_REJ0 || SUPFRAME==C_REJ1 || SUPFRAME==C_DISC){
                        state = 3;
                        c_set = SUPFRAME;
                        break;
                    }
                    else if(SUPFRAME==FLAG){
                        state=1;
                        break;
                    }
                    else{
                        state = 0;
                    }
                     break;

                }   
                case 3: {
                    if(SUPFRAME!=(a_set^c_set)){
                        state=0;
                        break;
                    }
                    else if(SUPFRAME==FLAG){
                        state=1;
                        break;
                    }
                    state = 4;
                     break;
                }   
                case 4:{
                    if(SUPFRAME!=FLAG){
                        state=0;
                        break;
                    }

                    state = 5;
                     break;
                }
            }
        }
        if (state == 5) {
            printf("yayyy!\n");
        }
        if(c_set==C_RR0 || c_set==C_RR1){
            status = C_RR;
            printf("status = C_RR\n");
        }
        else if(c_set==C_REJ0 || c_set==C_REJ1){
            status = C_REJ;
            printf("status = C_REJ\n");
        }
        else{
            continue;
            printf("status = NONE\n");
        }    
        if(status == C_RR) break;

    }

    //printf("starting to free\n");
    //free(infoframe);
    //printf("infoframe\n");
    free(dataBuf);
    
    free(trailer);
    //free(header);
    
    if(status==C_RR){
        return infoframeSize;
    }
    else{
        llclose(fd);
        return -1;
    }

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(int fd, unsigned char *packet){
    unsigned char buf = 0;
    int state=0;
    int i = 0;
    int bufSize = 0;
    int a_set = 0;
    int c_set = 0;
    int acum=0;
    int bcc2=0;

    printf("starting to llread\n");
    while(state != 5){
        if(read(fd, &buf, 1) > 0){
            printf("byte = 0x%02X, state = %d\n",buf, state);
            bufSize++;
            switch(state){
                case 0:{
                    if(buf!=FLAG){
                        state = 0;
                        break;
                    }
                    state = 1;
                    break;
                }
                case 1:{
                    
                    if(buf!=A_SET){
                        state=0;
                    
                        break;
                    }
                    else if(buf==FLAG){
                        state=1;

                        break;
                    }
                    state = 2;
                    a_set = buf;
                    break;
                }
                case 2:{
                    
                    if(buf==C_N(0) || buf==C_N(1)){
                        state=3;
                        break;
                    }
                    else if(buf==FLAG){
                        state=1;
                        break;
                    }
                    else if(buf==C_DISC){
                        unsigned char FRAME[5] = {FLAG, A_SET, C_DISC, A_SET^C_DISC, FLAG};
                        return write(fd, FRAME, 5);
                        return 0;
                    }
                    else{
                        state=0;
                        break;
                    }
                    break;
                }
                case 3:{
                    if(buf!=(a_set^c_set)){
                        state=4; 
                        break;
                    }
                    else if(buf==FLAG){
                        state=1;
                        break;
                    }
                    state = 4;
                    break;
                }
                case 4:{
                    if(buf==ESC){
                        if(i == bufSize){
                            return -1;
                        }
                        if(read(fd, &buf, 1) > 0) { // read the next character into buf
                            if(buf == 0x5e){
                                packet[i++] = FLAG;
                            }
                            else if(buf == 0x5d){
                                packet[i++] = ESC;
                            }
                            else{
                                return -1;
                            }
                        break;
                        }
                    }
                    if(buf==FLAG){
                        bcc2 = packet[i-1];
                        i--;
                        packet[i] = '\0';
                        printf("My packet is : \n");
                        for (int x = 0; x < i; x++) {
                            printf("0x%02x ", packet[x]);
                        }
                        for (int j = 0; j<i;j++){
                            acum ^= packet[j];
                        }

                        if(bcc2 == acum){
                            printf("\neverything is correct!\n");
                            state = 5;
                            frameRx=(frameRx+1)%2;
                            unsigned char FRAME[SUPFRAME_SIZE] = {FLAG, A_SET, C_RR(frameRx), A_SET^C_RR(frameRx), FLAG};
                            int bytes = write(fd, FRAME, SUPFRAME_SIZE);
                            return i;
                        }
                        else{
                            printf("The calculated value was: 0x%02X\n", acum);
                            printf("bcc2 incorrect\n");
                            unsigned char FRAME[SUPFRAME_SIZE] = {FLAG, A_SET, C_REJ(frameRx), A_SET^C_REJ(frameRx), FLAG};
                            int bytes = write(fd, FRAME, SUPFRAME_SIZE);
                            return -1;
                        }
                    }
                    else{
                        packet[i] = buf;
                        i++;
                    }
                    break;
                }
            }
        }
        
    }

    return -1;
}


////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int fd)
{
    if(ROLE == LlTx)
        llcloseTx(fd);
    else if(ROLE == LlRx)
        llcloseRx(fd);
    else
        return -1;

    return 1;
}

int llcloseTx(int fd)
{
    unsigned char DISCFRAME[SUPFRAME_SIZE] = {FLAG,A_SET,C_DISC,A_SET^C_DISC,FLAG};

    int bytes = write(fd, DISCFRAME, SUPFRAME_SIZE);
    printf("%d bytes written\n", bytes);

    (void) signal(SIGALRM, alarmHandler); // set the alarm to keep track of retransmissions and timeouts
    alarm(timeout);
    alarmEnabled = FALSE;
    int state;

    while(retransmitions > 0 && state != -1 ){ 

        bytes = read(fd, DISCFRAME, 1);  //read the frame sent by receiver 

        //we should check if fd is not equal to 0

        state = 0; //setting the starting state
        int a_ua_check = 0; //to store the a_ua and check the bcc1
        int c_disc_check = 0; //to store the c_ua and check the bcc1

        switch (state){

            case 0 :{
                if(DISCFRAME[0]!=FLAG){
                    state=0;
                    break;
                }
                printf("0x%02X\n",DISCFRAME[0]);
                state = 1;
            }
            case 1: {
                if(DISCFRAME[0]!=A_UA){
                    state=0;
                    break;
                }
                else if(DISCFRAME[1]==FLAG){
                    state=1;
                    break;
                }
                printf("0x%02X\n",DISCFRAME[0]);
                state=2;
                a_ua_check = DISCFRAME[0];
            }
            case 2: {
                if(DISCFRAME[0]!=C_DISC){
                    state = 0;
                    break;
                }
                else if(DISCFRAME[0]==FLAG){
                    state=1;
                    break;
                }
                printf("0x%02X\n",DISCFRAME[0]);
                state=3;
                c_disc_check = DISCFRAME[0];
            }   
            case 3: {
                if(DISCFRAME[0]!=(a_ua_check^c_disc_check)){
                    state=0;
                    break;
                }
                else if(DISCFRAME[0]==FLAG){
                    state=1;
                    break;
                }
                printf("0x%02X\n",DISCFRAME[0]);
                state = 4;
            }   
            case 4:{
                if(DISCFRAME[0]!=FLAG){
                    state=0;
                    break;
                }
                printf("0x%02X\n",DISCFRAME[0]);
                state = 5;
            }
            case 5:{
                state = 0;
                printf("0x%02X\n",DISCFRAME[0]);
                break;
            }
        }

        unsigned char UA_FRAME[SUPFRAME_SIZE] = {FLAG,A_UA, C_UA, A_UA^C_UA, FLAG};

        bytes = write(fd, UA_FRAME, SUPFRAME_SIZE);
 
        retransmitions--;
    }
    return fd;
}

int llcloseRx(int fd)
{
    unsigned char DISCFRAME[SUPFRAME_SIZE] = {0};

    int bytes = read(fd, DISCFRAME, 1);

    int state=0; //1-FLAG_RCV, 2-A_RCV, 3-C_RCV, 4-BCC_OK, 5-STOP

    int a_set = 0;
    int c_set = 0;
    int a_ua_check = 0;
    int c_ua_check = 0;

    while(retransmitions > 0 && state != -1 ){ 

        switch (state){
                case 0 :{
                    if(DISCFRAME[0]!=FLAG){
                        state=0;
                        break;
                    }
                    printf("0x%02X\n",DISCFRAME[0]);
                    state = 1;
                }
                case 1: {
                    if(DISCFRAME[0]!=A_SET){
                        state=0;
                        break;
                    }
                    else if(DISCFRAME[0]==FLAG){
                        state=1;
                        break;
                    }
                    printf("0x%02X\n",DISCFRAME[0]);
                    state = 2;
                    a_set = DISCFRAME[0];
                }
                case 2: {
                    if(DISCFRAME[0]!=C_SET){
                        state = 0;
                        break;
                    }
                    else if(DISCFRAME[0]==FLAG){
                        state=1;
                        break;
                    }
                    printf("0x%02X\n",DISCFRAME[0]);
                    state = 3;
                    c_set = DISCFRAME[0];
                }   
                case 3: {
                    if(DISCFRAME[0]!=(a_set^c_set)){
                        state=0;
                        break;
                    }
                    else if(DISCFRAME[0]==FLAG){
                        state=1;
                        break;
                    }
                    printf("0x%02X\n",DISCFRAME[0]);
                    state = 4;
                }   
                case 4:{
                    if(DISCFRAME[0]!=FLAG){
                        state=0;
                        break;
                    }
                    printf("0x%02X\n",DISCFRAME[0]);
                    state = 5;
                }
                case 5:{
                    state = 0;
                    break;
                }        

        }

        unsigned char DISCFRAME1[SUPFRAME_SIZE] = {FLAG, A_UA, C_DISC, A_UA^C_DISC, FLAG};

        bytes = write(fd, DISCFRAME1, SUPFRAME_SIZE);

        printf("%d bytes written\n", bytes);

        unsigned char UA_FRAME[SUPFRAME_SIZE] = {0};

        bytes = read(fd, UA_FRAME, 1);

        switch (state){
                case 0 :{
                    if(UA_FRAME[0]!=FLAG){
                        state=0;
                        break;
                    }
                    printf("0x%02X\n",UA_FRAME[0]);
                    state = 1;
                }
                case 1: {
                    if(UA_FRAME[0]!=A_UA){
                        state=0;
                        break;
                    }
                    else if(UA_FRAME[0]==FLAG){
                        state=1;
                        break;
                    }
                    printf("0x%02X\n",UA_FRAME[0]);
                    state = 2;
                    a_ua_check = UA_FRAME[0];
                }
                case 2: {
                    if(UA_FRAME[0]!=C_UA){
                        state = 0;
                        break;
                    }
                    else if(UA_FRAME[0]==FLAG){
                        state=1;
                        break;
                    }
                    printf("0x%02X\n",UA_FRAME[0]);
                    state = 3;
                    c_ua_check = UA_FRAME[0];
                }   
                case 3: {
                    if(UA_FRAME[0]!=(a_set^c_set)){
                        state=0;
                        break;
                    }
                    else if(UA_FRAME[0]==FLAG){
                        state=1;
                        break;
                    }
                    printf("0x%02X\n",UA_FRAME[0]);
                    state = 4;
                }   
                case 4:{
                    if(UA_FRAME[0]!=FLAG){
                        state=0;
                        break;
                    }
                    printf("0x%02X\n",UA_FRAME[0]);
                    state = 5;
                }
                case 5:{
                    state = 0;
                    break;
                }        
        }
    }
    return fd;
}
