#ifndef CONNECTION_POOL_HPP
#define CONNECTION_POOL_HPP

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

class ConnectionPool {
public:
    // 获取连接池单例
    static ConnectionPool& getInstance();

    // 禁止拷贝和赋值
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    // 初始化连接池
    bool init(const std::string& host, int port, 
              const std::string& username, const std::string& password, 
              const std::string& database, int min_size = 5, int max_size = 20);

    // 获取连接
    MYSQL* getConnection();

    // 归还连接
    void releaseConnection(MYSQL* conn);

    // 获取当前空闲连接数
    int getIdleCount();

    // 获取当前活跃连接数
    int getActiveCount();

    // 销毁连接池
    void destroy();

private:
    ConnectionPool();
    ~ConnectionPool();

    // 创建单个连接
    MYSQL* createConnection();

    // 销毁单个连接
    void destroyConnection(MYSQL* conn);

    // 连接池配置
    std::string host_;
    int port_;
    std::string username_;
    std::string password_;
    std::string database_;

    // 连接池大小配置
    int min_size_;
    int max_size_;

    // 连接队列
    std::queue<MYSQL*> connection_queue_;

    // 线程同步
    std::mutex mutex_;
    std::condition_variable cond_;

    // 连接计数
    std::atomic<int> active_count_;
    std::atomic<int> total_count_;

    // 运行标志
    std::atomic<bool> running_;
};

#endif // CONNECTION_POOL_HPP