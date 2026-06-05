#include "ConnectionPool.hpp"
#include <iostream>

ConnectionPool::ConnectionPool() 
    : host_("127.0.0.1"), port_(3306), min_size_(5), max_size_(20),
      active_count_(0), total_count_(0), running_(false) {
}

ConnectionPool::~ConnectionPool() {
    destroy();
}

ConnectionPool& ConnectionPool::getInstance() {
    static ConnectionPool instance;
    return instance;
}

bool ConnectionPool::init(const std::string& host, int port,
                          const std::string& username, const std::string& password,
                          const std::string& database, int min_size, int max_size) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (running_) {
        std::cout << "[WARN] 连接池已经初始化" << std::endl;
        return true;
    }

    host_ = host;
    port_ = port;
    username_ = username;
    password_ = password;
    database_ = database;
    min_size_ = min_size;
    max_size_ = max_size;

    // 创建初始连接
    for (int i = 0; i < min_size_; ++i) {
        MYSQL* conn = createConnection();
        if (conn) {
            connection_queue_.push(conn);
            total_count_++;
        }
    }

    running_ = true;
    std::cout << "[INFO] 连接池初始化完成，初始连接数: " << total_count_ << std::endl;
    return true;
}

MYSQL* ConnectionPool::createConnection() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "[ERROR] MySQL初始化失败" << std::endl;
        return nullptr;
    }

    // 设置连接选项
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    
    if (!mysql_real_connect(conn, host_.c_str(), username_.c_str(), 
                            password_.c_str(), database_.c_str(), port_, nullptr, 0)) {
        std::cerr << "[ERROR] MySQL连接失败: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return nullptr;
    }

    return conn;
}

void ConnectionPool::destroyConnection(MYSQL* conn) {
    if (conn) {
        mysql_close(conn);
    }
}

MYSQL* ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);

    if (!running_) {
        std::cerr << "[ERROR] 连接池未初始化" << std::endl;
        return nullptr;
    }

    // 如果有空闲连接，直接返回
    if (!connection_queue_.empty()) {
        MYSQL* conn = connection_queue_.front();
        connection_queue_.pop();
        active_count_++;
        return conn;
    }

    // 如果还能创建新连接，创建一个
    if (total_count_ < max_size_) {
        MYSQL* conn = createConnection();
        if (conn) {
            total_count_++;
            active_count_++;
            return conn;
        }
    }

    // 等待有连接可用
    cond_.wait(lock);

    if (!connection_queue_.empty()) {
        MYSQL* conn = connection_queue_.front();
        connection_queue_.pop();
        active_count_++;
        return conn;
    }

    return nullptr;
}

void ConnectionPool::releaseConnection(MYSQL* conn) {
    if (!conn) return;

    std::lock_guard<std::mutex> lock(mutex_);

    // 检查连接是否仍然有效
    if (mysql_ping(conn) != 0) {
        // 连接已断开，销毁它
        destroyConnection(conn);
        total_count_--;
        active_count_--;
        
        // 尝试创建新连接补充
        if (total_count_ < min_size_) {
            MYSQL* new_conn = createConnection();
            if (new_conn) {
                connection_queue_.push(new_conn);
                total_count_++;
            }
        }
    } else {
        // 连接有效，放回队列
        connection_queue_.push(conn);
        active_count_--;
    }

    // 通知等待的线程
    cond_.notify_one();
}

int ConnectionPool::getIdleCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return connection_queue_.size();
}

int ConnectionPool::getActiveCount() {
    return active_count_.load();
}

void ConnectionPool::destroy() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!running_) return;

    while (!connection_queue_.empty()) {
        MYSQL* conn = connection_queue_.front();
        connection_queue_.pop();
        destroyConnection(conn);
        total_count_--;
    }

    running_ = false;
    cond_.notify_all();
    std::cout << "[INFO] 连接池已销毁" << std::endl;
}
