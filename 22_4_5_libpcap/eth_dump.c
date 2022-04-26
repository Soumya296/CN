// gcc eth_dump.c -o eth -lpcap -lbsd
#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>

int main(int argc, const char * argv[]) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = NULL;
    u_char packet[] = "\x01\x02\x03\x04\x05\x06\xf1\xf2\xf3\xf4\xf5\xf5\x86\x00";
    int length = sizeof(packet) - 1;

    handle = pcap_open_live("lo", 65535, 1, 1, errbuf);
    if(!handle) {
        fprintf(stderr, "pcap_open_live: %s\n", errbuf);
        exit(1);
    }//end if

    //send packet
    while(1){
    if(pcap_sendpacket(handle, packet, length) < 0) {
        fprintf(stderr, "pcap_sendpacket: %s\n", pcap_geterr(handle));
    }//end if
    }

    //free
    pcap_close(handle);
    return 0;
}
