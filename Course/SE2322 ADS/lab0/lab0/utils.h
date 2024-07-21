#ifndef UTILS_H
#define UTILS_H

#include <cstring>
#include <fstream>
#include <bitset>

namespace utils
{

    std::string intToBinaryString(int num)
    {
        std::bitset<64> bits(num);
        return bits.to_string();
    }

    std::string getFileName(const std::string &fileName, const std::string &suffix)
    {
        size_t final_pos = fileName.find_last_of("/\\");
        size_t suffix_pos = fileName.find_last_of('.');
        return final_pos == std::string::npos ? "./output/" + fileName.substr(0, suffix_pos) + suffix : "./output/" + fileName.substr(final_pos + 1, suffix_pos - final_pos - 1) + suffix;
    }

    std::string parseText(const std::string &input)
    {
        // TODO: Your code here
        std::ifstream file(input, std::ios::binary);
        std::string result;
        char ch;
        while (file.get(ch))
            result += ch;
        file.close();
        // std::cout << result << std::endl;
        return result;
    }

    void output(const std::string &output, const std::string &data)
    {
        // TODO: Your code here
        std::ofstream file(output);
        file << data;
        file.close();
        // std::cout << data << std::endl;
    }

    std::string codingTable2String(const std::map<std::string, std::string> &codingTable)
    {
        std::string result = "";
        // TODO: Your code here
        for (const auto &pair : codingTable)
            result += pair.first + " " + pair.second + "\n";
        // std::cout << result << std::endl;
        return result;
    }

    void loadCodingTable(const std::string &input, std::map<std::string, std::string> &codingTable)
    {
        // TODO: Your code here
        std::string in = parseText(input);
        int currentPos = 0, readPos = 0;
        std::string character, code;
        while (readPos < in.size())
        {
            for (int i = 2; i >= 1; i--)
                if (in[readPos + i] == ' ')
                {
                    readPos += i;
                    character = in.substr(currentPos, readPos - currentPos);
                    currentPos = readPos + 1;
                    break;
                }
            readPos = in.find('\n', currentPos);
            code = in.substr(currentPos, readPos - currentPos);
            // std::cout << character << " " << code << std::endl;
            codingTable[character] = code;
            currentPos = readPos + 1;
            readPos = currentPos;
        }
    }

    std::string compress(const std::map<std::string, std::string> &codingTable, const std::string &text)
    {
        std::string result = "", tmp;
        // TODO: Your code here
        for (int i = 0; i < text.size(); i++)
        {
            if (i < text.size() - 1)
            {
                std::string character = text.substr(i, 2);
                if (codingTable.find(character) != codingTable.end())
                {
                    tmp += codingTable.at(character);
                    // std::cout << tmp << std::endl;
                    i++;
                    continue;
                }
            }
            std::string character = text.substr(i, 1);
            tmp += codingTable.at(character);
            // std::cout << tmp << std::endl;
        }
        int len = tmp.length();

        std::string bitsLen = intToBinaryString(len);
        std::string littleEndian;
        for (int i = 7; i >= 0; i--)
            littleEndian = littleEndian +
            char(bitsLen[i * 8]) + char(bitsLen[i * 8 + 1]) + 
            char(bitsLen[i * 8 + 2]) + char(bitsLen[i * 8 + 3]) +
            char(bitsLen[i * 8 + 4]) + char(bitsLen[i * 8 + 5]) +
            char(bitsLen[i * 8 + 6]) + char(bitsLen[i * 8 + 7]);
        //std::cout << littleEndian << std::endl;
        tmp = littleEndian + tmp;
        len += 64;

        if (len % 8 != 0)
        {
            tmp += std::string(8 - len % 8, '0');
            len += 8 - len % 8;
        }

        std::string tmpResult;
        for (int i = 0; i < len; i += 8)
        {
            tmpResult = tmp.substr(i, 8);
            result += char(stoi(tmpResult, nullptr, 2));
        }

        //std::cout << result << std::endl;
        return result;
    }

}; // namespace utils

#endif