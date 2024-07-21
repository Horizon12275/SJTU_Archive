#pragma once

#include "config.h"
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include "utils.h"
#include "bloomFilter.h"

/*
vLog 用于存储键值对的值，vLog 文件（文件名作为构造函数参数给
出）的结构如图3所示，其中 head 为新数据 append 时的起始位置，tail 为
空洞之后第一个有效数据的起始位置，文件由连续的 vLog entry 组成，每
个entry 包括五个字段：
1) Magic (1 Byte)：作为每个 entry 的开始符号，可自己定义，如 0xff。由
于系统没有持久化 vLog 文件 head 和 tail 的位置，Magic 被用来查找
vLog 数据开始的位置。
2) Checksum (2 Byte)：通过对由 key, vlen, value 拼接而成的二进制序列计
算crc16 得到（ crc16 函数在 utils.h 中给出）。Magic 和 checksum 二者
在读取数据时共同用于检查数据是否被完整写入，这有利于在系统
初始化时确定 tail 的位置（详见第 4 节 reset 操作）。
3) Key (8 Byte)：键，为了方便垃圾回收，vLog 同时保存了 Key，具体
原因请见第5节垃圾回收。
4) vlen (4 Byte)：值的长度。
5) Value：值。
由于对键值存在修改或删除操作，因此每间隔一段时间系统就需要对
vLog 文件进行垃圾回收。简单来说会扫描 vLog 尾部 (tail) 一段vLog entry，
检查其是否过期（被修改或删除），将没有过期的 entry 重新插入到 LSM
Tree 中，并使用文件系统的 fallocate 系统调用将这块回收的区域打洞。垃
圾回收的具体流程将在第5节详细介绍，在此先不赘述。
*/

// vLog entry，包括 Magic, Checksum, Key, vlen, Value
struct vlogEntry
{
    uint8_t magic;     // 作为每个 entry 的开始符号
    uint16_t checksum; // 通过对由 key, vlen, value 拼接而成的二进制序列计算crc16 得到
    key_type key;      // 键
    uint32_t vlen;     // 值的长度
    value_type value;  // 值
};

// vLog 类，用于操作 vLog 文件
class vLog
{
public:
    std::string vlog_path;            // vLog 文件路径
    uint64_t head;                    // 新数据 append 时的起始位置
    uint64_t tail;                    // 空洞之后第一个有效数据的起始位置
    uint8_t magic = vlog_entry_magic; // 作为每个 entry 的开始符号

    vLog(std::string vlog_path);                          // 从 vLog 文件路径构造 vLog 对象
    ~vLog();                                              // 析构函数
    std::string getValue(uint64_t offset, uint32_t vlen); // 从vLog中根据 offset 及 vlen 取出 value
};