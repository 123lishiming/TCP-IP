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

