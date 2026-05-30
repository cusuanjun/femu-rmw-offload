# F2FS 非页对齐写 RMW 卸载实验 -- 基础知识



## 目录



1. [背景：为什么需要 RMW 卸载](#1)

2. [F2FS 文件系统写路径分析](#2)

3. [FEMU SSD 仿真器架构](#3)

4. [NVMe 协议与命令体系](#4)

5. [RMW 卸载方案设计](#5)

6. [关键代码路径详解](#6)

7. [实验修改清单](#7)



---



## 1. 背景：为什么需要 RMW 卸载



### 1.1 F2FS 的写入粒度



F2FS 以 **4KB 页** 为最小管理单元。日志结构写入时，即使应用只写入 100 字节，F2FS 也必须以整页（4KB）为单位写入闪存。



### 1.2 非页对齐写的问题



当写入不满足以下条件时，触发 **Read-Modify-Write（RMW）**：



- 写入起始地址不是 4KB 对齐

- 写入长度不是 4KB 的整数倍

- 写入范围跨 4KB 边界



**传统 RMW 流程**（在主机侧完成）：



```

应用写入 512B（偏移 1024）

    |

    v

F2FS: 读取旧 4KB 页（SSD -> 主机内存）    [1. 读]

    |

    v

F2FS: 在内存中合并 512B 新数据到旧页      [2. 改]

    |

    v

F2FS: 将 4KB 完整页写回 SSD              [3. 写]

```



**问题**：

- 读回旧页浪费 SSD 带宽

- 主机内存拷贝浪费 CPU

- 写放大：4KB + 4KB = 8KB 传输，只为写入 512B



### 1.3 RMW 卸载方案



将 RMW 操作下沉到 **SSD 内部** 执行：



```

应用写入 512B（偏移 1024）

    |

    v

F2FS: 发送自定义 NVMe 命令（携带 512B + 偏移信息）

    |

    v

SSD 内部: 读旧页 -> 合并新数据 -> 写新页   [全在 SSD 内完成]

```



**优势**：

- 消除主机端读回开销

- 减少 PCIe 总线传输（只传 512B，不传 4KB 旧数据）

- 利用 SSD 内部带宽（远大于 PCIe）



---

## 2. F2FS 文件系统写路径分析



### 2.1 整体架构



```

用户空间

  | write()

  v

VFS 层

  |

  v

F2FS: f2fs_file_write_iter()       [文件写入口]

  |

  v

F2FS: f2fs_write_cache_pages()     [页缓存回写]

  |

  v

F2FS: f2fs_write_data_page()       [单页写入]

  |

  v

F2FS: f2fs_do_write_data_page()    [实际写操作]

  |

  +-- 分配新的 data block 地址

  +-- 构造 bio

  +-- submit_bio()                 [提交块层请求]

       |

       v

    块层 / NVMe 驱动

       |

       v

    SSD 设备

```



### 2.2 关键文件



| 文件 | 功能 |

|------|------|

| fs/f2fs/file.c | 文件操作入口，f2fs_file_write_iter() |

| fs/f2fs/data.c | 数据页读写，f2fs_write_data_page() 等 |

| fs/f2fs/f2fs.h | 核心数据结构（f2fs_sb_info 等） |

| fs/f2fs/segment.c | 段管理和 LFS 写入分配 |

| fs/f2fs/node.c | 节点（元数据）页面管理 |



### 2.3 RMW 触发点



在 f2fs_do_write_data_page() 中（fs/f2fs/data.c），当需要写入小于完整页时：



- 写部分页（非整页写入）

- 启用内联数据（inline data）

- 压缩写入 / 加密文件



标准 RMW：f2fs_readpage() -> 内存修改 -> f2fs_write_data_page()



**修改点**：不读回旧页，而是构造自定义 NVMe 命令直接发给 SSD。



---



## 3. FEMU SSD 仿真器架构



### 3.1 整体结构



```

Guest VM (Linux + F2FS)

  | NVMe 命令（通过 PCIe BAR / MMIO）

  v

+==========================================+

|  FEMU (QEMU 进程内)                      |

|                                          |

|  nvme-io.c      [IO 命令分发]             |

|    |                                     |

|    +-- NVME_CMD_READ  -> bbssd/bb.c      |

|    +-- NVME_CMD_WRITE -> bbssd/bb.c      |

|    +-- n->ext_ops.io_cmd()               |

|         |                                |

|         v                                |

|  bbssd/bb.c      [BBSSD IO 调度]         |

|    |                                     |

|    +-- ring buffer -> FTL 线程           |

|    |                                     |

|  bbssd/ftl.c     [FTL 闪存翻译层]         |

|    |                                     |

|    +-- 地址映射 (maptbl: LPN->PPA)       |

|    +-- 写入分配 (write pointer)           |

|    +-- 垃圾回收 (GC)                      |

|         |                                |

|         v                                |

|  backend/dram.c  [DRAM 存储后端]          |

+==========================================+

```



### 3.2 关键文件



| 文件 | 行数 | 功能 |

|------|------|------|

| hw/femu/nvme.h | 1498 | NVMe 数据结构（Ctrl, NS, Request, Cmd） |

| hw/femu/nvme-io.c | 560 | IO 命令 dispatch + CQ 完成处理 |

| hw/femu/nvme-admin.c | ~200 | Admin 命令 |

| hw/femu/nvme-util.c | ~100 | PRP/SGL 映射、地址读写 |

| hw/femu/femu.c | 692 | FEMU 设备初始化、PCIe 注册 |

| hw/femu/bbssd/bb.c | ~300 | BBSSD IO 调度 |

| hw/femu/bbssd/ftl.c | 921 | **FTL 核心**：读/写/GC |

| hw/femu/bbssd/ftl.h | 237 | FTL 数据结构 |

| hw/femu/backend/dram.c | ~50 | 内存读写后端 |



### 3.3 FTL 核心数据结构



```c

// ftl.h - 页级映射 FTL

struct ssd {

    struct ssdparams sp;         // SSD 参数

    struct ppa *maptbl;          // 映射表: LPN -> PPA

    uint64_t *rmap;              // 反向映射: PPA -> LPN

    struct write_pointer wp;     // 当前写指针

    struct line_mgmt lm;         // 块管理（free/victim/full）

    struct rte_ring **to_ftl;    // IO线程 -> FTL线程

    struct rte_ring **to_poller; // FTL线程 -> 完成轮询

};



// 物理页地址（通道:芯片:块:页:扇区）

struct ppa {

    uint64_t ch  : 7;   // 通道

    uint64_t lun : 8;   // LUN

    uint64_t pl  : 8;   // 平面

    uint64_t blk : 16;  // 块

    uint64_t pg  : 16;  // 页

    uint64_t sec : 8;   // 扇区

};

```



### 3.4 FTL 写流程



```c

// ftl.c - 简化写流程

int ssd_write(FemuCtrl *n, NvmeRequest *req) {

    uint64_t lpn = req->slba;

    // 1. 分配新物理页

    struct ppa ppa = get_new_page(n);

    // 2. 写入 NAND/DRAM

    nand_write(n, ppa, req->data, nsecs);

    // 3. 更新映射表

    struct ppa old_ppa = n->maptbl[lpn];

    n->maptbl[lpn] = ppa;

    // 4. 标记旧页无效

    if (old_ppa != INVALID_PPA)

        invalidate_page(n, old_ppa);

    return 0;

}

```



**RMW 修改思路**：在步骤2之前插入读旧页+合并逻辑



```c

if (req->is_rmw) {

    struct ppa old_ppa = ssd->maptbl[lpn];

    uint8_t *old_page = backend_read(n, old_ppa);

    memcpy(old_page + req->rmw_offset, req->new_data, req->rmw_len);

    nand_write(n, ppa, old_page, ssd->sp.secs_per_pg);

    free(old_page);

}

```



---

## 4. NVMe 协议与命令体系



### 4.1 NVMe 命令格式（64 字节）



```

Byte 0:   OPCODE        [命令类型, 0x00=Flush, 0x01=Write, 0x02=Read]

Byte 1:   FUSE | PSDT   [融合标志]

Byte 2-3: CID           [命令 ID]

Byte 4-7: NSID          [命名空间 ID]

Byte 8-23: Reserved

Byte 24-39: MPTR         [元数据指针]

Byte 40-47: PRP1         [物理区域页1 / 数据缓冲区]

Byte 48-55: PRP2         [物理区域页2]

Byte 56-63: SLBA         [起始 LBA]

Byte 64-67: NLB          [逻辑块数, 0-based]

Byte 68-71: CDW12        [命令专用字12]

Byte 72-75: CDW13

Byte 76-79: CDW14

Byte 80-83: CDW15

```



### 4.2 标准 NVMe Opcode



| Opcode | 命令 | 说明 |

|--------|------|------|

| 0x00 | Flush | 刷新缓存 |

| 0x01 | Write | 数据写入 |

| 0x02 | Read | 数据读取 |

| 0x04 | Write Uncorrectable | 标记不可纠正 |

| 0x05 | Compare | 数据比较 |

| 0x08 | Write Zeroes | 写零 |

| 0x09 | Dataset Management | Trim |

| **0xC0-0xFF** | **Vendor Specific** | **自定义命令（RMW 用此范围）** |



### 4.3 RMW 卸载命令设计



```c

#define NVME_CMD_RMW_OFFLOAD  0xC0



// RMW 命令的 NvmeCmd 布局：

// CDW0  (Byte 0): 0xC0        [RMW Opcode]

// SLBA  (Byte 56-63): LBA     [目标 LBA]

// NLB   (Byte 64-67): nsecs-1 [写入扇区数-1]

// CDW12 (Byte 68-71): offset  [页内偏移，字节]

// PRP1  (Byte 40-47): data    [新数据缓冲区]

```



### 4.4 NVMe 命令在 FEMU 中的处理



```c

// nvme-io.c - 命令分发

static uint16_t nvme_io_cmd(FemuCtrl *n, NvmeCmd *cmd, NvmeRequest *req)

{

    switch (cmd->opcode) {

    case NVME_CMD_FLUSH:       return nvme_flush(...);

    case NVME_CMD_DSM:         return nvme_dsm(...);

    case NVME_CMD_COMPARE:     return nvme_compare(...);

    case NVME_CMD_WRITE_ZEROES: return nvme_write_zeros(...);

    case NVME_CMD_WRITE_UNCOR: return nvme_write_uncor(...);

    default:

        if (n->ext_ops.io_cmd)

            return n->ext_ops.io_cmd(n, ns, cmd, req);

        return NVME_INVALID_OPCODE | NVME_DNR;

    }

}

```



---

## 5. RMW 卸载方案设计



### 5.1 端到端流程



```

Host (Guest VM)

  F2FS: f2fs_write_data_page()

    |

    +-- 检测：写入非 4KB 对齐？

    |   +-- 是 -> 构造 NVMe RMW-offload 命令

    |   +-- 否 -> 正常 Write 命令

    |

    +-- nvme_submit_rmw_cmd(ns, lba, data, len, offset)

         |

============== PCIe/NVMe ===============

         |

         v

  FEMU: nvme_io_cmd()

    |

    +-- case NVME_CMD_RMW_OFFLOAD:

         +-- ftl_rmw_write(n, ns, cmd, req)

              |

              +-- 读旧页 (SSD内部，不走PCIe)

              +-- 在偏移处合并新数据

              +-- 分配新物理页

              +-- 写入 NAND/DRAM

              +-- 更新 maptbl[LPN]

```



### 5.2 主机侧（F2FS）修改



| 文件 | 修改 |

|------|------|

| drivers/nvme/host/nvme.h | 添加 RMW opcode 和 helper 声明 |

| drivers/nvme/host/core.c | 实现 nvme_submit_rmw_cmd() |

| fs/f2fs/data.c | 非整页写路径调 RMW 命令替代读回旧页 |

| fs/f2fs/f2fs.h | 添加调试开关 |



### 5.3 设备侧（FEMU）修改



| 文件 | 修改 |

|------|------|

| hw/femu/nvme.h | 定义 NVME_CMD_RMW_OFFLOAD 和结构体 |

| hw/femu/nvme-io.c | nvme_io_cmd() 添加 RMW case |

| hw/femu/bbssd/ftl.c | 实现 ftl_rmw_write() 读-改-写 |

| hw/femu/bbssd/ftl.h | 声明 ftl_rmw_write() |

| hw/femu/bbssd/bb.c | 路由 RMW 请求到 FTL |



---



## 6. 关键代码路径详解



### 6.1 FEMU: nvme_io_cmd() -- IO 命令入口 (hw/femu/nvme-io.c:482)



```c

static uint16_t nvme_io_cmd(FemuCtrl *n, NvmeCmd *cmd, NvmeRequest *req)

{

    NvmeNamespace *ns;

    uint32_t nsid = le32_to_cpu(cmd->nsid);

    if (nsid == 0 || nsid > n->num_namespaces)

        return NVME_INVALID_NSID | NVME_DNR;

    req->ns = ns = &n->namespaces[nsid - 1];



    switch (cmd->opcode) {

    case NVME_CMD_FLUSH:

        return nvme_flush(n, ns, cmd, req);

    /* ... 其他标准命令 ... */



    /* == 你的修改：新增 RMW 命令 == */

    case NVME_CMD_RMW_OFFLOAD:

        return nvme_rmw_offload(n, ns, cmd, req);



    default:

        if (n->ext_ops.io_cmd)

            return n->ext_ops.io_cmd(n, ns, cmd, req);

        return NVME_INVALID_OPCODE | NVME_DNR;

    }

}

```



### 6.2 FEMU: FTL 写入 -- ssd_advance_write() (ftl.c ~700行)



```c

static bool ssd_advance_write(FemuCtrl *n, NvmeRequest *req)

{

    struct ssd *ssd = n->ssd;

    uint64_t lpn = req->slba;

    struct ppa ppa;

    // 1. 获取写位置

    ppa = get_next_ppa(n);

    // 2. 写入数据

    ssd_advance_status(n, req, &ppa, nsecs, NAND_WRITE);

    // 3. 更新映射

    struct ppa old_ppa = ssd->maptbl[lpn];

    ssd->maptbl[lpn] = ppa;

    ssd->rmap[ppa.ppa] = lpn;

    // 4. 失效旧页

    if (old_ppa.ppa != INVALID_PPA)

        invalidate_old_page(n, old_ppa);

    return true;

}

```



**RMW 插入逻辑**（在步骤2前）：



```c

if (req->is_rmw) {

    struct ppa old_ppa = ssd->maptbl[lpn];

    uint8_t *old_page = backend_read(n, old_ppa); // SSD内部读

    memcpy(old_page + req->rmw_offset, req->new_data, req->rmw_len);

    nand_write(n, ppa, old_page, ssd->sp.secs_per_pg);

    free(old_page);

}

```



### 6.3 Linux NVMe 驱动: RMW 命令提交 (drivers/nvme/host/core.c)



```c

int nvme_submit_rmw_cmd(struct nvme_ns *ns, sector_t lba,

                         void *data, unsigned int len,

                         unsigned int offset)

{

    struct nvme_command cmd = {};

    cmd.common.opcode = nvme_cmd_rmw_offload;

    cmd.rw.nsid = cpu_to_le32(ns->head->ns_id);

    cmd.rw.slba = cpu_to_le64(lba);

    cmd.rw.length = cpu_to_le16((len >> ns->lba_shift) - 1);

    cmd.rw.cdw12 = cpu_to_le32(offset);  // 页内偏移

    return __nvme_submit_sync_cmd(ns->queue, &cmd, NULL, data, len, ...);

}

```



---

## 7. 实验修改清单



### 第一阶段：FEMU 侧（SSD 仿真器）



| # | 文件 | 修改 |

|---|------|------|

| 1 | hw/femu/nvme.h | 添加 #define NVME_CMD_RMW_OFFLOAD 0xC0 |

| 2 | hw/femu/nvme.h | NvmeRequest 加 is_rmw, rmw_offset, rmw_len 字段 |

| 3 | hw/femu/nvme-io.c | switch 加 case NVME_CMD_RMW_OFFLOAD |

| 4 | hw/femu/nvme-io.c | 实现 nvme_rmw_offload() handler |

| 5 | hw/femu/bbssd/ftl.h | 声明 int ftl_rmw_write() |

| 6 | hw/femu/bbssd/ftl.c | 实现读旧页+合并+写新页+更新映射 |

| 7 | hw/femu/bbssd/bb.c | 识别 RMW 命令，设置 req->is_rmw |

| 8 | 重编译 | cd build-femu && make -j$(nproc) |



### 第二阶段：Linux 内核侧（F2FS + NVMe 驱动）



| # | 文件 | 修改 |

|---|------|------|

| 9 | drivers/nvme/host/nvme.h | 添加 nvme_cmd_rmw_offload 和相关声明 |

| 10 | drivers/nvme/host/core.c | 实现 nvme_submit_rmw_cmd() |

| 11 | fs/f2fs/data.c | 非对齐写路径调 RMW 命令替代读回旧页 |

| 12 | fs/f2fs/f2fs.h | 添加调试/proc 接口控制 RMW 开关 |

| 13 | 重编译 | cd linux && make -j$(nproc) |



### 第三阶段：验证



| # | 操作 |

|---|------|

| 14 | 替换 bzImage + initrd，重启 FEMU |

| 15 | VM 内挂载 F2FS，执行非对齐写测试 |

| 16 | 通过 QMP 查看 FTL 统计验证 RMW 次数下降 |



---



## 附录：快速参考



### 编译命令



```bash

# 重编译 FEMU（在 WSL 中）

cd ~/femu-project/FEMU/build-femu

make -j$(nproc)



# 重编译内核

cd ~/femu-project/linux

make -j$(nproc)



# 生成新 initrd

sudo make modules_install

KVER=$(make kernelrelease)

sudo mkinitramfs -o ~/femu-project/initrd.img $KVER

```



### 调试命令



```bash

# 在 VM 内查看 NVMe 设备

nvme list

nvme id-ctrl /dev/nvme0



# 在 WSL 宿主机通过 QMP 调试 FEMU

echo ${$"execute$":$"qmp_capabilities$"$}} | socat - UNIX-CONNECT:./qmp-sock

```



### 关键概念映射



| 概念 | F2FS 侧 | FEMU 侧 |

|------|---------|---------|

| 最小写入单元 | 4KB Page | Sector（512B 或 4KB） |

| 地址映射 | Node -> Data Block Addr | LPN -> PPA（maptbl[]） |

| 写分配 | LFS：追加到当前 Segment | 顺序写：wp 指向下一个空闲页 |

| 垃圾回收 | Segment Cleaning | Line GC（victim block 回收） |

| 数据通路 | bio -> NVMe 驱动 -> PCIe | ring buffer -> FTL 线程 -> NAND 后端 |



---



## 补充 1：F2FS Direct IO vs Buffered IO

### Buffered IO（走 Page Cache）

写入路径：
```
f2fs_file_write_iter()
  -> f2fs_buffered_write_iter()
    -> f2fs_write_begin()        [准备页缓存页]
    -> copy_page_from_iter()     [用户数据拷贝到页缓存]
    -> f2fs_write_end()          [标记脏页]

[异步回写，由 writeback 机制触发]
  -> f2fs_write_data_pages()
    -> f2fs_write_single_data_page()
      -> f2fs_do_write_data_page()
        -> do_write_page()       [== 所有写路径汇聚点 ==]
          -> f2fs_submit_page_write()
            -> submit_bio()
```

Buffered IO 的 RMW 场景：
- 写部分页时，f2fs_write_begin() 先 readpage() 读回旧页到 page cache
- 用户数据 copy 到 page cache 中偏移位置
- writeback 时将完整 4KB 页写出

### Direct IO（绕过 Page Cache）

F2FS 不走传统 .direct_IO 回调（设置为 noop_direct_IO），而是通过 iomap 框架：

```
f2fs_file_write_iter()
  -> f2fs_dio_write_iter()
    -> iomap_dio_rw()           [内核通用 DIO 框架]
      -> f2fs_iomap_begin()     [映射逻辑块到物理块]
        -> f2fs_map_blocks()
      -> bio_iov_iter_get_pages() [直接 pin 用户页]
      -> submit_bio()           [直接提交 bio，不经过 page cache]
```

Direct IO 的 RMW 差异：
- DIO 要求对齐写入（O_DIRECT 必须扇区对齐）
- 因此 DIO 场景下通常不会产生非页对齐写
- 但如果块设备支持 512B 逻辑块，DIO 写 512B 仍是对齐的
- RMW 卸载主要针对 Buffered IO 的写部分页场景

### 关键差异总结

| 特性 | Buffered IO | Direct IO |
|------|-------------|-----------|
| 数据路径 | Page Cache -> bio | 用户页直接 -> bio |
| RMW 触发 | 写部分页时 readpage | 通常不触发（对齐要求） |
| 合入点 | do_write_page() | bio 直接提交 |
| RMW 卸载适用 | 是（主要场景） | 较少（对齐保证） |

---

## 补充 2：F2FS 元数据写入路径

F2FS 写操作分三大类，全部汇聚到 do_write_page()：

```
                    writeback / fsync / checkpoint
                           |
        +------------------+------------------+
        |                  |                  |
        v                  v                  v
   DATA pages         NODE pages          META pages
   (文件数据)          (inode/间接块)       (CP/SIT/NAT/SSA)
        |                  |                  |
        v                  v                  v
f2fs_write_data_page  f2fs_write_node_page  f2fs_write_meta_page
  (data.c:4022)         (node.c:1719)        (checkpoint.c:350)
        |                  |                  |
        v                  v                  v
f2fs_do_write_data_   __write_node_page    __f2fs_write_meta_page
    page()             (node.c:1571)        (checkpoint.c:317)
        |                  |                  |
        v                  v                  v
  f2fs_outplace_write_  f2fs_do_write_node_  f2fs_do_write_meta_
      data()                page()               page()
        |                  |                  |
        +------------------+------------------+
                           |
                           v
                    do_write_page()
                  (segment.c:3337)
                           |
                           v
                f2fs_submit_page_write()
                  (segment.c:3354)
                           |
                           v
                       submit_bio()
```

### 各类元数据说明

| 类型 | 说明 | 写入频率 | RMW 卸载价值 |
|------|------|----------|-------------|
| DATA | 文件数据页 | 高 | 最高（大量小写） |
| NODE | inode 和间接块 | 中 | 中（4KB 整页居多） |
| META/CP | Checkpoint 页 | 低（定期） | 低（整页写入） |
| META/SIT | Segment 信息表 | 低 | 低 |
| META/NAT | Node 地址表 | 中 | 中 |
| META/SSA | Segment 摘要区 | 低 | 低 |

关键结论：
1. DATA 和 NODE 写入频率最高，是 RMW 卸载的主要目标
2. 所有路径汇聚到 do_write_page()，如果在 do_write_page() 层做拦截，能覆盖所有写入类型
3. 但 DATA 和 NODE 对 RMW 卸载的收益不同，建议先聚焦 DATA 路径

---

## 补充 3：FEMU 侧 RMW 缓存设计

### 设计动机

每次非对齐写都立即触发 SSD 内部 RMW（读旧页 + 合并 + 写新页）仍有效率问题：
- 同一 LBA 多次小写会触发多次 RMW
- 如果后续写入覆盖了整个页，之前的 RMW 是浪费

RMW 缓存：在 SSD 内部用小块 DRAM 暂存非对齐写，延迟合并，批量刷写。

### 缓存结构

```c
// RMW 缓存条目
struct rmw_cache_entry {
    uint64_t lba;              // 逻辑块地址
    uint8_t *data;             // 缓存的完整页数据（4KB）
    bool    *dirty_map;        // 哪些扇区被修改过（bitmap）
    uint64_t timestamp;        // 插入时间（LRU 淘汰用）
    bool    valid;             // 条目是否有效
};

// RMW 缓存
struct rmw_cache {
    struct rmw_cache_entry *entries;  // 条目数组
    int max_entries;                  // 最大条目数（如 64/128/256）
    int nr_entries;                   // 当前条目数
    struct rte_ring *free_list;       // 空闲条目链表
};
```

### 工作流程

```
Host 发送 RMW-OFFLOAD 命令
    |
    v
FEMU 收到命令：
    |
    +-- 查询 RMW 缓存：LBA 是否已存在？
    |   |
    |   +-- 命中（Cache Hit）：
    |   |   |
    |   |   +-- 合并新数据到缓存条目
    |   |   +-- 更新 dirty_map
    |   |   +-- 如果 dirty_map 全满（整页脏）：
    |   |   |     -> 直接写入 NAND，释放缓存条目
    |   |   +-- 如果仍是部分脏：
    |   |         -> 留在缓存，返回成功
    |   |
    |   +-- 未命中（Cache Miss）：
    |       |
    |       +-- 分配新缓存条目
    |       +-- 标记对应扇区为脏
    |       +-- 如果缓存满：
    |             -> LRU 淘汰一个条目
    |             -> 对该条目执行 RMW 写入 NAND
    |
    v
返回完成
```

### 正常 Write 命令的缓存交互

当 Host 发送正常整页 Write 命令时，也需要检查：

```
Host 发送普通 WRITE 命令（整页）
    |
    v
FEMU: 查询 RMW 缓存
    |
    +-- 如果 LBA 在缓存中：
    |   |
    |   +-- 合并缓存中的部分修改到新写入的整页数据
    |   +-- 写入 NAND（完整页）
    |   +-- 释放缓存条目
    |
    +-- 如果 LBA 不在缓存中：
        |
        +-- 正常 FTL 写入流程
```

### 缓存参数

| 参数 | 建议值 | 说明 |
|------|--------|------|
| max_entries | 64 ~ 256 | 缓存条目数，每条占用 4KB+bitmap |
| 总内存占用 | 256KB ~ 1MB | max_entries * 4KB |
| 淘汰策略 | LRU | 最久未使用的条目先淘汰 |
| 刷写策略 | 被动淘汰 + 定时刷写 | 缓存满或超时（如前 100ms）刷写 |

### 缓存命中率分析

对于 F2FS 非对齐写场景：

| 场景 | 命中率预期 | 原因 |
|------|-----------|------|
| 追加写多字节 | 高（>80%） | 同一 LBA 连续追加 |
| 随机小写 | 中（50%） | LBA 分散 |
| 大文件顺序写 | 低 | 每次写新 LBA |

### 完整修改清单（更新）

| # | 文件 | 修改 |
|---|------|------|
| 1 | hw/femu/nvme.h | 添加 NVME_CMD_RMW_OFFLOAD 0xC0 |
| 2 | hw/femu/nvme.h | NvmeRequest 加 is_rmw, rmw_offset, rmw_len |
| 3 | hw/femu/nvme-io.c | switch 加 case NVME_CMD_RMW_OFFLOAD |
| 4 | hw/femu/nvme-io.c | 实现 nvme_rmw_offload() handler |
| 5 | hw/femu/bbssd/rmw_cache.h | RMW 缓存数据结构（新增文件） |
| 6 | hw/femu/bbssd/rmw_cache.c | 缓存查找/插入/合并/淘汰/刷写（新增） |
| 7 | hw/femu/bbssd/ftl.c | FTL 写路径集成缓存 + RMW 逻辑 |
| 8 | hw/femu/bbssd/bb.c | BBSSD IO 调度集成缓存检查 |
| 9 | drivers/nvme/host/nvme.h | 添加 RMW opcode 和 helper 声明 |
| 10 | drivers/nvme/host/core.c | 实现 nvme_submit_rmw_cmd() |
| 11 | fs/f2fs/data.c | Buffered IO 非对齐写调 RMW 命令 |
| 12 | fs/f2fs/f2fs.h | 添加调试/proc 开关控制 RMW |
