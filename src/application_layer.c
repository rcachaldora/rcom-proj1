
// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    strcpy(ll.serialPort,serialPort);
    ll.role = (strcmp(role, "tx") == 0) ? LlTx : LlRx;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;


    int fd = llopen(ll);
    
    
    if (ll.role == LlTx) {
        // Create some data to write
        printf("\n\n\n\n\n\n\n");
        const unsigned char *dataToWrite = "}Hello, world";
        int dataSize = strlen(dataToWrite) + 1; // +1 for the null terminator

        // Call llwrite and print the result

        int writeResult = llwrite(fd, dataToWrite, dataSize);
        printf("llwrite result: %d\n", writeResult);

        const unsigned char *dataToWrite2 = "}this }}is working";

        int dataSize2 = strlen(dataToWrite2) + 1; // +1 for the null terminator

        int writeResult2 = llwrite(fd, dataToWrite2, dataSize2);

        printf("llwrite result2: %d\n", writeResult);

    } else if (ll.role == LlRx) {

        printf("\n\n\n\n\n\n\n");

        // Create a buffer to read data into
        unsigned char readBuffer[1024];

        // Call llread and print the result
        int readResult = llread(fd, readBuffer);
        printf("llread result: %d\n", readResult);

        // If the read was successful, print the data
        if (readResult >= 0) {
            printf("Read data: %s\n", readBuffer);
        }

        int readResult2 = llread(fd, readBuffer);
        printf("llread result2: %d\n", readResult);

        // If the read was successful, print the data
        if (readResult >= 0) {
            printf("Read data2: %s\n", readBuffer);
        }
    }
    
}
