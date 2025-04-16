#include <string.h>
#include <winsock2.h>
#include "tcp_echo_server.h"
#include "sys_plat.h"
void tcp_echo_server_start (int port)
{
    plat_printf("tcp echo server,port: %d\n", port);
    //主要是windos
    //创建一个tcp套接字
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); // 初始化Winsock库

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        plat_printf("tcp echo server: open socket error");
        goto end;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); // 取端口号，注意大小端转换
    server_addr.sin_addr.s_addr = INADDR_ANY; // 绑定到所有可用的接口

    if(bind(s, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        plat_printf("bind error");
        goto end;
    }
    listen(s, 5); // 监听端口，最大连接数为5
    // 等待客户端连接
    while(1){
        //监听套接字
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(s, (struct sockaddr*)&client_addr, &addr_len);
        if(client_socket < 0) {
            plat_printf("accept error");
            break;
        }
        plat_printf("tcp echo server: client ip: %s, port: %d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        char buf[256];
        int size; // 修改为 int 类型以匹配 recv 的返回值
        while ((size = recv(client_socket, buf, sizeof(buf), 0)) > 0) { // 接收数据并获取长度
            buf[size] = '\0'; // 确保字符串以 '\0' 结尾
            plat_printf("tcp echo server: recv data: %s\n", buf);
            // 发送回显数据
            send(client_socket, buf, size, 0); // 使用接收到的实际长度发送数据
        }
        closesocket(client_socket); // 关闭客户端套接字
    }

end:
    if(s >= 0) {
        closesocket(s); // 关闭服务器套接字
    }
}