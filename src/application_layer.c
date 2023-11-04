// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer ll;
    strcpy(ll.serialPort,serialPort);
    ll.role = strcmp(role, "tx") ? LlRx : LlTx;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;


    llopen(ll);
}
