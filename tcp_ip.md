# 开发过程记录

## 项目概述
- **项目名称**: 网络协议栈开发
- **目标**: 实现一个基础的网络协议栈，包括链表管理、内存块管理、调试日志、TCP 回显服务器等功能。
- **技术栈**: C语言、CMake、PCAP、POSIX线程（Linux）/Win32线程（Windows）

---

## 项目目录结构
以下是项目的目录结构：
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

## 开发步骤

### 1. 项目初始化
- 创建项目目录结构。
- 配置 `CMakeLists.txt` 文件，支持跨平台构建。

#### **CMake 配置**
```cmake
# filepath: d:\start\CMakeLists.txt
# ...existing code...
```

---

### 2. 功能模块开发

#### 2.1 调试日志模块
- **文件**: `d:\start\src\net\net\dbg.h`
- **功能**:
  - 提供调试日志输出功能，支持不同级别（INFO、WARNING、ERROR）。
  - 支持断言功能 `dbg_assert`，在条件不满足时输出错误信息并停止程序。
- **关键代码**:
  ```c
  #define dbg_info(moudle, fmt, ...) \
      dbg_print(moudle, DBG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
  ```

- **效果图**:
  调试日志输出示例：
  ```
  [INFO] main.c:93: info
  [WARNING] main.c:94: warning
  [ERROR] main.c:95: error
  ```

---

#### 2.2 双向链表模块
- **文件**: `d:\start\src\net\net\nlist.h` 和 `nlist.c`
- **功能**:
  - 实现双向链表的基本操作，包括插入、删除、遍历等。
- **测试代码**:
  ```c
  void nlist_test() {
      nlist_t list;
      nlist_init(&list);
      // 插入、遍历、删除操作
  }
  ```

- **效果图**:
  双向链表插入和遍历示例：
  ```
  insert first
  id: 3
  id: 2
  id: 1
  id: 0
  ```

---

#### 2.3 内存块管理模块
- **文件**: `d:\start\src\net\net\mblock.h` 和 `mblock.c`
- **功能**:
  - 提供内存块的分配和释放功能，支持多线程安全。
- **测试代码**:
  ```c
  void mblock_test() {
      mblock_t blist;
      mblock_init(&blist, buffer, 100, 10, NLOCKER_THREAD);
      // 分配和释放内存块
  }
  ```

- **效果图**:
  内存块分配和释放示例：
  ```
  block :0x12345678, free_count: 9
  block :0x12345679, free_count: 8
  ...
  free_count: 10
  ```

---

#### 2.4 TCP 回显服务器
- **文件**: `d:\start\src\app\echo\tcp_echo_server.c`
- **功能**:
  - 实现一个简单的 TCP 回显服务器，接收客户端数据并回显。
- **关键代码**:
  ```c
  while ((size = recv(client_socket, buf, sizeof(buf), 0)) > 0) {
      send(client_socket, buf, size, 0);
  }
  ```

- **效果图**:
  使用 `telnet` 测试 TCP 回显服务器：
  ```
  Client: Hello, Server!
  Server: Hello, Server!
  ```

---

#### 2.5 平台适配层
- **文件**: `d:\start\src\plat\sys_plat.h` 和 `sys_plat.c`
- **功能**:
  - 提供跨平台的系统接口封装，包括线程、信号量、互斥锁等。

- **效果图**:
  平台适配层接口调用示例：
  ```
  创建线程: 成功
  创建信号量: 成功
  ```

---

### 3. 测试与验证

#### 3.1 基础功能测试
- **测试文件**: `d:\start\src\app\test\main.c`
- **测试内容**:
  - 调试日志模块测试。
  - 双向链表模块测试。
  - 内存块管理模块测试。

#### 3.2 网络功能测试
- **测试内容**:
  - 启动 TCP 回显服务器，使用客户端工具（如 `telnet`）进行测试。
  - 验证数据接收和回显功能是否正常。

---

### 4. 问题与解决

#### 4.1 调试日志宏参数错误
- **问题**: 宏定义中参数名不匹配，导致编译错误。
- **解决**: 修改宏定义，确保参数名一致。

#### 4.2 链表操作逻辑错误
- **问题**: 插入操作中 `node->next` 指向错误。
- **解决**: 修正逻辑，确保 `node->next` 指向链表的第一个节点。

#### 4.3 平台适配问题
- **问题**: Windows 和 Linux 平台的接口实现不一致。
- **解决**: 在 `sys_plat.c` 中分别实现不同平台的接口。

---

### 5. 未来改进方向
- 优化内存块管理模块，支持动态扩展。
- 增加更多网络协议支持，如 UDP 和 HTTP。
- 提供更完善的单元测试和集成测试。

---

### 6. 参考资料
- [CMake 官方文档](https://cmake.org/documentation/)
- [PCAP 库文档](https://www.tcpdump.org/pcap.html)
- [POSIX 线程编程](https://man7.org/linux/man-pages/man7/pthreads.7.html)