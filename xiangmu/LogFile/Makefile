# ================= LogFile Makefile =================

# 编译器
CXX = g++
CXXFLAGS = -std=c++14 -Wall -g

# 包含目录
INCLUDES = -Iinclude

# 库文件路径
LIBS = -lpthread

# 源文件
SRCS = src/AppendFile.cpp src/AsynLogging.cpp src/CountDownLatch.cpp \
       src/LogFile.cpp src/LogMessage.cpp src/Logger.cpp src/Timestamp.cpp

# 目标文件
OBJS = $(SRCS:.cpp=.o)

# 静态库名称
TARGET = liblogfile.a

# 测试程序
TEST_TARGET = test_log_starter

# 默认目标
all : $(TARGET) $(TEST_TARGET)

# 编译静态库
$(TARGET) : $(OBJS)
	ar rcs $@ $(OBJS)

# 编译静态库并清理中间文件
liblogfile.a.clean : $(OBJS)
	ar rcs $(TARGET) $(OBJS)
	rm -f $(OBJS)

# 编译测试程序
$(TEST_TARGET) : test_log_starter.cpp $(TARGET)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(TARGET) $(LIBS)

# 编译源文件
src/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET)
	rm -rf test_logs config_logs

.PHONY: all clean
