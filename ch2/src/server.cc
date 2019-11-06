#include <iostream>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
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
    src_addr.nl_pid = 1024;
    socklen_t addr_size = sizeof(src_addr);
    int ret;
    sock_fd = socket(PF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_USERSOCK);
    checkRetAndExit(sock_fd, "open socket error ");

    ret = bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    checkRetAndExit(ret, "bind sock failed ");

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 2048; /* client unique id  */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = 1024;
    nlh->nlmsg_flags = 0;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    /* Read message from client */
    printf("Waiting for message from client\n");
    ret = recvmsg(sock_fd, &msg, MSG_TRUNC);
    checkRetAndExit(ret, "recvMsg socket error ");

    printf("Received message payload: %s\n", NLMSG_DATA(nlh));

    strcpy((char *)NLMSG_DATA(nlh), "from server");

    printf("Sending message to client\n");
    ret = sendmsg(sock_fd, &msg, 0);
    checkRetAndExit(ret, "sendMsg socket error ");

    close(sock_fd);

    return 0;



    return 0;
}