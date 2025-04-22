# TCP/IP 协议栈开发项目

## 项目概述
本项目旨在实现一个基础的 TCP/IP 协议栈，包含链表管理、内存块管理、调试日志、数据包管理以及 TCP 回显服务器等功能。项目支持跨平台开发，适配 Windows 和 Linux 系统。

---

## 功能模块
### 1. 调试日志模块
- **文件**: `src/net/net/dbg.h`
- **功能**:
  - 提供调试日志输出功能，支持不同级别（INFO、WARNING、ERROR）。
  - 支持断言功能 `dbg_assert`，在条件不满足时输出错误信息并停止程序。
- **示例**:
  ```c
  dbg_info(DBG_TEST, "info");
  dbg_warning(DBG_TEST, "warning");
  dbg_error(DBG_TEST, "error");
  ```
- **效果图**:
  ![调试日志输出示例](docs/images/debug_log_example.png)

---

### 2. 双向链表模块
- **文件**: `src/net/net/nlist.h` 和 `src/net/src/nlist.c`
- **功能**:
  - 实现双向链表的基本操作，包括插入、删除、遍历等。
- **示例**:
  ```c
  nlist_t list;
  nlist_init(&list);
  nlist_insert_first(&list, &node);
  nlist_remove_first(&list);
  ```
- **效果图**:
  ![双向链表操作示例](docs/images/nlist_example.png)

---

### 3. 内存块管理模块
- **文件**: `src/net/net/mblock.h` 和 `src/net/src/mblock.c`
- **功能**:
  - 提供内存块的分配和释放功能，支持多线程安全。
- **示例**:
  ```c
  mblock_t blist;
  mblock_init(&blist, buffer, 100, 10, NLOCKER_THREAD);
  void *block = mblock_alloc(&blist, -1);
  mblock_free(&blist, block);
  ```
- **效果图**:
  ![内存块管理示例](docs/images/mblock_example.png)

---

### 4. 数据包管理模块
- **文件**: `src/net/net/pktbuf.h` 和 `src/net/src/pktbuf.c`
- **功能**:
  - 管理网络数据包的分配、释放、头部添加和移除等操作。
- **示例**:
  ```c
  pktbuf_t *buf = pktbuf_alloc(2000);
  pktbuf_add_header(buf, 100, 1);
  pktbuf_remove_header(buf, 50);
  pktbuf_free(buf);
  ```
- **效果图**:
  ![数据包管理示例](docs/images/pktbuf_example.png)

---

### 5. TCP 回显服务器
- **文件**: `src/app/echo/tcp_echo_server.c`
- **功能**:
  - 实现一个简单的 TCP 回显服务器，接收客户端数据并回显。
- **示例**:
  使用 `telnet` 测试：
  ```
  Client: Hello, Server!
  Server: Hello, Server!
  ```
- **效果图**:
  ![TCP 回显服务器示例](docs/images/tcp_echo_server_example.png)

---

### 6. 平台适配层
- **文件**: `src/plat/sys_plat.h` 和 `src/plat/sys_plat.c`
- **功能**:
  - 提供跨平台的系统接口封装，包括线程、信号量、互斥锁等。

---

## 项目目录结构
```
d:\start\
    ├── src\
    │   ├── app\
    │   │   ├── echo\              # TCP 回显服务器
    │   │   └── test\              # 测试代码
    │   ├── net\                   # 网络协议栈核心模块
    │   └── plat\                  # 平台适配层
    ├── npcap\                     # PCAP 库
    ├── .vscode\                   # VSCode 配置
    └── CMakeLists.txt             # CMake 配置文件
```

---

## 编译与运行
### 1. 环境要求
- **操作系统**: Windows 或 Linux
- **工具链**: GCC 或 MSVC
- **依赖**: CMake、PCAP 库

### 2. 编译步骤
1. 使用 CMake 生成构建文件：
   ```bash
   cmake -S . -B build
   ```
2. 进入构建目录并编译：
   ```bash
   cmake --build build
   ```

### 3. 运行测试
进入测试程序目录并运行：
```bash
cd build/src/app/test
./test_program
```

---

## 测试与验证
### 1. 基础功能测试
- 测试文件: `src/app/test/main.c`
- 测试内容:
  - 调试日志模块测试。
  - 双向链表模块测试。
  - 内存块管理模块测试。
  - 数据包管理模块测试。

### 2. 网络功能测试
- 启动 TCP 回显服务器，使用客户端工具（如 `telnet`）进行测试。
- 验证数据接收和回显功能是否正常。

---

## 问题与解决
### 1. 调试日志宏参数错误
- **问题**: 宏定义中参数名不匹配，导致编译错误。
- **解决**: 修改宏定义，确保参数名一致。

### 2. 链表操作逻辑错误
- **问题**: 插入操作中 `node->next` 指向错误。
- **解决**: 修正逻辑，确保 `node->next` 指向链表的第一个节点。

### 3. 平台适配问题
- **问题**: Windows 和 Linux 平台的接口实现不一致。
- **解决**: 在 `sys_plat.c` 中分别实现不同平台的接口。

---

## 未来改进方向
- 优化内存块管理模块，支持动态扩展。
- 增加更多网络协议支持，如 UDP 和 HTTP。
- 提供更完善的单元测试和集成测试。

---

## 参考资料
- [CMake 官方文档](https://cmake.org/documentation/)
- [PCAP 库文档](https://www.tcpdump.org/pcap.html)
- [POSIX 线程编程](https://man7.org/linux/man-pages/man7/pthreads.7.html)
