#include <string.h>
#include "sys_plat.h"
#include <winsock2.h>
//#include <arpa/inet.h>

#include "tcp_echo_client.h"

int tcp_echo_client_start (const char* ip, int port) {
    plat_printf("tcp echo client, ip: %s, port: %d\n", ip, port);
    plat_printf("Enter quit to exit\n");
    //windows
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); // 初始化Winsock库

    // 创建套接字，使用流式传输，即tcp
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        plat_printf("tcp echo client: open socket error");
        goto end;
    }

    // 创建套接字，使用流式传输，即tcp
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);             // 取端口号，注意大小端转换
    server_addr.sin_addr.s_addr = inet_addr(ip);   // 设置服务器IP地址

    if (connect(s, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        plat_printf("connect error");
        goto end;
    }

    // 循环，读取一行后发出去
    char buf[256];
    plat_printf(">>");
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        // 将数据写到服务器中，不含结束符
        // 修复 C4267 警告，显式转换为 int
        if (send(s, buf, (int)(plat_strlen(buf) - 1), 0) <= 0) {
            plat_printf("write error");
            goto end;
        }

        // 读取回显结果并显示到屏幕上，不含结束符
        plat_memset(buf, 0, sizeof(buf));
        int len = recv(s, buf, sizeof(buf) - 1, 0);
        if (len <= 0) {
            plat_printf("read error");
            goto end;
        }
        buf[sizeof(buf) - 1] = '\0';        // 谨慎起见，写结束符

        // 显示回显结果
        printf("%s", buf);
        plat_printf("\n>>");
    }

end:
    // 关闭套接字
    if(s >= 0) {
        closesocket(s); // 关闭套接字
    }
    return 0;
}
