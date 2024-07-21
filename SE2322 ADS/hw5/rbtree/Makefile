# 指定编译器
CC=g++
# 编译参数
CFLAGS=-c -Wall
# 目标文件
TARGET=test
# 源文件
SOURCES=main.cpp rbtree.cpp
# 头文件
HEADERS=rbtree.h
# 目标文件
OBJECTS=$(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
