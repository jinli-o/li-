
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
<img width="1311" height="1200" alt="项目一" src="https://github.com/user-attachments/assets/f8b4294e-a6e0-4259-870e-0615c1646157" />
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

```
┌─────────────────┐         ┌─────────────────┐
│    user_info     │         │  ticket_table    │
├─────────────────┤         ├─────────────────┤
│ Tel (PK)        │         │ tk_id (PK)      │
│ Name            │         │ tk_name         │
│ Passwd          │         │ tk_max          │
│ Salt            │         │ tk_count        │
│ Status          │         │ day_time        │
│ CreateDate      │         │                 │
└────────┬────────┘         └────────┬────────┘
         │                           │
         │         ┌─────────────────┘
         │         │
         ▼         ▼
    ┌─────────────────┐
    │    yd_table      │
    ├─────────────────┤
    │ yd_id (PK)      │
    │ tel (FK)        │
    │ tk_id (FK)      │
    │ ctime           │
    │ status          │
    └─────────────────┘
```

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
确认密码: 123456
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
│ ID │    票务名称      │  总数  │  剩余  │    日期    │
├────┼─────────────────┼────────┼────────┼────────────┤
│  1 │ 电影票-A座      │   100  │    50  │ 2026-03-01 │
│  2 │ 演唱会门票      │   200  │   100  │ 2026-03-15 │
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
