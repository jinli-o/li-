# ================= Server Makefile =================
# 依赖：需要先编译 LogFile 日志库

# 编译器
CXX = g++
CXXFLAGS = -std=c++14 -Wall -g

# 包含目录
INCLUDES = -I../LogFile/include

# 库文件路径
LIBS = -lpthread -ljsoncpp -lmysqlclient -lcrypto

# LogFile 库路径
LOGFILE_LIB = ../LogFile/liblogfile.a
LOGFILE_DIR = ../LogFile

# 源文件
SRCS = server.cpp ConnectionPool.cpp

# 目标
all : logfile server

# 先编译 LogFile 库（编译后自动清理中间文件）
logfile:
	@echo "编译 LogFile 日志库..."
	cd $(LOGFILE_DIR) && make liblogfile.a.clean

# 编译服务器（直接编译，不生成中间 .o 文件）
server : $(LOGFILE_LIB)
	@echo "编译服务器..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o server $(SRCS) $(LOGFILE_LIB) $(LIBS)

# 清理
clean:
	rm -f server
	cd $(LOGFILE_DIR) && make clean

.PHONY: all clean logfile
