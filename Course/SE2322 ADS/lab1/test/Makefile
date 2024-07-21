# 定义编译器
CXX = g++
# 定义编译选项
STD = c++17
CXXFLAGS = -g
# 定义目标文件
TARGET = main
# 定义源文件
SRCS = main.cpp compressed_radix_tree.cpp radix_tree.cpp
# 生成目标文件列表
OBJS = $(SRCS:.cpp=.o)
# 默认目标
all: $(TARGET)
# 生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
# 生成目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)
