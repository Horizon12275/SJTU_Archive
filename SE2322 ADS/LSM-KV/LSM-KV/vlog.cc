#include "vlog.h"

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

/*
同时还需要恢复 tail 和 head 的正确值，具体来说：head 的值就是当前文件的大
小；tail 则需要首先定位到文件空洞后的第一个 Magic，接着进行 crc
校验，如果校验通过则该 Magic 的位置即为 tail 的值，否则寻找下一
个 Magic 并进行校验，直到校验通过，则对应的 Magic 位置即为 tail
的正确值。因此，其启动后应读取到上次系统运行所记录的 SSTable
数据及 vLog 文件。
*/

/*
 off_t seek_data_block(const std::string& path)：返回自 path 文件开头起
第一个数据（即 vLog 尾部第一个非洞的字节）的偏移量。注意这个
偏移量可能会比实际数据所处的偏移量小，你可能需要继续往后读
取直到找到你自定义的 vLog entry 的开始符号，并在确认 crc16 校验
通过之后，才能设置 tail 的值（详见第 4 节 reset 操作）。
*/

vLog::vLog(std::string vlog_path) : vlog_path(vlog_path)
{
    // 读取vLog文件，恢复tail和head的值，存储到tail和head中
    std::ifstream in(vlog_path, std::ios::binary | std::ios::in);

    // 如果文件不存在，则tail和head都为0，在对应位置创建文件
    if (!in)
    {
        tail = 0;
        head = 0;
        std::ofstream out(vlog_path, std::ios::binary);
        out.close();
        return;
    }
    // 如果文件存在，则读取tail和head的值，其中tail在文件的开头，head在文件的末尾
    // 读取head的值，直接定位到文件末尾，并进行设置
    in.seekg(0, std::ios::end);
    this->head = in.tellg();

    // 读取tail的值，首先跳过所有文件空洞，找到第一个Magic，然后进行crc校验，如果校验通过则该Magic的位置即为tail的值
    in.seekg(utils::seek_data_block(vlog_path));
    // 一个个地读字符、直到找到第一个Magic
    // 以下这段代码由于调试了半天都没有找到问题，所以我学习了一下我的舍友的写法，发现他的写法更加简洁且正确，感谢他允许我将这部分作为参考
    char c;
    uint16_t checksum;            // 校验和
    std::vector<unsigned char> v; // key vlen value组成的二进制序列
    while (in.get(c))
    {
        // 如果不是magic，则继续读取
        if (c != (char)magic)
            continue;
        int pos = in.tellg() - 1;                                       // 记录c的位置 因为get会使指针后移 所以要减1
        in.read(reinterpret_cast<char *>(&checksum), sizeof(uint16_t)); // 读取校验和
        v.clear();
        // 读取key vlen value组成的二进制序列 直到遇到magic
        while (in.get(c))
        {
            if (c == (char)magic)
            {
                // std::cout << "find magic : " << c << std::endl;
                in.seekg(-1, std::ios::cur); // 回退一个字节 下一个循环会读取到magic
                break;
            }
            v.push_back(c);
        }
        if (utils::crc16(v) == checksum) // 校验和正确
        {
            tail = pos; // 更新尾部位置
            return;
        }
    }
    tail = head; // 找不到尾部 说明文件全是空洞 则文件尾部是文件大小
    in.close();
}

// 析构函数
vLog::~vLog()
{
}

// 从vLog中根据offset及vlen取出value，offset指向value的起始位置，vlen为value的长度
std::string vLog::getValue(uint64_t offset, uint32_t vlen)
{
    std::ifstream in(vlog_path, std::ios::binary);
    in.seekg(offset, std::ios::beg);
    char value[vlen];
    in.read(value, vlen);
    in.close();
    return std::string(value, vlen);
}