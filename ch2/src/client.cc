#include <iostream>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#define MAX_PAYLOAD 1024 /* maximum payload size*/

void checkRetAndExit(int ret, const char * msg){
    if(ret < 0){
        std::cout << msg << " " << errno << std::endl;
        exit(errno);
    }
}

int main(int argc, char * argv[]){

    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    int sock_fd;
    struct msghdr msg;
    memset((void *)&msg, 0, sizeof(msg));

    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_groups = 0;
    src_addr.nl_pid = 2048;
    socklen_t addr_size = sizeof(src_addr);
    int ret;
    sock_fd = socket(PF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_USERSOCK);
    checkRetAndExit(sock_fd, "open socket error ");

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 1024; /* server side unique id */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = 2048;
    nlh->nlmsg_flags = 0;

    strcpy((char *)NLMSG_DATA(nlh), "from client");

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    printf("Sending message to server\n");
    ret = sendmsg(sock_fd, &msg, 0);
    checkRetAndExit(ret, "sendmsg from socket ");

    printf("Waiting for message from server\n");

    memset(NLMSG_DATA(nlh), 0, MAX_PAYLOAD);
    /* Read message from server */
    ret = recvmsg(sock_fd, &msg, 0);
    checkRetAndExit(ret, "recvmsg from socket ");
    printf("Received message payload: %s\n", NLMSG_DATA(nlh));
    close(sock_fd);

    return 0;
}
