# Debug

## 1.结构体对齐问题

```c
typedef struct _ether_hdr_t {
	uint8_t dest[ETHER_HWA_SIZE];
	uint8_t src[ETHER_HWA_SIZE];
	uint16_t protocal; //可能当作四字节处理，结构体对齐问题(此时应该禁用自动填充对齐才能解析正确)
	uint32_t aa;
}ether_hdr_t;
```
<h2 style="color: red;">解决方法</h2>

```c
#pragma pack(1)
// 定义包头
typedef struct _ether_hdr_t {
	uint8_t dest[ETHER_HWA_SIZE];
	uint8_t src[ETHER_HWA_SIZE];
	uint16_t protocal; // 网络字节序的以太类型/协议
	uint32_t aa;
} ether_hdr_t;

// 定义数据包类型
typedef struct _ether_pkt_t {
	ether_hdr_t hdr;
	uint8_t data[ETHER_MIU];
} ether_pkt_t;
#pragma pack()
```

<h2 style="color: #0070c0;">2. 大小端转换问题</h2>

在处理多字节整数（如 16/32/64 位）时，内存中有两种常见的存储方式：

**小端存储（Little Endian）**
> 低字节在前，高字节在后。例如：
```text
0x78564312
```
内存排列：12 43 56 78

**大端存储（Big Endian）**
> 高字节在前，低字节在后。例如：
```text
0x12345678
```
内存排列：12 34 56 78

<h2 style="color: #ff6b35;">3. 32位字节序转换错误分析</h2>

基于你提供的调试结果，我们来分析为什么会出现这些错误的输出：

**问题代码：**
```c
static inline uint32_t swap_u32(uint16_t v){  // ❌ 参数类型错误！应该是 uint32_t
    uint32_t r = 
    (((v >> 0) & 0xFF) << 24)
    | (((v >> 8) & 0xFF) << 16)
    | (((v >> 16) & 0xFF) << 8)   // ❌ uint16_t 无法右移16位以上
    | (((v >> 24) & 0xFF) << 0);  // ❌ uint16_t 无法右移24位
    return r;
}
```

**调试输出分析：**
```text
(((v >> 0) & 0xFF) << 24) = 0x78000000   ✓ 正确
(((v >> 8) & 0xFF) << 16) = 0x00560000   ✓ 正确
(((v >> 16) & 0xFF) << 8) = 0x00000000   ❌ 错误！应该是非零值
(((v >> 24) & 0xFF) << 0) = 0x00000000   ❌ 错误！应该是非零值
```

**问题原因：**
1. 函数参数声明为 `uint16_t`，但实际需要处理32位数据
2. 当对16位数据进行 `>> 16` 和 `>> 24` 右移时，结果总是0
3. 因为16位数据最多只有16位，右移16位以上就全部变成0了

**正确的实现应该是：**
```c
static inline uint32_t swap_u32(uint32_t v){  // ✓ 正确的参数类型
    uint32_t r = 
    (((v >> 0) & 0xFF) << 24)
    | (((v >> 8) & 0xFF) << 16)
    | (((v >> 16) & 0xFF) << 8)
    | (((v >> 24) & 0xFF) << 0);
    return r;
}
```

**测试用例验证：**
```c
uint32_t test_value = 0x12345678;
printf("原值: 0x%08X\n", test_value);
printf("转换后: 0x%08X\n", swap_u32(test_value));
// 期望输出: 0x78563412
```

## 4. 百度面试题：判断机器字节序

**题目：** 简述大小端的概念，设计一个小程序来判断当前机器的字节序

### 概念回顾
**大小端：** 在处理多字节整数（如 16/32/64 位）时，内存中有两种常见的存储方式：
- **小端（Little Endian）**：低字节存储在低地址
- **大端（Big Endian）**：高字节存储在低地址

### 解题思路
用一个指针去访问多字节数据的第一个字节：
- 如果第一个字节是**高位字节** → **大端**
- 如果第一个字节是**低位字节** → **小端**

### 实现代码

```c
#include <stdio.h>
#include <stdint.h>

// 方法1：联合体方式
int check_endian_union() {
    union {
        uint32_t value;
        uint8_t bytes[4];
    } test;
    
    test.value = 0x12345678;
    
    if (test.bytes[0] == 0x78) {
        return 0; // 小端
    } else if (test.bytes[0] == 0x12) {
        return 1; // 大端
    }
    return -1; // 未知
}

// 方法2：指针转换方式
int check_endian_pointer() {
    uint32_t value = 0x12345678;
    uint8_t *ptr = (uint8_t*)&value;
    
    if (*ptr == 0x78) {
        return 0; // 小端
    } else if (*ptr == 0x12) {
        return 1; // 大端
    }
    return -1; // 未知
}

int main() {
    printf("测试值: 0x12345678\n");
    
    int result1 = check_endian_union();
    int result2 = check_endian_pointer();
    
    printf("联合体方法: %s\n", result1 == 0 ? "小端" : result1 == 1 ? "大端" : "未知");
    printf("指针方法: %s\n", result2 == 0 ? "小端" : result2 == 1 ? "大端" : "未知");
    
    return 0;
}
```

### 输出示例
```text
测试值: 0x12345678
联合体方法: 小端
指针方法: 小端
```

### 原理解释
1. **0x12345678** 在内存中的存储：
   - **小端机器**：78 56 34 12（低字节在前）
   - **大端机器**：12 34 56 78（高字节在前）

2. **指针访问第一个字节**：
   - 小端：`*ptr = 0x78`（低字节）
   - 大端：`*ptr = 0x12`（高字节）


#define PKTBUF_BLK_SIZE  128 // 数据块大小如果数据块太小，那么数据包可能不是连续的