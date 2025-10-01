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


