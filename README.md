
### 这是我的项目基本架构，虽然暂时还没有实现前端页面但是基础框架还是比较全面的。


# 票务预约系统 - 项目详细文档

## 目录

- [一、项目概述](#一项目概述)
- [二、项目架构](#二项目架构)
- [三、目录结构](#三目录结构)
- [四、技术栈](#四技术栈)
- [五、模块详解](#五模块详解)
  - [5.1 LogFile 日志模块](#51-logfile-日志模块)
  - [5.2 Server 服务端模块](#52-server-服务端模块)
  - [5.3 Client 客户端模块](#53-client-客户端模块)
- [六、通信协议](#六通信协议)
- [七、数据库设计](#七数据库设计)
- [八、配置文件说明](#八配置文件说明)
- [九、编译与运行](#九编译与运行)
- [十、设计模式应用](#十设计模式应用)
- [十一、技术亮点](#十一技术亮点)
- [十二、API 接口文档](#十二api-接口文档)
- [十三、常见问题](#十三常见问题)
- [十四、Redis 缓存方案分析](#十四redis-缓存方案分析)
---

## 一、项目概述

### 1.1 项目简介

本项目是一个基于 **C++ 的票务预约系统（Ticket Reservation System）**，采用经典的 **TCP C/S（客户端/服务器）架构** 实现。用户可以通过命令行客户端进行票务查询、预约、取消等操作，服务器负责处理并发请求并与 MySQL 数据库交互。

### 1.2 核心功能

| 功能模块 | 功能描述 |
|----------|----------|
| 用户管理 | 注册、登录、退出 |
| 票务查询 | 查看所有可预约的票务信息 |
| 票务预约 | 预约指定票务 |
| 预约管理 | 查看个人预约、取消预约 |
| 系统日志 | 自研异步日志系统，支持多级别日志输出 |

### 1.3 系统特点

- **高性能网络 I/O**：基于 epoll + EPOLLONESHOT 实现事件驱动
- **并发处理**：线程池解耦网络事件与业务逻辑
- **数据库连接池**：减少频繁建立/销毁连接的开销
- **异步日志**：双缓冲机制，不影响主线程性能
- **安全加密**：MD5 密码加密存储
- **事务保障**：数据库事务保证操作原子性

---

## 二、项目架构

### 2.1 整体架构图
<img width="1536" height="1024" alt="项目1" src="https://github.com/user-attachments/assets/265e62bc-3f90-4786-b315-d9c938bd4b69" />

2.2 数据流

```
用户输入 → 客户端验证 → JSON序列化 → TCP发送
                                        ↓
                              服务器接收 → JSON反序列化
                                        ↓
                              业务处理 → 数据库操作
                                        ↓
                              结果序列化 → TCP返回
                                        ↓
                              客户端显示 ← JSON反序列化
```

### 虽然我没有加redis但是我觉的加redis可以降低对数据库直接操作也是很好的
<img width="1331" height="1181" alt="项目2" src="https://github.com/user-attachments/assets/2eff8491-15b4-4031-a7bd-a8bea558fdbf" />

---

## 三、目录结构

```
xiangmu/
├── .vscode/                          # VS Code IDE 配置
│   ├── c_cpp_properties.json         
│   ├── launch.json                   
│   └── settings.json                 
│
├── LogFile/                          # 自研日志库（静态库）
│   ├── Makefile                      # 日志库编译脚本
│   ├── liblogfile.a                  # 编译产物：静态库
│   ├── include/                      # 头文件目录
│   │   ├── AppendFile.hpp            # 底层文件追加写入类
│   │   ├── AsynLogging.hpp           # 异步日志核心类
│   │   ├── CountDownLatch.hpp        # 倒计时门闩
│   │   ├── LogCleaner.hpp            # 日志文件清理器
│   │   ├── LogCommon.hpp             # 公共定义
│   │   ├── LogFile.hpp               # 日志文件管理类
│   │   ├── Logger.hpp                # 日志器类
│   │   ├── LoggerManager.hpp         # 日志管理器单例
│   │   ├── LogMessage.hpp            # 日志消息类
│   │   ├── LogStarter.hpp            # 日志系统启动器
│   │   └── Timestamp.hpp             # 高精度时间戳类
│   └── src/                          # 源文件目录
│       ├── AppendFile.cpp
│       ├── AsynLogging.cpp
│       ├── CountDownLatch.cpp
│       ├── LogFile.cpp
│       ├── Logger.cpp
│       ├── LogMessage.cpp
│       └── Timestamp.cpp
│
├── server/                           # 服务端
│   ├── makefile                      # 服务端编译脚本
│   ├── run.sh                        # 一键编译+启动脚本
│   ├── my.conf                       # 服务器配置文件
│   ├── server.h                      # 服务端头文件
│   ├── server.cpp                    # 服务端实现 + main()
│   ├── ConnectionPool.hpp            # MySQL 连接池头文件
│   └── ConnectionPool.cpp            # MySQL 连接池实现
│
└── client/                           # 客户端
    ├── makefile                      # 客户端编译脚本
    ├── run.sh                        # 一键编译+启动脚本
    ├── client.h                      # 客户端头文件
    └── client.cpp                    # 客户端实现 + main()
```

---

## 四、技术栈

| 类别 | 技术 | 版本/说明 |
|------|------|----------|
| **编程语言** | C++ | C++14 标准 |
| **编译器** | g++ | GCC |
| **操作系统** | Linux | 依赖 POSIX API |
| **网络通信** | TCP Socket | AF_INET, SOCK_STREAM |
| **I/O 多路复用** | epoll | epoll_create1, epoll_ctl, epoll_wait |
| **并发模型** | 多线程 | pthread 线程池 |
| **数据库** | MySQL | libmysqlclient |
| **数据序列化** | JSON | jsoncpp 库 |
| **加密算法** | MD5 | OpenSSL libcrypto |
| **构建工具** | Makefile | - |
| **日志系统** | 自研 LogFile | 异步日志库 |
| **IDE** | VS Code | C/C++ Runner + GDB |

### 4.1 依赖库

```bash
# 编译时需要链接的库
-lpthread          # POSIX 线程库
-ljsoncpp          # JSON 序列化库
-lmysqlclient      # MySQL 客户端库
-lcrypto           # OpenSSL 加密库
```

---

## 五、模块详解

### 5.1 LogFile 日志模块

#### 5.1.1 模块概述

LogFile 是一个自研的异步日志库，提供高性能、低延迟的日志记录能力。采用双缓冲机制，前端写入与后台落盘分离，不影响业务线程性能。

**编译产物**：`liblogfile.a`（静态库）

#### 5.1.2 架构层次
<img width="1195" height="1316" alt="日志" src="https://github.com/user-attachments/assets/ff0b68af-a3d6-4d0e-88bf-c8fb7d52549c" />

#### 5.1.3 核心类详解

##### 1. AppendFile - 文件追加写入类

**功能**：最底层的文件 I/O 封装，负责将日志数据写入文件。

**关键特性**：
- 使用 1MB 用户态缓冲区（`setbuffer`），减少系统调用次数
- 使用 `fwrite_unlocked`（非线程安全版本），由上层保证线程安全
- 提供 C 和 C++ 两种风格的 append 接口

**核心接口**：
```cpp
class AppendFile {
public:
    explicit AppendFile(const std::string& filename);
    ~AppendFile();

    void append(const char* logline, size_t len);  // C++ 风格
    void append(const char* logline);               // C 风格
    void flush();
    off_t writtenBytes() const { return writtenBytes_; }

private:
    size_t write(const char* logline, size_t len);
    FILE* fp_;
    char buffer_[1024 * 1024];  // 1MB 缓冲区
    off_t writtenBytes_;
};
```

##### 2. Timestamp - 高精度时间戳类

**功能**：提供微秒级精度的时间戳，支持多种格式化方式。

**关键特性**：
- 微秒级精度（`gettimeofday`）
- 支持日志显示格式：`2026/02/22-14:30:25.123456Z`
- 支持文件名格式
- 内联辅助函数：时间差计算、时间加法

**核心接口**：
```cpp
class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    static Timestamp now();
    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;
};
```

##### 3. CountDownLatch - 倒计时

**功能**：线程同步原语，用于主线程等待后台线程启动完成。

**实现原理**：基于 `mutex` + `condition_variable`

**核心接口**：
```cpp
class CountDownLatch {
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();
    int getCount() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    int count_;
};
```

##### 4. LogMessage - 日志消息类

**功能**：封装日志头部信息，支持流式写入。

**日志头部格式**：
```
[2026/02/22-14:30:25.123456Z] [TID:12345] [INFO] [file.cpp:func():42]
```

**核心接口**：
```cpp
class LogMessage {
public:
    LogMessage(LogLevel level, const char* file, int line, const char* func);
    ~LogMessage();

    std::ostream& stream() { return stream_; }
    std::string toString() const;

private:
    LogLevel level_;
    const char* file_;
    int line_;
    const char* func_;
    std::ostringstream stream_;
    Timestamp time_;
    pid_t tid_;
};

// 流式写入模板
template<typename T>
LogMessage& operator<<(LogMessage& msg, const T& t) {
    msg.stream() << t;
    return msg;
}
```

##### 5. Logger - 日志器类

**功能**：用户接口层，提供 `LOG_INFO`、`LOG_ERROR` 等宏接口。

**关键特性**：
- 利用 RAII：析构时自动输出日志
- 支持全局回调（OutputFunc / FlushFunc）自定义输出目标
- 自动注入 `__FILE__`、`__func__`、`__LINE__`

**核心接口**：
```cpp
class Logger {
public:
    enum LogLevel {
        TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LOG_LEVELS
    };

    Logger(LogLevel level, const char* file, int line, const char* func);
    ~Logger();

    LogMessage& stream() { return impl_.stream(); }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);
    static void setOutput(OutputFunc func);
    static void setFlush(FlushFunc func);

private:
    LogMessage impl_;
};

// 用户接口宏
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
    Logger(Logger::INFO, __FILE__, __LINE__, __func__).stream()
#define LOG_ERROR if (Logger::logLevel() <= Logger::ERROR) \
    Logger(Logger::ERROR, __FILE__, __LINE__, __func__).stream()
```

##### 6. LogFile - 日志文件管理类

**功能**：管理日志文件的滚动、刷新和清理。

**关键特性**：
- 日志滚动：按文件大小滚动、按天滚动
- 自动刷新：定时刷新 + 定次刷新
- 可选线程安全（通过 mutex 控制）
- 自动创建日志目录

**核心接口**：
```cpp
class LogFile {
public:
    LogFile(const std::string& basename, size_t rollSize, bool threadSafe = true);
    ~LogFile();

    void append(const char* logline, size_t len);
    void flush();

private:
    void appendUnlock(const char* logline, size_t len);
    static std::string makeFileName(const std::string& basename, size_t& fileNumber);
    void rollFile();

    const std::string basename_;
    size_t rollSize_;
    size_t count_;
    std::unique_ptr<MutexLock> mutex_;

    std::unique_ptr<AppendFile> file_;
    size_t startOfPeriod_;
    size_t lastRoll_;
    size_t lastFlush_;
};
```

##### 7. AsynLogging - 异步日志核心类

**功能**：实现双缓冲机制，前端写入与后台落盘分离。

**双缓冲机制**：
```
前端线程写入 currentBuffer_
         ↓ （写满后）
移动到 buffers_ 队列
         ↓
后台线程从队列取出
         ↓
批量写入 LogFile
```

**关键特性**：
- 单个缓冲区最大 4MB
- 队列容量 16
- 条件变量通知 + 200ms 超时等待

**核心接口**：
```cpp
class AsynLogging {
public:
    AsynLogging(const std::string& basename, size_t rollSize);
    ~AsynLogging();

    void append(const char* logline, size_t len);

private:
    void threadFunc();

    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const size_t rollSize_;
    bool running_;

    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;

    std::mutex mutex_;
    std::condition_variable cond_;
    CountDownLatch latch_;
};
```

##### 8. LogStarter - 日志系统启动器

**功能**：提供链式配置接口（Builder 模式），统一初始化日志系统。

**使用示例**：
```cpp
logStarter()
    .setBasename("server")
    .setLogDirectory("logs")
    .setRollSize(10 * 1024 * 1024)  // 10MB
    .setLogLevel(Logger::INFO)
    .start();
```

**核心接口**：
```cpp
class LogStarter {
public:
    static LogStarter& instance();

    LogStarter& setBasename(const std::string& basename);
    LogStarter& setLogDirectory(const std::string& dir);
    LogStarter& setRollSize(size_t size);
    LogStarter& setLogLevel(Logger::LogLevel level);
    LogStarter& setCleanDays(int days);

    void start();
    void stop();

private:
    LogStarter();
    std::string basename_;
    std::string logDir_;
    size_t rollSize_;
    Logger::LogLevel logLevel_;
    int cleanDays_;
    bool started_;
};
```

---

### 5.2 Server 服务端模块

#### 5.2.1 模块概述

服务端采用 epoll + 线程池架构，处理多客户端并发连接。核心功能包括用户认证、票务管理、数据库操作等。

#### 5.2.2 核心类详解

##### 1. SerConf - 配置管理类

**功能**：单例模式，从 `my.conf` 文件读取配置。

**关键特性**：
- 双重检查锁实现线程安全单例
- 管理网络配置、线程池配置、数据库配置等

**核心接口**：
```cpp
class SerConf {
public:
    static SerConf& getInstance();

    // 网络配置
    std::string getIp() const;
    int getPort() const;
    int getLisMax() const;

    // 线程池配置
    int getThreadNum() const;
    int getTaskMax() const;

    // 数据库配置
    std::string getDbIp() const;
    int getDbPort() const;
    std::string getDbName() const;
    std::string getDbUsername() const;
    std::string getDbPasswd() const;

    // 连接池配置
    int getPoolMinSize() const;
    int getPoolMaxSize() const;

    // 日志配置
    std::string getLogDir() const;
    bool isLogEnabled() const;
    std::string getLogLevel() const;

private:
    SerConf();
    void loadConfig(const std::string& filename);

    std::string ip_;
    int port_;
    int lismax_;
    int threadNum_;
    int taskMax_;
    std::string dbIp_;
    int dbPort_;
    std::string dbName_;
    std::string dbUsername_;
    std::string dbPasswd_;
    int poolMinSize_;
    int poolMaxSize_;
    std::string logDir_;
    bool logEnabled_;
    std::string logLevel_;
};
```

##### 2. TcpSer - TCP 服务器类

**功能**：核心服务器类，负责 socket 创建、epoll 管理、事件分发。

**关键特性**：
- epoll + EPOLLONESHOT 实现高性能 I/O
- 线程池分发业务处理
- 非阻塞 socket

**核心接口**：
```cpp
class TcpSer {
public:
    TcpSer();
    ~TcpSer();

    void Ser_Init();    // 初始化 socket、bind、listen、epoll
    void Run();         // 主事件循环
    void Stop();        // 停止服务器

private:
    void addEpoll(int fd);
    void delEpoll(int fd);
    void modEpoll(int fd, int events);

    int listenFd_;
    int epollFd_;
    std::map<int, std::shared_ptr<Socket>> socketMap_;
};
```

**启动流程**：
```cpp
void TcpSer::Ser_Init() {
    // 1. 创建 socket
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 设置 SO_REUSEADDR
    int opt = 1;
    setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. 绑定地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SerConf::getInstance().getPort());
    addr.sin_addr.s_addr = inet_addr(SerConf::getInstance().getIp().c_str());
    bind(listenFd_, (struct sockaddr*)&addr, sizeof(addr));

    // 4. 监听
    listen(listenFd_, SerConf::getInstance().getLisMax());

    // 5. 创建 epoll
    epollFd_ = epoll_create1(0);

    // 6. 添加监听 socket
    addEpoll(listenFd_);
}
```

##### 3. Socket 层次结构

```
Socket（基类）
    │
    ├── LisSocket（监听套接字）
    │       处理 accept 新连接
    │
    └── ConSocket（连接套接字）
            处理业务逻辑
```

**Socket 基类**：
```cpp
class Socket {
public:
    Socket(int fd);
    virtual ~Socket();

    virtual void Handle_Data() = 0;  // 纯虚函数
    int getFd() const { return fd_; }

protected:
    int fd_;
};
```

**LisSocket - 监听套接字**：
```cpp
class LisSocket : public Socket {
public:
    LisSocket(int fd);
    ~LisSocket() override;

    void Handle_Data() override;

private:
    void acceptConnection();
};
```

**ConSocket - 连接套接字**：
```cpp
class ConSocket : public Socket {
public:
    ConSocket(int fd);
    ~ConSocket() override;

    void Handle_Data() override;

private:
    void handleLogin(const Json::Value& req, Json::Value& res);
    void handleRegister(const Json::Value& req, Json::Value& res);
    void handleShowTicket(const Json::Value& req, Json::Value& res);
    void handleBookTicket(const Json::Value& req, Json::Value& res);
    void handleMyTicket(const Json::Value& req, Json::Value& res);
    void handleCancelTicket(const Json::Value& req, Json::Value& res);

    bool sendResponse(const Json::Value& res);
    bool recvRequest(Json::Value& req);
};
```

##### 4. ThreadPool - 线程池

**功能**：管理工作线程，处理业务任务。

**关键特性**：
- 环形队列任务队列
- `pthread_mutex` + `pthread_cond` 同步
- 可配置线程数和任务队列大小

**核心接口**：
```cpp
class ThreadPool {
public:
    ThreadPool(int threadNum, int taskMax);
    ~ThreadPool();

    bool addTask(std::function<void()> task);
    void stop();

private:
    static void* worker(void* arg);
    void run();

    std::vector<pthread_t> threads_;
    std::queue<std::function<void()>> tasks_;

    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
    bool running_;
    int threadNum_;
    int taskMax_;
};
```

**工作流程**：
```
1. 主线程调用 addTask() 添加任务
2. 任务入队，唤醒等待的工作线程
3. 工作线程被唤醒，从队列取出任务
4. 工作线程执行任务
5. 任务执行完毕，继续等待新任务
```

##### 5. MysqlClient - 数据库操作封装

**功能**：封装 MySQL 操作，提供事务支持。

**核心接口**：
```cpp
class MysqlClient {
public:
    MysqlClient(MYSQL* conn);
    ~MysqlClient();

    bool Begin();       // 开始事务
    bool RollBack();    // 回滚事务
    bool Commit();      // 提交事务

    bool query(const std::string& sql);
    MYSQL_RES* storeResult();

    bool login(const std::string& tel, const std::string& passwd, std::string& name);
    bool registerUser(const std::string& tel, const std::string& name, const std::string& passwd);
    bool showTicket(Json::Value& tickets);
    bool bookTicket(const std::string& tel, int ticketId);
    bool myTicket(const std::string& tel, Json::Value& tickets);
    bool cancelTicket(const std::string& tel, int ydId);

private:
    MYSQL* conn_;
    std::string salt_;
};
```

##### 6. ConnectionPool - MySQL 连接池

**功能**：管理 MySQL 连接的复用，减少连接建立/销毁开销。

**关键特性**：
- 单例模式（Meyers' Singleton）
- 最小/最大连接数可配
- 连接归还时自动 ping 检测有效性
- 条件变量 + mutex 线程安全

**核心接口**：
```cpp
class ConnectionPool {
public:
    static ConnectionPool& getInstance();

    void init(const std::string& ip, int port, const std::string& dbname,
              const std::string& username, const std::string& passwd,
              int minSize, int maxSize);

    MYSQL* getConnection();
    void releaseConnection(MYSQL* conn);

private:
    ConnectionPool();
    ~ConnectionPool();

    void createConnection();

    std::queue<MYSQL*> connQueue_;
    std::mutex mutex_;
    std::condition_variable cond_;

    int minSize_;
    int maxSize_;
    int currentSize_;

    std::string ip_;
    int port_;
    std::string dbname_;
    std::string username_;
    std::string passwd_;
};
```

**连接池工作流程**：
```
1. 初始化时创建 minSize 个连接
2. 获取连接：
   - 队列非空：取出一个连接，ping 检测有效性
   - 队列为空且未达上限：创建新连接
   - 队列为空且已达上限：等待其他线程释放连接
3. 释放连接：
   - 连接有效：归还到队列
   - 连接无效：销毁，创建新连接
```

---

### 5.3 Client 客户端模块

#### 5.3.1 模块概述

客户端提供命令行交互界面，用户可以通过菜单进行各项操作。

#### 5.3.2 核心类详解

##### TcpClient - TCP 客户端类

**功能**：连接服务器，提供交互式菜单。

**关键特性**：
- 支持手机号验证（11位数字）
- 支持密码验证（至少6位）
- 使用 `select` + `MSG_PEEK` 检测连接状态

**核心接口**：
```cpp
class TcpClient {
public:
    TcpClient(const std::string& ip, int port);
    ~TcpClient();

    bool connect();
    void disconnect();
    bool isConnected() const;

    void run();  // 主交互循环

private:
    void showMenu();
    void showLoginMenu();
    void showMainMenu();

    bool login();
    bool registerUser();
    void showTicket();
    void bookTicket();
    void myTicket();
    void cancelTicket();

    bool sendRequest(const Json::Value& req);
    bool recvResponse(Json::Value& res);

    bool validateTel(const std::string& tel);
    bool validatePasswd(const std::string& passwd);

    std::string md5(const std::string& str);

    int sockfd_;
    std::string ip_;
    int port_;
    bool connected_;
    std::string currentUserTel_;
    std::string currentUserName_;
};
```

**交互流程**：
```
┌─────────────────────────────────┐
│         欢迎使用票务系统          │
├─────────────────────────────────┤
│  1. 登录                        │
│  2. 注册                        │
│  3. 退出                        │
└─────────────────────────────────┘
        ↓ （登录成功）
┌─────────────────────────────────┐
│         主菜单 [用户名]          │
├─────────────────────────────────┤
│  1. 查看票务                    │
│  2. 预约票务                    │
│  3. 我的预约                    │
│  4. 取消预约                    │
│  5. 退出登录                    │
└─────────────────────────────────┘
```

---

## 六、通信协议

### 6.1 协议概述

客户端与服务器之间使用 **JSON 格式** 通过 TCP socket 进行通信。

### 6.2 请求格式（客户端 → 服务器）

```json
{
    "type": <操作类型编号>,
    "user_tel": "手机号",
    "user_passwd": "密码（MD5加密）",
    "user_name": "用户名",
    "ticket_id": "票务ID",
    "yd_id": "预约ID"
}
```

### 6.3 响应格式（服务器 → 客户端）

```json
{
    "status": "OK" 或 "ERR",
    "msg": "提示信息",
    "user_name": "用户名",
    "ticket_arr": [
        {
            "tk_id": 1,
            "tk_name": "票务名称",
            "tk_max": 100,
            "tk_count": 50,
            "day_time": "2026-03-01"
        }
    ],
    "yd_arr": [
        {
            "yd_id": 1,
            "tk_id": 1,
            "tk_name": "票务名称",
            "day_time": "2026-03-01",
            "status": 1,
            "ctime": "2026-02-22 14:30:00"
        }
    ]
}
```

### 6.4 操作类型

| 编号 | 枚举值 | 功能 | 说明 |
|------|--------|------|------|
| 1 | DL | 登录 | 验证手机号和密码 |
| 2 | ZC | 注册 | 创建新用户 |
| 3 | CKYY | 查看预约 | 获取所有可预约票务 |
| 4 | YD | 预定 | 预约指定票务 |
| 5 | WDYY | 我的预约 | 获取当前用户预约列表 |
| 6 | QXYY | 取消预约 | 取消指定预约 |
| 7 | TC | 退出 | 断开连接 |

---

## 七、数据库设计

### 7.1 数据库概览

数据库名：`2501db`

包含三张表：
- `user_info`：用户信息表
- `ticket_table`：票务表
- `yd_table`：预约表

### 7.2 表结构详解

#### 7.2.1 user_info（用户信息表）

| 字段名 | 类型 | 约束 | 说明 |
|--------|------|------|------|
| Tel | VARCHAR(11) | PRIMARY KEY | 手机号 |
| Name | VARCHAR(50) | NOT NULL | 用户名 |
| Passwd | VARCHAR(32) | NOT NULL | 密码（MD5加密） |
| Salt | VARCHAR(16) | NOT NULL | 加密盐值 |
| Status | INT | DEFAULT 1 | 状态（1:正常 0:禁用） |
| CreateDate | DATETIME | DEFAULT CURRENT_TIMESTAMP | 创建时间 |

**建表语句**：
```sql
CREATE TABLE user_info (
    Tel VARCHAR(11) PRIMARY KEY,
    Name VARCHAR(50) NOT NULL,
    Passwd VARCHAR(32) NOT NULL,
    Salt VARCHAR(16) NOT NULL,
    Status INT DEFAULT 1,
    CreateDate DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

#### 7.2.2 ticket_table（票务表）

| 字段名 | 类型 | 约束 | 说明 |
|--------|------|------|------|
| tk_id | INT | PRIMARY KEY AUTO_INCREMENT | 票务ID |
| tk_name | VARCHAR(100) | NOT NULL | 票务名称 |
| tk_max | INT | NOT NULL | 最大数量 |
| tk_count | INT | NOT NULL | 剩余数量 |
| day_time | DATE | NOT NULL | 日期 |

**建表语句**：
```sql
CREATE TABLE ticket_table (
    tk_id INT PRIMARY KEY AUTO_INCREMENT,
    tk_name VARCHAR(100) NOT NULL,
    tk_max INT NOT NULL,
    tk_count INT NOT NULL,
    day_time DATE NOT NULL
);
```

#### 7.2.3 yd_table（预约表）

| 字段名 | 类型 | 约束 | 说明 |
|--------|------|------|------|
| yd_id | INT | PRIMARY KEY AUTO_INCREMENT | 预约ID |
| tel | VARCHAR(11) | FOREIGN KEY | 用户手机号 |
| tk_id | INT | FOREIGN KEY | 票务ID |
| ctime | DATETIME | DEFAULT CURRENT_TIMESTAMP | 预约时间 |
| status | INT | DEFAULT 1 | 状态（1:有效 0:已取消） |

**建表语句**：
```sql
CREATE TABLE yd_table (
    yd_id INT PRIMARY KEY AUTO_INCREMENT,
    tel VARCHAR(11),
    tk_id INT,
    ctime DATETIME DEFAULT CURRENT_TIMESTAMP,
    status INT DEFAULT 1,
    FOREIGN KEY (tel) REFERENCES user_info(Tel),
    FOREIGN KEY (tk_id) REFERENCES ticket_table(tk_id)
);
```

### 7.3 ER 关系图

<img width="1329" height="1183" alt="关系图" src="https://github.com/user-attachments/assets/cbe05bb2-a920-4925-834c-e2fd774e7ddf" />

---

## 八、配置文件说明

### 8.1 服务器配置文件（my.conf）

```ini
# 网络配置
ip=127.0.0.1
port=6000
lismax=4096

# 线程池配置
threadnum=4
taskmax=1024

# 数据库配置
db_ip=127.0.0.1
db_port=3306
db_name=2501db
db_username=root
db_passwd=123456

# 连接池配置
pool_min_size=5
pool_max_size=20

# 日志配置
log_dir=logs
log_enable=1
log_level=INFO
```

### 8.2 配置项说明

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `ip` | 127.0.0.1 | 服务器监听 IP |
| `port` | 6000 | 服务器监听端口 |
| `lismax` | 4096 | 监听队列大小（backlog） |
| `threadnum` | 4 | 线程池工作线程数 |
| `taskmax` | 1024 | 任务队列最大容量 |
| `db_ip` | 127.0.0.1 | MySQL 服务器地址 |
| `db_port` | 3306 | MySQL 服务器端口 |
| `db_name` | 2501db | 数据库名称 |
| `db_username` | root | 数据库用户名 |
| `db_passwd` | 123456 | 数据库密码 |
| `pool_min_size` | 5 | 连接池最小连接数 |
| `pool_max_size` | 20 | 连接池最大连接数 |
| `log_dir` | logs | 日志文件目录 |
| `log_enable` | 1 | 是否启用日志（1:启用 0:禁用） |
| `log_level` | INFO | 日志级别（TRACE/DEBUG/INFO/WARN/ERROR/FATAL） |

---

## 九、编译与运行

### 9.1 环境依赖

```bash
# Ubuntu/Debian
sudo apt-get install g++ make libjsoncpp-dev libmysqlclient-dev libssl-dev

# CentOS/RHEL
sudo yum install gcc-c++ make jsoncpp-devel mysql-devel openssl-devel
```

### 9.2 编译步骤

#### 编译日志库
```bash
cd LogFile
make clean
make
```

#### 编译服务端
```bash
cd server
make clean
make
```

#### 编译客户端
```bash
cd client
make clean
make
```

### 9.3 运行步骤

#### 1. 启动 MySQL 服务
```bash
sudo systemctl start mysql
```

#### 2. 创建数据库和表
```bash
mysql -u root -p
```

```sql
CREATE DATABASE 2501db;
USE 2501db;

-- 创建用户表
CREATE TABLE user_info (
    Tel VARCHAR(11) PRIMARY KEY,
    Name VARCHAR(50) NOT NULL,
    Passwd VARCHAR(32) NOT NULL,
    Salt VARCHAR(16) NOT NULL,
    Status INT DEFAULT 1,
    CreateDate DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 创建票务表
CREATE TABLE ticket_table (
    tk_id INT PRIMARY KEY AUTO_INCREMENT,
    tk_name VARCHAR(100) NOT NULL,
    tk_max INT NOT NULL,
    tk_count INT NOT NULL,
    day_time DATE NOT NULL
);

-- 创建预约表
CREATE TABLE yd_table (
    yd_id INT PRIMARY KEY AUTO_INCREMENT,
    tel VARCHAR(11),
    tk_id INT,
    ctime DATETIME DEFAULT CURRENT_TIMESTAMP,
    status INT DEFAULT 1,
    FOREIGN KEY (tel) REFERENCES user_info(Tel),
    FOREIGN KEY (tk_id) REFERENCES ticket_table(tk_id)
);

-- 插入测试数据
INSERT INTO ticket_table (tk_name, tk_max, tk_count, day_time) VALUES
('电影票-A座', 100, 50, '2026-03-01'),
('演唱会门票', 200, 100, '2026-03-15'),
('火车票-北京', 500, 300, '2026-04-01');
```

#### 4. 启动服务器
```bash
cd server
./server
```

或使用一键脚本：
```bash
cd server
chmod +x run.sh
./run.sh
```

#### 5. 启动客户端
```bash
cd client
./client
```

或使用一键脚本：
```bash
cd client
chmod +x run.sh
./run.sh
```

### 9.4 使用示例

```
┌─────────────────────────────────────┐
│      欢迎使用票务预约系统            │
├─────────────────────────────────────┤
│  1. 登录                            │
│  2. 注册                            │
│  3. 退出                            │
└─────────────────────────────────────┘
请输入选项: 2

请输入手机号: 13800138000
请输入用户名: testuser
请输入密码: 123456
注册成功！

请输入选项: 1

请输入手机号: 13800138000
请输入密码: 123456
登录成功！欢迎 testuser

┌─────────────────────────────────────┐
│         主菜单 [testuser]           │
├─────────────────────────────────────┤
│  1. 查看票务                        │
│  2. 预约票务                        │
│  3. 我的预约                        │
│  4. 取消预约                        │
│  5. 退出登录                        │
└─────────────────────────────────────┘
请输入选项: 1

┌────┬─────────────────┬────────┬────────┬────────────┐
│ ID │    票务名称      │  总数  │  剩余   │    日期    │
├────┼─────────────────┼────────┼────────┼────────────┤
│  1 │ 电影票-A座       │   100  │    50  │ 2026-03-01 │
│  2 │ 演唱会门票       │   200  │   100  │ 2026-03-15 │
│  3 │ 火车票-北京      │   500  │   300  │ 2026-04-01 │
└────┴─────────────────┴────────┴────────┴────────────┘
```

---

## 十、设计模式应用

### 10.1 单例模式

**应用场景**：全局唯一的配置管理、日志管理、连接池管理

**实现方式**：
1. **双重检查锁**（SerConf）：线程安全且高效
2. **Meyers' Singleton**（ConnectionPool）：利用静态局部变量保证线程安全

```cpp
// 双重检查锁实现
class SerConf {
public:
    static SerConf& getInstance() {
        if (instance_ == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (instance_ == nullptr) {
                instance_ = new SerConf();
            }
        }
        return *instance_;
    }

private:
    static SerConf* instance_;
    static std::mutex mutex_;
};

// Meyers' Singleton 实现
class ConnectionPool {
public:
    static ConnectionPool& getInstance() {
        static ConnectionPool instance;
        return instance;
    }
};
```

### 10.2 RAII（资源获取即初始化）

**应用场景**：日志器、锁管理、智能指针

**关键特性**：
- 析构时自动释放资源
- 异常安全

```cpp
// Logger 的 RAII 应用
class Logger {
public:
    Logger(LogLevel level, const char* file, int line, const char* func)
        : impl_(level, file, line, func) {}

    ~Logger() {
        // 析构时自动输出日志
        // ...
    }
};

// 使用宏接口
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
    Logger(Logger::INFO, __FILE__, __LINE__, __func__).stream()
```

### 10.3 Builder 模式

**应用场景**：LogStarter 链式配置

```cpp
logStarter()
    .setBasename("server")
    .setLogDirectory("logs")
    .setRollSize(10 * 1024 * 1024)
    .setLogLevel(Logger::INFO)
    .start();
```

### 10.4 多态

**应用场景**：Socket 层次结构

```cpp
class Socket {
public:
    virtual void Handle_Data() = 0;  // 纯虚函数
};

class LisSocket : public Socket {
public:
    void Handle_Data() override {
        // 处理 accept 新连接
    }
};

class ConSocket : public Socket {
public:
    void Handle_Data() override {
        // 处理业务逻辑
    }
};
```

### 10.5 生产者-消费者

**应用场景**：AsynLogging 异步日志

```
前端线程（生产者）→ currentBuffer_ → buffers_ 队列 → 后台线程（消费者）
```

### 10.6 连接池模式

**应用场景**：ConnectionPool 数据库连接复用

```
获取连接 → 队列取连接 → 使用连接 → 释放连接 → 归还队列
```

### 10.7 回调函数

**应用场景**：Logger 自定义输出

```cpp
// 设置自定义输出函数
Logger::setOutput([](const char* msg, size_t len) {
    // 自定义输出逻辑
});

// 设置自定义刷新函数
Logger::setFlush([]() {
    // 自定义刷新逻辑
});
```

---

## 十一、技术亮点

### 11.1 高性能网络 I/O

- **epoll + EPOLLONESHOT**：事件驱动，支持高并发
- **非阻塞 socket**：避免线程阻塞
- **线程池**：解耦网络事件与业务处理

### 11.2 异步日志系统

- **双缓冲机制**：前端写入与后台落盘分离
- **日志滚动**：按文件大小、按天滚动
- **自动清理**：删除过期日志文件
- **多级别控制**：TRACE/DEBUG/INFO/WARN/ERROR/FATAL

### 11.3 数据库连接池

- **连接复用**：减少连接建立/销毁开销
- **自动检测**：连接归还时自动 ping 检测有效性
- **线程安全**：条件变量 + mutex 保证并发安全

### 11.4 安全性

- **MD5 加密**：密码加密存储
- **事务保障**：数据库事务保证操作原子性
- **输入验证**：手机号、密码格式验证

### 11.5 用户体验

- **丰富的命令行 UI**：表格化显示、emoji 状态提示
- **友好的错误提示**：详细的错误信息
- **流畅的交互**：响应迅速，体验良好

---

## 十二、API 接口文档

### 12.1 登录接口

**请求**：
```json
{
    "type": 1,
    "user_tel": "13800138000",
    "user_passwd": "e10adc3949ba59abbe56e057f20f883e"
}
```

**响应（成功）**：
```json
{
    "status": "OK",
    "msg": "登录成功",
    "user_name": "testuser"
}
```

**响应（失败）**：
```json
{
    "status": "ERR",
    "msg": "手机号或密码错误"
}
```

### 12.2 注册接口

**请求**：
```json
{
    "type": 2,
    "user_tel": "13800138000",
    "user_passwd": "e10adc3949ba59abbe56e057f20f883e",
    "user_name": "testuser"
}
```

**响应（成功）**：
```json
{
    "status": "OK",
    "msg": "注册成功"
}
```

**响应（失败）**：
```json
{
    "status": "ERR",
    "msg": "该手机号已注册"
}
```

### 12.3 查看票务接口

**请求**：
```json
{
    "type": 3
}
```

**响应**：
```json
{
    "status": "OK",
    "ticket_arr": [
        {
            "tk_id": 1,
            "tk_name": "电影票-A座",
            "tk_max": 100,
            "tk_count": 50,
            "day_time": "2026-03-01"
        }
    ]
}
```

### 12.4 预约票务接口

**请求**：
```json
{
    "type": 4,
    "user_tel": "13800138000",
    "ticket_id": "1"
}
```

**响应（成功）**：
```json
{
    "status": "OK",
    "msg": "预约成功"
}
```

**响应（失败）**：
```json
{
    "status": "ERR",
    "msg": "票务余量不足"
}
```

### 12.5 我的预约接口

**请求**：
```json
{
    "type": 5,
    "user_tel": "13800138000"
}
```

**响应**：
```json
{
    "status": "OK",
    "yd_arr": [
        {
            "yd_id": 1,
            "tk_id": 1,
            "tk_name": "电影票-A座",
            "day_time": "2026-03-01",
            "status": 1,
            "ctime": "2026-02-22 14:30:00"
        }
    ]
}
```

### 12.6 取消预约接口

**请求**：
```json
{
    "type": 6,
    "user_tel": "13800138000",
    "yd_id": "1"
}
```

**响应（成功）**：
```json
{
    "status": "OK",
    "msg": "取消成功"
}
```

**响应（失败）**：
```json
{
    "status": "ERR",
    "msg": "预约不存在或已取消"
}
```

---

## 十三、常见问题

### Q1: 编译时找不到头文件

**解决方案**：
```bash
# 安装依赖库
sudo apt-get install libjsoncpp-dev libmysqlclient-dev libssl-dev

# 或指定头文件路径
g++ -I/path/to/headers ...
```

### Q2: 连接 MySQL 失败

**解决方案**：
1. 检查 MySQL 服务是否启动：`sudo systemctl status mysql`
2. 检查配置文件中的数据库连接信息
3. 检查 MySQL 用户权限

### Q3: 端口被占用

**解决方案**：
```bash
# 查找占用端口的进程
lsof -i:6000

# 终止进程
kill -9 <PID>
```

### Q4: 日志文件不生成

**解决方案**：
1. 检查 `my.conf` 中 `log_enable=1`
2. 检查日志目录权限
3. 检查磁盘空间

### Q5: 客户端连接超时

**解决方案**：
1. 检查服务器是否启动
2. 检查网络连接
3. 检查防火墙设置

---
## 十四、Redis 缓存方案分析

### 14.1 背景与动机

在项目开发过程中，曾考虑引入 **Redis** 作为缓存中间件来优化系统性能。票务预约系统存在以下特点，使得 Redis 成为一个值得讨论的技术选项：

- **热点数据频繁读取**：票务信息（剩余数量、日期等）被大量客户端反复查询
- **库存扣减的原子性**：多用户并发预约同一票务时，需要保证不超卖
- **用户会话管理**：登录状态需要在多次请求间维持

最终经过综合评估，**本项目未引入 Redis**，原因及详细分析如下。

---

### 14.2 引入 Redis 的优势分析

#### 14.2.1 缓存热点数据，减轻 MySQL 压力

```
当前方案（无 Redis）:
Client → Server → MySQL（每次查询直接查库）

引入 Redis 后:
Client → Server → Redis（缓存命中） → 直接返回
                              ↓（缓存未命中）
                        MySQL → 写入 Redis → 返回
```

**具体收益**：
- 票务信息查询（操作类型 3）是高频操作，缓存后可将响应时间从 **毫秒级降低到微秒级**
- MySQL 连接池压力大幅下降，有限的连接可以留给写操作（预约、取消）
- 对于读多写少的票务查询场景，**Redis 内存读取速度是 MySQL 磁盘查询的 10-100 倍**

#### 14.2.2 分布式锁防止超卖

票务预约的核心问题是 **并发扣减库存**。Redis 提供了成熟的分布式锁方案：

```cpp
// 方案一：Redis SETNX 实现分布式锁
// 获取锁
SET lock:ticket:1 <unique_id> NX EX 10

// 扣减库存（Lua 脚本保证原子性）
if redis.call('get', KEYS[1]) == ARGV[1] then
    local stock = tonumber(redis.call('get', KEYS[2]))
    if stock > 0 then
        redis.call('decr', KEYS[2])
        return 1
    end
    return 0
end

// 释放锁
if redis.call('get', KEYS[1]) == ARGV[1] then
    return redis.call('del', KEYS[1])
end

// 方案二：Redis 原子操作（更推荐）
// 使用 Lua 脚本，单线程执行，天然原子
```

**对比当前方案**：

| 对比维度 | 当前方案（MySQL事务） | Redis + MySQL 方案 |
|----------|----------------------|-------------------|
| 原子性保障 | `SELECT ... FOR UPDATE` 行锁 | Redis Lua 原子脚本 |
| 并发性能 | 行锁竞争，高并发下排队等待 | 内存操作，无锁竞争 |
| 实现复杂度 | 低（纯 SQL 事务） | 中（需要双写一致性） |
| 数据可靠性 | 高（MySQL 事务 ACID） | 中（需要持久化策略） |

#### 14.2.3 用户会话与令牌管理

```cpp
// 使用 Redis 管理用户登录状态
// 登录成功后
redis.setex("session:13800138000", 3600, "testuser");  // 1小时过期

// 后续请求验证
std::string session = redis.get("session:13800138000");
if (session.empty()) {
    // 会话过期，要求重新登录
}

// 退出时删除
redis.del("session:13800138000");
```

**收益**：
- 服务端无需在内存中维护用户会话状态
- 支持会话自动过期（TTL 机制）
- 如果未来扩展为多实例部署，天然支持会话共享

#### 14.2.4 热门票务预加载

```cpp
// 定时将票务数据同步到 Redis
void syncTicketToRedis() {
    // 从 MySQL 查询所有票务
    auto tickets = mysqlClient->query("SELECT * FROM ticket_table");
    for (auto& ticket : tickets) {
        // 写入 Redis Hash
        redis.hset("ticket:" + ticket.tk_id, "name", ticket.tk_name);
        redis.hset("ticket:" + ticket.tk_id, "max", ticket.tk_max);
        redis.hset("ticket:" + ticket.tk_id, "count", ticket.tk_count);
        redis.hset("ticket:" + ticket.tk_id, "day", ticket.day_time);
        // 设置过期时间，保证最终一致性
        redis.expire("ticket:" + ticket.tk_id, 300);  // 5分钟
    }
}
```

---

### 14.3 引入 Redis 的劣势分析

#### 14.3.1 系统复杂度显著增加

**架构对比**：

<img width="1331" height="1181" alt="架构图" src="https://github.com/user-attachments/assets/02453c10-f332-42c9-b52a-a43fa48ccdb2" />


**复杂度增加的具体体现**：

| 方面 | 无 Redis | 有 Redis |
|------|---------|---------|
| 启动流程 | 启动 MySQL → 启动 Server | 启动 MySQL → 启动 Redis → 启动 Server |
| 故障排查 | 只排查 Server + MySQL | 需额外排查 Redis 连接、内存、持久化 |
| 配置管理 | my.conf 一个文件 | my.conf + redis.conf 两个文件 |
| 代码量 | 纯业务代码 | 需要 Redis 客户端封装、缓存策略代码 |
| 部署运维 | 只需部署一个数据库 | 需要部署和监控两个中间件 |

#### 14.3.2 数据一致性问题

引入缓存后，必须处理 **MySQL 与 Redis 的数据一致性**：

```
问题场景：用户预约了票务
1. Redis 中库存 -1          ← 成功
2. MySQL 中库存 -1          ← 失败（网络异常）
3. 结果：Redis 和 MySQL 数据不一致！
```

**需要解决的一致性问题**：

```
  1. 缓存与数据库双写顺序                                 
     - 先更新数据库，再更新缓存（推荐）                     
     - 先更新缓存，再更新数据库（容易不一致）               
     - 先删除缓存，再更新数据库（存在脏读窗口）              
                                                         
  2. 缓存穿透（查询不存在的票务）                          
     - 解决方案：布隆过滤器 / 空值缓存                     
                                                         
  3. 缓存雪崩（大量缓存同时过期）                          
     - 解决方案：过期时间加随机偏移量                       
                                                         
  4. 缓存击穿（热点 key 过期瞬间）                        
     - 解决方案：互斥锁 / 永不过期 + 异步更新              

```

**应对方案示例**：

```cpp
// 方案：先更新数据库，再删除缓存（延迟双删）
bool bookTicket(const std::string& tel, int ticketId) {
    // 1. Redis 预扣减（快速失败）
    if (!redis.decrIfPositive("ticket:stock:" + ticketId)) {
        return false;  // 库存不足，直接返回
    }

    // 2. 数据库事务处理
    if (!mysqlClient->Begin()) {
        redis.incr("ticket:stock:" + ticketId);  // 回滚 Redis
        return false;
    }

    // 扣减库存 + 插入预约记录
    if (!mysqlClient->bookTicket(tel, ticketId)) {
        mysqlClient->RollBack();
        redis.incr("ticket:stock:" + ticketId);  // 回滚 Redis
        return false;
    }

    mysqlClient->Commit();

    // 3. 延迟删除缓存（确保数据库已提交）
    usleep(500 * 1000);  // 500ms 延迟
    redis.del("ticket:stock:" + ticketId);

    return true;
}
```

#### 14.3.3 资源消耗增加

| 资源 | 无 Redis | 有 Redis |
|------|---------|---------|
| 内存占用 | Server ~50MB + MySQL | Server ~50MB + MySQL + Redis ~100-200MB |
| 端口占用 | 6000 (Server) + 3306 (MySQL) | 6000 + 3306 + 6379 |
| 进程数量 | Server 1个 + MySQL 1个 | Server + MySQL + Redis = 3个 |
| 系统依赖 | libmysqlclient, libjsoncpp | + hiredis / redis-plus-plus |

对于单机小项目来说，这些额外开销是 **不成比例的**。

#### 14.3.4 增加运维负担

```
无 Redis 的运维:
✅ Server 崩溃 → 重启 Server 即可
✅ MySQL 崩溃 → 重启 MySQL 即可

有 Redis 的运维:
⚠️ Server 崩溃 → 重启 Server
⚠️ MySQL 崩溃 → 重启 MySQL + 检查 Redis 缓存是否脏数据
⚠️ Redis 崩溃 → 重启 Redis + 热数据预热 + 检查数据一致性
⚠️ Redis 内存满了 → 排查 + 清理 + 调整 maxmemory
⚠️ Redis 持久化失败 → 排查 AOF/RDB 配置
```

---

### 14.4 最终决策：不引入 Redis

#### 14.4.1 决策依据

| 评估维度 | 评分（1-5） | 说明 |
|----------|:-----------:|------|
| 性能收益 | ⭐⭐⭐ | 有提升，但单机场景下 MySQL 连接池 + epoll 已经够用 |
| 架构匹配度 | ⭐⭐ | 单机单实例项目，Redis 分布式优势无法体现 |
| 复杂度代价 | ⭐⭐⭐⭐ | 引入后需处理缓存一致性、故障恢复等问题 |
| 运维成本 | ⭐⭐⭐⭐ | 多一个中间件需要维护和监控 |
| 技术必要性 | ⭐ | 当前并发量和数据量不需要 Redis 来支撑 |
| **综合评分** | **2.4 / 5** | **不建议引入** |

#### 14.4.2 替代方案（当前项目已采用）

通过以下方案，在不引入 Redis 的情况下达到较好的性能：

```

              当前项目的性能优化策略
            
  1. MySQL 连接池                                         
     - 最小 5 / 最大 20 连接                              
     - 连接复用，减少建立/销毁开销                         
     - 自动 ping 检测连接有效性                           
                                                         
  2. epoll + 线程池                                      
     - epoll 事件驱动，处理万级并发连接                    
     - 线程池（4线程）解耦 I/O 与业务处理                  
     - EPOLLONESHOT 防止重复触发                         
                                                         
  3. 数据库事务优化                                       
     - SELECT ... FOR UPDATE 行锁保证原子性              
     - 事务内完成 扣库存 + 插记录                         
     - 事务失败自动回滚                                   
                                                         
  4. 异步日志                                             
     - 双缓冲机制，日志写入不影响业务                     
     - 后台线程异步落盘                                   

```

**关键 SQL 优化（行锁防超卖）**：
```sql
-- 使用 SELECT ... FOR UPDATE 加行锁，防止并发超卖
BEGIN;
SELECT tk_count FROM ticket_table WHERE tk_id = 1 FOR UPDATE;
-- 应用层判断 tk_count > 0
UPDATE ticket_table SET tk_count = tk_count - 1 WHERE tk_id = 1;
INSERT INTO yd_table (tel, tk_id, ctime, status) VALUES ('13800138000', 1, NOW(), 1);
COMMIT;
```

---

### 14.5 未来扩展：何时应该引入 Redis

当项目满足以下条件时，建议引入 Redis：

```
引入 Redis 的触发条件:
┌─────────────────────────────────────────────────────────┐
│                                                         │
│  □ 并发量突破 1000 QPS                                   │
│    → MySQL 连接池成为瓶颈                                 │
│                                                         │
│  □ 部署多台服务器（分布式部署）                            │
│    → 需要分布式缓存和会话共享                              │
│                                                         │
│  □ 热门票务查询占总请求 > 70%                             │
│    → 缓存命中率高，引入 Redis 收益明显                     │
│                                                         │
│  □ 需要实现秒杀/抢票功能                                  │
│    → Redis 原子操作 + Lua 脚本天然适合                    │
│                                                         │
│  □ 需要消息队列（订单异步处理等）                          │
│    → Redis List / Stream 可做轻量队列                    │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

**推荐的渐进式引入路径**：

```
阶段 1（当前）: 纯 MySQL + 连接池
    ↓
阶段 2: Redis 做只读缓存（票务信息查询）
    ↓
阶段 3: Redis 做分布式锁（库存扣减）
    ↓
阶段 4: Redis 做会话管理 + 消息队列
    ↓
阶段 5: Redis Cluster 集群部署
```

---

### 14.6 Redis 集成代码参考（预留）

虽然当前项目未引入 Redis，但预留了集成方案供未来扩展参考：

```cpp
// Redis 客户端封装（基于 hiredis）
class RedisClient {
public:
    static RedisClient& getInstance();

    bool connect(const std::string& ip, int port);
    void disconnect();

    // String 操作
    bool set(const std::string& key, const std::string& value);
    bool setex(const std::string& key, int expire, const std::string& value);
    std::string get(const std::string& key);

    // Hash 操作
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::string hget(const std::string& key, const std::string& field);

    // 原子操作
    bool decrIfPositive(const std::string& key);
    bool setnx(const std::string& key, const std::string& value, int expire);

    // Lua 脚本执行
    std::string eval(const std::string& script, const std::vector<std::string>& keys,
                     const std::vector<std::string>& args);

private:
    RedisClient();
    redisContext* context_;
    std::mutex mutex_;
};

// ---- 票务缓存封装 ----
class TicketCache {
public:
    // 查询票务（优先从 Redis 读取）
    bool getTicket(int ticketId, Json::Value& ticket) {
        // 1. 尝试从 Redis 获取
        std::string cached = RedisClient::getInstance().hget("ticket:" + std::to_string(ticketId), "name");
        if (!cached.empty()) {
            // 缓存命中
            ticket["tk_name"] = cached;
            ticket["tk_max"] = RedisClient::getInstance().hget("ticket:" + std::to_string(ticketId), "max");
            ticket["tk_count"] = RedisClient::getInstance().hget("ticket:" + std::to_string(ticketId), "count");
            return true;
        }

        // 2. 缓存未命中，查询 MySQL
        if (!mysqlClient->queryTicket(ticketId, ticket)) {
            return false;
        }

        // 3. 回写 Redis 缓存
        RedisClient::getInstance().hset("ticket:" + std::to_string(ticketId), "name", ticket["tk_name"].asString());
        RedisClient::getInstance().hset("ticket:" + std::to_string(ticketId), "max", ticket["tk_max"].asString());
        RedisClient::getInstance().hset("ticket:" + std::to_string(ticketId), "count", ticket["tk_count"].asString());
        return true;
    }

    // 扣减库存（Redis Lua 原子操作）
    bool decrementStock(int ticketId) {
        // Lua 脚本：原子性检查并扣减
        static const char* script = R"(
            local stock = tonumber(redis.call('get', KEYS[1]))
            if stock and stock > 0 then
                redis.call('decr', KEYS[1])
                return 1
            end
            return 0
        )";
        std::string result = RedisClient::getInstance().eval(
            script,
            {"ticket:stock:" + std::to_string(ticketId)},
            {}
        );
        return result == "1";
    }
};
```

```conf
# Redis 配置项（预留，添加到 my.conf）
redis_ip=127.0.0.1
redis_port=6379
redis_passwd=
redis_db=0
```

---

### 14.7 总结

| 维度 | 结论 |
|------|------|
| **当前是否需要 Redis** | ❌ 不需要。单机项目，MySQL 连接池 + epoll + 线程池已足够 |
| **引入 Redis 是否值得** | ❌ 不值得。复杂度增加远大于性能收益 |
| **Redis 有何优势** | ✅ 缓存热点数据、原子扣减库存、会话管理、分布式锁 |
| **Redis 有何劣势** | ❌ 数据一致性复杂、运维负担增加、资源消耗额外 |
| **未来是否可能引入** | ✅ 会。当项目扩展到多实例或需要秒杀功能时，Redis 是必要的 |
| **最佳策略** | 📋 当前保持简洁架构，预留 Redis 集成接口，按需引入 |

> **核心原则**：技术选型应该匹配项目规模。单机票务系统使用 MySQL 连接池已经能很好地解决问题，引入 Redis 属于"过度设计"。当业务规模增长到需要分布式能力时，再引入 Redis 才是正确的时机。

---

## 附录

### A. 编译选项说明

```bash
# 服务端编译命令
g++ -std=c++14 -o server server.cpp ConnectionPool.cpp \
    -I/path/to/logfile/include \
    -L/path/to/logfile \
    -llogfile -lpthread -ljsoncpp -lmysqlclient -lcrypto

# 客户端编译命令
g++ -std=c++14 -o client client.cpp \
    -ljsoncpp -lcrypto
```

### B. 日志级别说明

| 级别 | 说明 | 使用场景 |
|------|------|----------|
| TRACE | 追踪 | 最详细的日志，用于调试 |
| DEBUG | 调试 | 调试信息 |
| INFO | 信息 | 一般信息 |
| WARN | 警告 | 警告信息 |
| ERROR | 错误 | 错误信息 |
| FATAL | 致命 | 致命错误，程序即将退出 |
