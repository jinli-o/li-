#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>

using namespace std;

const int EV_MAX = 128; // epoll_wait，一次最多返回128个就绪事件
enum OPTYPE
{
    DL = 1,
    ZC,
    CKYY,
    YD,
    WDYY,
    QXYY,
    TC
};
#include <mutex>

// 操作配置文件 - 单例模式（双重检查锁）
class SerConf
{
private:
    // 私有构造函数，防止外部实例化
    SerConf()
    {
        ips = "127.0.0.1";
        port = 6000;
        lismax = 1024;
        taskmax = 1024;
        threadnum = 4;
        db_ip = "127.0.0.1";
        db_port = 3306;
        db_name = "2501db";
        db_username = "root";
        db_passwd = "123456";
        pool_min_size = 5;
        pool_max_size = 20;
        log_dir = "logs";
        log_enable = 1;
        log_level = "INFO";
    }
    
    // 禁止拷贝和赋值
    SerConf(const SerConf&) = delete;
    SerConf& operator=(const SerConf&) = delete;
    
    // 静态实例指针
    static SerConf* instance;
    
    // 互斥锁
    static std::mutex mutex_;

public:
    // 获取单例实例（双重检查锁模式）
    static SerConf* getInstance();
    
    bool ReadConf(string filename);
    void PrintInfo();

    string ips;       // ip地址
    short port;       // 服务器端口
    int lismax;       // 监听队列大小
    int taskmax;      // 存放就绪事件的任务队列大小
    int threadnum;    // 线程数目

    string db_ip;     // 数据库IP
    short db_port;    // 数据库端口
    string db_name;   // 数据库名称
    string db_username; // 数据库用户名
    string db_passwd;  // 数据库密码

    int pool_min_size; // 连接池最小连接数
    int pool_max_size; // 连接池最大连接数

    string log_dir;   // 日志目录
    int log_enable;   // 是否启用日志
    string log_level; // 日志级别
};

class Socket;

// 任务--
typedef struct
{
    Socket *con;
} task_t;

// 线程池 （包含 任务队列）
class ThreadPool
{
public:
    ThreadPool(int num, int task_num) : max_queue(task_num), thread_num(num)
    {
    }

    bool Thread_Pool_Init();
    void Add_Task(Socket *csocket); // 向任务队列添加任务，
    void Start_Thread();            // 线程启动
    void Work();                    // 工作线程要做的事情

private:
    task_t *queue; // 任务队列
    int front;     // 队头指针
    int rear;      // 队尾指针
    int count;     // 当前任务数量

    int stop; // 线程是否退出

    int max_queue = 1024; // 任务队列的大小
    int thread_num = 4;   // 线程数目

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};
// 操作数据库的类
class MysqlClient
{
public:
    MysqlClient();
    ~MysqlClient();

    bool Connect_MysqlServer();
    bool Db_User_Register(const string &tel,const string & name, const string &passwd);
    bool Db_User_Login(const string &tel, string &name, const string &passwd);
    bool Db_Show_Ticket(Json::Value &res);
    bool Db_Yd_Ticket(string usertel, string tk_id);
    bool Db_My_Ticket(const string &usertel, Json::Value &res);
    bool Db_Cancel_Ticket(const string &usertel, const string &yd_id);
    bool Db_Cancel_Ticket_By_TkId(const string &usertel, const string &tk_id);

    void Mysql_Begin();//开启事物
    void Mysql_RollBack();//回滚事物
    void Mysql_Commit();//提交事物

private:
    MYSQL* mysql_conn_;  // 从连接池获取的连接
};

// 基类
class Socket
{
public:
    Socket(int fd, int epfd) : m_fd(fd), m_epfd(epfd)
    {
    }
    virtual ~Socket()
    {
        close(m_fd);
    }

    virtual void Handle_Data() = 0;

    int m_fd;   // socket描述符
    int m_epfd; // 属于那个epoll的内核事件表（红黑树)
};

// 派生类,处理监听套接字
class LisSocket : public Socket
{
public:
    LisSocket(int fd, int epfd) : Socket(fd, epfd)
    {
    }

    void Handle_Data(); // 接受客户端连接
private:
    void RestEvent(); // 重置事件，让epoll_wait继续触发事件
private:
    static int m_Count;
};
// 派生类，处理连接套接字
class ConSocket : public Socket
{
public:
    ConSocket(int fd, int epfd) : Socket(fd, epfd)
    {
        m_OpType = -1;
    }
    void Handle_Data(); // 接收客户端发送的数据
private:
    void RestEvent();
    void Get_OpType(char buff[]);
    void Send_Ok();
    void Send_Err();
    void Send_Err(const string &msg);
    void Send_JsonObj(Json::Value &root);

    void User_Register();
    void User_Login();
    void Show_Ticket();//查看预约信息
    void Yd_Ticket();//预定
    void My_Ticket();//我的预约
    void Cancel_Ticket();//取消预约

private:
    Json::Value m_val;
    int m_OpType; // 客户端操作类型
};
// tcp服务器
class TcpSer
{
public:
    TcpSer(SerConf *sc) : m_conf(sc), m_pool(sc->threadnum, sc->taskmax)
    {
        m_sockfd = -1;
        m_epfd = -1;
    }
    bool Ser_Init();
    void Run();

private:
    bool create_socket();
    void do_events();

private:
    SerConf* m_conf; // 配置文件信息（指针，避免拷贝）
    int m_sockfd;   // 服务器监听套接字

    int m_epfd; // 内核事件表,epoll
    int m_num;  // 记录就绪事件的个数,epoll_wait返回值
    struct epoll_event m_evs[EV_MAX];

    ThreadPool m_pool;
};

