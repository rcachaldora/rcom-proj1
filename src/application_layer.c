// Application layer protocol implementation

#include "../include/application_layer.h"
#include "../include/link_layer.h"
#include <string.h>

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer connectionParameters;
    strcpy(connectionParameters.serialPort, serialPort);

    if (strcmp(role, "transmitter") == 0) {
            connectionParameters.role = LlTx;
        } else if (strcmp(role, "receiver") == 0) {
            connectionParameters.role = LlRx;
        }

    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;

    int fd = llopen(connectionParameters);


}
