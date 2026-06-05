// ================= 日志系统启动器 =================
// 功能：封装日志系统的初始化和启动逻辑
// 特点：与业务代码低耦合，提供简洁的启动接口

#ifndef LOG_STARTER_HPP
#define LOG_STARTER_HPP

#include "LoggerManager.hpp"
#include "LogCommon.hpp"
#include <string>
#include <memory>
#include <iostream>

namespace logfile
{
    // ================= 日志配置结构体 =================
    struct LogConfig
    {
        std::string log_basename = "server";   // 日志文件名前缀
        std::string log_directory = "./logs";   // 日志存储目录
        int keep_days = 7;                      // 日志保留天数
        bool enable_cleaner = true;             // 是否启用日志清理器
        LOG_LEVEL log_level = LOG_LEVEL::INFO;  // 日志级别
    };

    // ================= 日志启动器类 =================
    class LogStarter
    {
    public:
        // 获取单例实例
        static LogStarter& getInstance() {
            static LogStarter instance;
            return instance;
        }

        // ================= 配置方法 =================
        
        // 设置日志基础配置
        LogStarter& setBasename(const std::string& basename) {
            config_.log_basename = basename;
            return *this;
        }

        LogStarter& setLogDirectory(const std::string& dir) {
            config_.log_directory = dir;
            return *this;
        }

        LogStarter& setKeepDays(int days) {
            config_.keep_days = days;
            return *this;
        }

        LogStarter& setCleanerEnabled(bool enable) {
            config_.enable_cleaner = enable;
            return *this;
        }

        LogStarter& setLogLevel(LOG_LEVEL level) {
            config_.log_level = level;
            return *this;
        }

        // 从字符串设置日志级别
        LogStarter& setLogLevel(const std::string& level_str) {
            if (level_str == "TRACE") config_.log_level = LOG_LEVEL::TRACE;
            else if (level_str == "DEBUG") config_.log_level = LOG_LEVEL::DEBUG;
            else if (level_str == "INFO") config_.log_level = LOG_LEVEL::INFO;
            else if (level_str == "WARN") config_.log_level = LOG_LEVEL::WARN;
            else if (level_str == "ERROR") config_.log_level = LOG_LEVEL::ERROR;
            else if (level_str == "FATAL") config_.log_level = LOG_LEVEL::FATAL;
            return *this;
        }

        // 批量设置配置
        LogStarter& configure(const LogConfig& config) {
            config_ = config;
            return *this;
        }

        // ================= 启动/停止方法 =================
        
        // 启动日志系统
        bool start() {
            if (started_) {
                LOG_WARN << "日志系统已经启动，无需重复启动";
                return true;
            }

            try {
                // 初始化日志管理器
                LoggerManager::getInstance().init(
                    config_.log_basename,
                    config_.keep_days,
                    config_.log_directory,
                    config_.enable_cleaner
                );

                // 设置日志级别
                LoggerManager::getInstance().setLevel(config_.log_level);

                // 启动日志系统
                LoggerManager::getInstance().start();

                started_ = true;
                
                LOG_INFO << "日志系统启动成功";
                LOG_INFO << "日志目录: " << config_.log_directory;
                LOG_INFO << "日志级别: " << logLevelToString(config_.log_level);
                LOG_INFO << "保留天数: " << config_.keep_days;
                
                return true;
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] 日志系统启动失败: " << e.what() << std::endl;
                return false;
            }
        }

        // 停止日志系统
        void stop() {
            if (!started_) {
                return;
            }

            LoggerManager::getInstance().stop();
            started_ = false;
            
            std::cout << "[INFO] 日志系统已停止" << std::endl;
        }

        // 检查是否已启动
        bool isStarted() const {
            return started_;
        }

        // 获取当前配置
        const LogConfig& getConfig() const {
            return config_;
        }

    private:
        // 私有构造函数（单例模式）
        LogStarter() : started_(false) {}
        ~LogStarter() = default;
        
        // 禁止拷贝和赋值
        LogStarter(const LogStarter&) = delete;
        LogStarter& operator=(const LogStarter&) = delete;

        // 将日志级别转换为字符串
        std::string logLevelToString(LOG_LEVEL level) {
            switch (level) {
                case LOG_LEVEL::TRACE: return "TRACE";
                case LOG_LEVEL::DEBUG: return "DEBUG";
                case LOG_LEVEL::INFO:  return "INFO";
                case LOG_LEVEL::WARN:  return "WARN";
                case LOG_LEVEL::ERROR: return "ERROR";
                case LOG_LEVEL::FATAL: return "FATAL";
                default: return "UNKNOWN";
            }
        }

        // 成员变量
        LogConfig config_;
        bool started_;
    };

    // ================= 便捷启动函数 =================
    
    // 快速启动日志系统（使用默认配置）
    inline bool startLogSystem() {
        return LogStarter::getInstance().start();
    }

    // 使用配置启动日志系统
    inline bool startLogSystem(const LogConfig& config) {
        return LogStarter::getInstance().configure(config).start();
    }

    // 链式配置启动
    inline LogStarter& logStarter() {
        return LogStarter::getInstance();
    }

} // namespace logfile

#endif // LOG_STARTER_HPP