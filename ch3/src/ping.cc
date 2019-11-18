#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <sys/capability.h>
#include <sys/types.h>

#define PACK_SZ 2048
void checkRetAndExit(int ret, const char * msg){
    if(ret < 0){
        std::cout << msg << " " << errno << std::endl;
        exit(errno);
    }
}
int pingloop = 10;
void intHandler(int sig)
{
    pingloop=0;
}

u_short in_cksum(u_short * addr, int len){
    register int nleft = len;
    register u_short *w = addr;
    register u_short answer;
    register int sum = 0;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if( nleft == 1 ) {
        u_short	u = 0;

        *(u_char *)(&u) = *(u_char *)w ;
        sum += u;
    }

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
    sum += (sum >> 16);			/* add carry */
    answer = ~sum;				/* truncate to 16 bits */
    return (answer);
}

void ping_ip(struct in_addr * dstAddr){

    int sock = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    checkRetAndExit(sock, "Init ICMP socket failed ");
    struct sockaddr_in addr;
    int sequence = 0;

    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr = *dstAddr;

    while (pingloop--) {
        unsigned char data[PACK_SZ];
        memset(data, 0, PACK_SZ);
        struct icmp * imp = (struct icmp *)data;
        int rc;
        struct timeval timeout = {3, 0}; //wait max 3 seconds for a reply
        fd_set read_set;

        imp->icmp_type = ICMP_ECHO;
        imp->icmp_code = 0;
        imp->icmp_id = 0x1234;
        imp->icmp_seq = sequence++;
        memcpy(data + sizeof(struct icmp), "hello", 5); //icmp payload
        imp->icmp_cksum = in_cksum((u_short *)imp, sizeof(struct icmp) + 5);
        rc = sendto(sock, data, sizeof(struct icmp) + 5,
                    0, (struct sockaddr*)&addr, sizeof(addr));
        if (rc <= 0) {
            perror("Sendto");
            break;
        }
        puts("Sent ICMP");
        memset(&read_set, 0, sizeof(read_set));
        FD_SET(sock, &read_set);

        //wait for a reply with a timeout
        rc = select(sock + 1, &read_set, NULL, NULL, &timeout);
        if (rc == 0) {
            puts("Got no reply\n");
            continue;
        } else if (rc < 0) {
            perror("Select");
            break;
        }

        unsigned char recvData[PACK_SZ];
        memset(recvData, 0, PACK_SZ);
        struct icmp * recvPack = (struct icmp *)recvData;

        struct sockaddr_in r_addr;
        socklen_t slen = sizeof(r_addr);

        rc = recvfrom(sock, recvData, sizeof(recvData), 0, (sockaddr *)&r_addr, &slen);
        if (rc <= 0) {
            perror("recvfrom");
            break;
        } else if (rc < sizeof(struct icmp)) {
            printf("Error, got short ICMP packet, %d bytes\n", rc);
            break;
        }
        char * cAddr = inet_ntoa(r_addr.sin_addr);

        if (recvPack->icmp_type == ICMP_ECHOREPLY || (recvPack->icmp_type == 0x45 && recvPack->icmp_code == 0)) {
            printf("ICMP Reply from %s , id=0x%x, sequence =  0x%x\n", cAddr,
                   recvPack->icmp_id, recvPack->icmp_seq);
        } else {
            printf("Got ICMP packet from %s with type 0x%x and code %d\n", cAddr, recvPack->icmp_type, recvPack->icmp_code);
        }
    }
}

int main(int argc, char * argv[]) {
    int ret;
    cap_t state = cap_get_proc();
    cap_flag_value_t val;
    ret = cap_get_flag(state, CAP_NET_RAW, CAP_EFFECTIVE, &val);
    checkRetAndExit(ret, "Get cap ");
    std::cout << "Effective net_raw " << val << std::endl;
    ret = cap_get_flag(state, CAP_NET_RAW, CAP_PERMITTED, &val);
    checkRetAndExit(ret, "Get cap ");
    std::cout << "Permitted net_raw " << val << std::endl;
    cap_value_t  caps[2] = {CAP_NET_RAW, CAP_NET_ADMIN};
    ret = cap_set_flag(state, CAP_EFFECTIVE, 2, caps, val);
    checkRetAndExit(ret, "set cap flag value");
    ret = cap_set_proc(state);
    checkRetAndExit(ret, "set cap for proc");
    ret = cap_get_flag(state, CAP_NET_RAW, CAP_EFFECTIVE, &val);
    checkRetAndExit(ret, "Get cap ");
    std::cout << "Effective net_raw " << val << std::endl;
    ret = cap_get_flag(state, CAP_NET_RAW, CAP_PERMITTED, &val);
    checkRetAndExit(ret, "Get cap ");
    std::cout << "Permitted net_raw " << val << std::endl;


    if (argc != 2) {
        printf("usage: %s destination_ip\n", argv[0]);
        return 1;
    }
    struct in_addr dst;

    if (inet_aton(argv[1], &dst) == 0) {

        perror("inet_aton");
        printf("%s isn't a valid IP address\n", argv[1]);
        return 1;
    }
    signal(SIGINT, intHandler);

    ping_ip(&dst);
    return 0;
}