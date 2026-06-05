#include "server.h"
#include <time.h>
#include <sys/stat.h>
#include <openssl/md5.h>

// 引入 LogFile 日志系统
#include "../LogFile/include/LogStarter.hpp"

// 引入连接池
#include "ConnectionPool.hpp"

// 单例静态成员初始化
SerConf* SerConf::instance = nullptr;
std::mutex SerConf::mutex_;

// 获取单例实例（双重检查锁模式）
SerConf* SerConf::getInstance() {
    // 第一次检查（不加锁）
    if (instance == nullptr) {
        // 加锁
        std::lock_guard<std::mutex> lock(mutex_);
        // 第二次检查（加锁后）
        if (instance == nullptr) {
            instance = new SerConf();
        }
    }
    return instance;
}

// 日志系统初始化函数
bool InitLogSystem() {
    SerConf* conf = SerConf::getInstance();
    return logfile::logStarter()
        .setBasename("server")
        .setLogDirectory(conf->log_dir)
        .setKeepDays(7)
        .setCleanerEnabled(true)
        .setLogLevel(conf->log_level)
        .start();
}

string md5_hash(const string &input)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)input.c_str(), input.length(), digest);
    
    char md5str[33];
    for(int i = 0; i < 16; i++)
        sprintf(&md5str[i*2], "%02x", (unsigned int)digest[i]);
    
    return string(md5str);
}

// ================= 日志函数包装（兼容原有代码） =================
void log_error(const string &msg)
{
    LOG_ERROR << msg;
}

void log_info(const string &msg)
{
    LOG_INFO << msg;
}

bool SerConf::ReadConf(string filename)
{
    log_info("Reading configuration file: " + filename);
    
    FILE *fp = fopen(filename.c_str(), "r");
    if (fp == NULL)
    {
        log_error("Failed to open configuration file: " + filename);
        cout << "open file: " << filename << " err!" << endl;
        return false;
    }

    char buff[128] = {0};
    int num = 1;
    while (fgets(buff, 127, fp) != NULL)
    {
        if (strncmp(buff, "#", 1) == 0 || strncmp(buff, "\n", 1) == 0)
        {
            num++;
            continue;
        }

        if (buff[strlen(buff) - 1] == '\n')
        {
            buff[strlen(buff) - 1] = '\0';
        }

        char *k = strtok(buff, " ");
        char *v = strtok(NULL, " ");

        if (k == NULL || v == NULL)
        {
            cout << "文件第: " << num++ << " 行无法解析" << endl;
            continue;
        }

        if (strcmp(k, "ip") == 0)
        {
            ips = v;
        }
        else if (strcmp(k, "port") == 0)
        {
            port = atoi(v);
        }
        else if (strcmp(k, "lismax") == 0)
        {
            lismax = atoi(v);
        }
        else if (strcmp(k, "taskmax") == 0)
        {
            taskmax = atoi(v);
        }
        else if (strcmp(k, "threadnum") == 0)
        {
            threadnum = atoi(v);
        }
        else if (strcmp(k, "db_ip") == 0)
        {
            db_ip = v;
        }
        else if (strcmp(k, "db_port") == 0)
        {
            db_port = atoi(v);
        }
        else if (strcmp(k, "db_name") == 0)
        {
            db_name = v;
        }
        else if (strcmp(k, "db_username") == 0)
        {
            db_username = v;
        }
        else if (strcmp(k, "db_passwd") == 0)
        {
            db_passwd = v;
        }
        else if (strcmp(k, "pool_min_size") == 0)
        {
            pool_min_size = atoi(v);
        }
        else if (strcmp(k, "pool_max_size") == 0)
        {
            pool_max_size = atoi(v);
        }
        else if (strcmp(k, "log_dir") == 0)
        {
            log_dir = v;
        }
        else if (strcmp(k, "log_enable") == 0)
        {
            log_enable = atoi(v);
        }
        else if (strcmp(k, "log_level") == 0)
        {
            log_level = v;
        }
        else
        {
            cout << "无效k:" << k << endl;
        }

        num++;
    }

    fclose(fp);
    return true;
}
void SerConf::PrintInfo()
{
    cout << "----SerConf----" << endl;
    cout << "=== 网络配置 ===" << endl;
    cout << "ip: " << ips << endl;
    cout << "port: " << port << endl;
    cout << "lismax: " << lismax << endl;
    cout << "=== 线程池配置 ===" << endl;
    cout << "threadnum: " << threadnum << endl;
    cout << "taskmax: " << taskmax << endl;
    cout << "=== 数据库配置 ===" << endl;
    cout << "db_ip: " << db_ip << endl;
    cout << "db_port: " << db_port << endl;
    cout << "db_name: " << db_name << endl;
    cout << "db_username: " << db_username << endl;
    cout << "=== 日志配置 ===" << endl;
    cout << "log_dir: " << log_dir << endl;
    cout << "log_enable: " << (log_enable ? "是" : "否") << endl;
    cout << "log_level: " << log_level << endl;
    cout << "---------------" << endl;
}

// 线程函数
void *worker_thread(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;
    pool->Work();
    return NULL;
}
// 线程池类的实现
bool ThreadPool::Thread_Pool_Init()
{
    queue = (task_t *)malloc(sizeof(task_t) * max_queue); // 任务队列分配空间
    if (queue == nullptr)
    {
        return false;
    }

    front = 0;
    rear = 0;
    count = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    stop = 0; // 不退出，

    return true;
}
void ThreadPool::Add_Task(Socket *csocket) // 向任务队列添加任务，
{
    pthread_mutex_lock(&mutex);
    if (count >= max_queue) // 任务队列满了
    {
        delete csocket;
        pthread_mutex_unlock(&mutex);
        return;
    }

    queue[rear].con = csocket; // 添加任务
    rear = (rear + 1) % max_queue;
    count++;

    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}
void ThreadPool::Start_Thread() // 线程启动
{
    pthread_t ids[thread_num];
    for (int i = 0; i < thread_num; i++)
    {
        pthread_create(&ids[i], NULL, worker_thread, this); // 启动线程函数，将线程池对象指针传递
    }
}
void ThreadPool::Work() // 工作线程要做的事情
{
    while (true)
    {
        pthread_mutex_lock(&mutex);
        while (count == 0 && !stop)
        {
            pthread_cond_wait(&cond, &mutex);
        }

        if (stop) // 退出线程，结束程序
        {
            break;
        }

        Socket *con = queue[front].con; // con, LisSocket* , ConSocet*
        front = (front + 1) % max_queue;
        count--;

        pthread_mutex_unlock(&mutex);

        if (con != nullptr)
        {
            con->Handle_Data();
        }
    }
}

// LisSocket
int LisSocket::m_Count = 0;
void LisSocket::RestEvent()
{
    struct epoll_event ev;
    ev.data.ptr = this;
    ev.events = EPOLLIN | EPOLLONESHOT;
    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, m_fd, &ev) == -1)
    {
        cout << "ResetEvent err" << endl;
    }
}
void LisSocket::Handle_Data()
{
    // 接受客户端连接
    int c = accept(m_fd, NULL, NULL);
    if (c < 0)
    {
        log_error("Failed to accept client connection");
        return;
    }

    RestEvent(); // 重置事件，让epoll_wait继续触发

    log_info("New client connected, fd: " + to_string(c) + ", total connections: " + to_string(++m_Count));
    cout << "accept c=" << c << "客户端连接数:" << m_Count << endl;

    ConSocket *cs = new ConSocket(c, m_epfd);
    if (cs == nullptr)
    {
        close(c);
        return;
    }
    struct epoll_event ev;
    ev.data.ptr = cs;                   //
    ev.events = EPOLLIN | EPOLLONESHOT; // 只触发一次
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, c, &ev) == -1)
    {
        cout << "epoll ctl add c err" << endl;
        return;
    }
}

// MysqlClient 构造函数
MysqlClient::MysqlClient() : mysql_conn_(nullptr) {
}

MysqlClient::~MysqlClient() {
    if (mysql_conn_) {
        ConnectionPool::getInstance().releaseConnection(mysql_conn_);
        mysql_conn_ = nullptr;
    }
}

bool MysqlClient::Connect_MysqlServer() {
    mysql_conn_ = ConnectionPool::getInstance().getConnection();
    if (!mysql_conn_) {
        log_error("Failed to get connection from pool");
        return false;
    }
    log_info("Got connection from pool, active connections: " + 
             std::to_string(ConnectionPool::getInstance().getActiveCount()));
    return true;
}
bool MysqlClient::Db_User_Register(const string &tel, const string &name, const string &passwd)
{
    // 对密码进行MD5加密
    string encrypted_passwd = md5_hash(passwd);
    
    // insert into user_info values(0,'13700000001','小王','md5_pass','salt',1,curdate());
    string sql = string("insert into user_info values(0,'") + tel + string("','") + name + string("','") + encrypted_passwd + string("','',1,curdate())");
    if (mysql_query(mysql_conn_, sql.c_str()) != 0)
    {
        log_error(string("Register failed: ") + mysql_error(mysql_conn_));
        return false;
    }
    log_info("User registered: " + tel);
    return true;
}
bool MysqlClient::Db_User_Login(const string &tel, string &name, const string &passwd)
{
    // 对密码进行MD5加密
    string encrypted_passwd = md5_hash(passwd);
    
    // select Name from user_info where Tel='13400000002' and Passwd='md5_pass'
    string sql = string("select Name from user_info where Tel='") + tel + string("' and Passwd='") + encrypted_passwd + string("'");
    if (mysql_query(mysql_conn_, sql.c_str()) != 0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(mysql_conn_); // 获取结果集
    if (r == NULL)
    {
        return false;
    }

    int num = mysql_num_rows(r); // 获取结果集有几行记录（实际应该只有1行）
    if (num != 1)
    {
        mysql_free_result(r);
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(r); // 获取一行记录（实际也就这一行)
    name = row[0];
    mysql_free_result(r);
    return true;
}

bool MysqlClient::Db_Show_Ticket(Json::Value &res)
{
    string sql = "select * from ticket_table"; // 补充，根据日期处理
    if (mysql_query(mysql_conn_, sql.c_str()) != 0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(mysql_conn_);
    if (r == nullptr)
    {
        return false;
    }

    int num = mysql_num_rows(r); // 获取结果集有多少行
    res["status"] = "OK";
    res["num"] = num;

    if (num == 0)
    {
        mysql_free_result(r);
        return true;
    }

    for (int i = 0; i < num; i++)
    {
        Json::Value tmp;
        MYSQL_ROW row = mysql_fetch_row(r); // 获取一行信息，
        tmp["ticket_id"] = row[0];
        tmp["ticket_name"] = row[1];
        tmp["ticket_max"] = row[2];
        tmp["ticket_count"] = row[3];
        tmp["day_time"] = row[4]; // 实际字段是 datetime
        res["ticket_arr"].append(tmp);
    }
    mysql_free_result(r);
    return true;
}
bool MysqlClient::Db_Yd_Ticket(string usertel, string tk_id)
{
    // select tk_max,tk_count from ticket_table where tk_id=1;
    string sql_max_count = string("select tk_max,tk_count from ticket_table where tk_id=") + tk_id;
    if (mysql_query(mysql_conn_, sql_max_count.c_str()) != 0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(mysql_conn_); // 获取结果集
    if (r == nullptr)
    {
        return false;
    }

    int num = mysql_num_rows(r); // 获取查询结果有几条记录
    if (num == 0)
    {
        mysql_free_result(r);
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(r); // 获取一条记录（也只能是一条) 包含 max,count
    int max = atoi(row[0]);
    int count = atoi(row[1]);
    mysql_free_result(r);

    if (count >= max) // 没票可以定,
    {
        return false;
    }

    count++;

    Mysql_Begin(); // 开启事物
    // update ticket_table set tk_count = 2 where tk_id = 1;
    string sql_setcount = string("update ticket_table set tk_count = ") + to_string(count) + string(" where tk_id = ") + tk_id;
    if (mysql_query(mysql_conn_, sql_setcount.c_str()) != 0)
    {
        Mysql_RollBack();
        return false;
    }

    // insert into yd_table values(0,'手机号',1,now(),1);
    // 字段顺序: yd_id, tel, tk_id, ctime, status
    string sql_yd = string("insert into yd_table values(0,'") + usertel + string("',") + tk_id + string(",now(),1)");
    if (mysql_query(mysql_conn_, sql_yd.c_str()) != 0)
    {
        Mysql_RollBack();
        return false;
    }

    Mysql_Commit();
    return true;
}
bool MysqlClient::Db_My_Ticket(const string &usertel, Json::Value &res)
{
    // 字段: yd_table(yd_id, tel, tk_id, ctime, status), ticket_table(tk_id, tk_name, ...)
    // 注意: yd_table 用 tel 字段，不是 usertel；用 ctime，不是 yd_time
    string sql = string("select yd_table.yd_id, ticket_table.tk_name, yd_table.ctime, yd_table.tk_id from yd_table, ticket_table where yd_table.tk_id = ticket_table.tk_id and yd_table.tel='") + usertel + string("'");
    if (mysql_query(mysql_conn_, sql.c_str()) != 0)
    {
        return false;
    }

    MYSQL_RES *r = mysql_store_result(mysql_conn_);
    if (r == nullptr)
    {
        return false;
    }

    int num = mysql_num_rows(r);
    res["status"] = "OK";
    res["num"] = num;

    if (num == 0)
    {
        mysql_free_result(r);
        return true;
    }

    for (int i = 0; i < num; i++)
    {
        Json::Value tmp;
        MYSQL_ROW row = mysql_fetch_row(r);
        tmp["yd_id"] = row[0];
        tmp["tk_name"] = row[1];
        tmp["yd_time"] = row[2];
        tmp["tk_id"] = row[3];
        res["yd_arr"].append(tmp);
    }
    mysql_free_result(r);
    return true;
}
bool MysqlClient::Db_Cancel_Ticket(const string &usertel, const string &yd_id)
{
    // 根据预约ID查询预约信息，验证是否属于该用户
    // yd_id 加引号，确保字符串类型也能正确查询
    string sql_sel = string("select tk_id from yd_table where tel='") + usertel + string("' and yd_id='") + yd_id + string("'");
    cout << "[DEBUG] Db_Cancel_Ticket SQL: " << sql_sel << endl;
    if (mysql_query(mysql_conn_, sql_sel.c_str()) != 0)
    {
        cout << "[ERROR] Query failed: " << mysql_error(mysql_conn_) << endl;
        return false;
    }

    MYSQL_RES *r = mysql_store_result(mysql_conn_);
    if (r == nullptr)
    {
        cout << "[ERROR] Store result failed: " << mysql_error(mysql_conn_) << endl;
        return false;
    }

    int num = mysql_num_rows(r);
    cout << "[DEBUG] Found " << num << " reservation(s) matching yd_id: " << yd_id << endl;
    if (num == 0)
    {
        mysql_free_result(r);
        cout << "[DEBUG] No reservation found for yd_id: " << yd_id << endl;
        return false; // 没有预约
    }

    // 获取票ID
    MYSQL_ROW row = mysql_fetch_row(r);
    string tk_id = row[0];
    mysql_free_result(r);

    Mysql_Begin(); // 开启事物

    // 根据预约ID删除单条预约记录，yd_id 加引号
    string sql_del = string("delete from yd_table where yd_id='") + yd_id + string("'");
    cout << "[DEBUG] Delete SQL: " << sql_del << endl;
    if (mysql_query(mysql_conn_, sql_del.c_str()) != 0)
    {
        cout << "[ERROR] Delete failed: " << mysql_error(mysql_conn_) << endl;
        Mysql_RollBack();
        return false;
    }

    // 更新票的数量 - 恢复1张
    string sql_max_count = string("select tk_count from ticket_table where tk_id=") + tk_id;
    cout << "[DEBUG] Select count SQL: " << sql_max_count << endl;
    if (mysql_query(mysql_conn_, sql_max_count.c_str()) != 0)
    {
        cout << "[ERROR] Select count failed: " << mysql_error(mysql_conn_) << endl;
        Mysql_RollBack();
        return false;
    }

    MYSQL_RES *r2 = mysql_store_result(mysql_conn_);
    if (r2 == nullptr)
    {
        Mysql_RollBack();
        return false;
    }

    MYSQL_ROW row2 = mysql_fetch_row(r2);
    int count = atoi(row2[0]);
    mysql_free_result(r2);

    count -= 1; // 恢复1张票
    string sql_setcount = string("update ticket_table set tk_count=") + to_string(count) + string(" where tk_id=") + tk_id;
    cout << "[DEBUG] Update SQL: " << sql_setcount << endl;
    if (mysql_query(mysql_conn_, sql_setcount.c_str()) != 0)
    {
        cout << "[ERROR] Update failed: " << mysql_error(mysql_conn_) << endl;
        Mysql_RollBack();
        return false;
    }

    Mysql_Commit();
    return true;
}

// 按票ID取消预约（兼容旧版，会取消该用户所有该票的预约）
bool MysqlClient::Db_Cancel_Ticket_By_TkId(const string &usertel, const string &tk_id)
{
    // 先查询预约数量
    string sql_sel = string("select yd_id from yd_table where tel='") + usertel + string("' and tk_id=") + tk_id;
    cout << "[DEBUG] Db_Cancel_Ticket_By_TkId SQL: " << sql_sel << endl;
    if (mysql_query(mysql_conn_, sql_sel.c_str()) != 0)
    {
        cout << "[ERROR] Query failed: " << mysql_error(mysql_conn_) << endl;
        return false;
    }

    MYSQL_RES *r = mysql_store_result(mysql_conn_);
    if (r == nullptr)
    {
        return false;
    }

    int num = mysql_num_rows(r);
    mysql_free_result(r);

    if (num == 0)
    {
        return false; // 没有预约
    }

    Mysql_Begin(); // 开启事物

    // 删除所有该用户该票的预约记录
    string sql_del = string("delete from yd_table where tel='") + usertel + string("' and tk_id=") + tk_id;
    cout << "[DEBUG] Delete SQL: " << sql_del << endl;
    if (mysql_query(mysql_conn_, sql_del.c_str()) != 0)
    {
        cout << "[ERROR] Delete failed: " << mysql_error(mysql_conn_) << endl;
        Mysql_RollBack();
        return false;
    }

    // 更新票的数量 - 恢复num张
    string sql_max_count = string("select tk_count from ticket_table where tk_id=") + tk_id;
    cout << "[DEBUG] Select count SQL: " << sql_max_count << endl;
    if (mysql_query(mysql_conn_, sql_max_count.c_str()) != 0)
    {
        cout << "[ERROR] Select count failed: " << mysql_error(mysql_conn_) << endl;
        Mysql_RollBack();
        return false;
    }

    MYSQL_RES *r2 = mysql_store_result(mysql_conn_);
    if (r2 == nullptr)
    {
        Mysql_RollBack();
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(r2);
    int count = atoi(row[0]);
    mysql_free_result(r2);

    count += num; // 恢复num张票
    string sql_setcount = string("update ticket_table set tk_count=") + to_string(count) + string(" where tk_id=") + tk_id;
    cout << "[DEBUG] Update SQL: " << sql_setcount << endl;
    if (mysql_query(mysql_conn_, sql_setcount.c_str()) != 0)
    {
        cout << "[ERROR] Update failed: " << mysql_error(mysql_conn_) << endl;
        Mysql_RollBack();
        return false;
    }

    Mysql_Commit();
    return true;
}

void MysqlClient::Mysql_Begin() // 开启事物
{
    if (mysql_query(mysql_conn_, "begin") != 0)
    {
        cout << "启动事物失败" << endl;
    }
}
void MysqlClient::Mysql_RollBack() // 回滚事物
{
    if (mysql_query(mysql_conn_, "rollback") != 0)
    {
        cout << "回滚事物失败" << endl;
    }
}
void MysqlClient::Mysql_Commit() // 提交事物
{
    if (mysql_query(mysql_conn_, "commit") != 0)
    {
        cout << "提交事物失败" << endl;
    }
}
// ConSocket
void ConSocket::RestEvent()
{
    struct epoll_event ev;
    ev.data.ptr = this;
    ev.events = EPOLLIN | EPOLLONESHOT;
    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, m_fd, &ev) == -1)
    {
        cout << "ResetEvent err" << endl;
    }
}
void ConSocket::Send_Ok()
{
    Json::Value val;
    val["status"] = "OK";
    send(m_fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);
}
void ConSocket::Send_Err()
{
    Json::Value val;
    val["status"] = "ERR";
    val["msg"] = "操作失败";
    send(m_fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);
}

void ConSocket::Send_Err(const string &msg)
{
    Json::Value val;
    val["status"] = "ERR";
    val["msg"] = msg;
    send(m_fd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);
}
void ConSocket::Send_JsonObj(Json::Value &root)
{
    send(m_fd, root.toStyledString().c_str(), strlen(root.toStyledString().c_str()), 0);
}
void ConSocket::Get_OpType(char buff[])
{
    Json::Reader Read;
    m_val.clear();
    if (!Read.parse(buff, m_val))
    {
        cout << "json 解析失败" << endl;
        m_OpType = -1;
        return;
    }

    m_OpType = m_val["type"].asInt();
}

void ConSocket::User_Register()
{
    // 从json对象获取各个字段
    string username = m_val["user_name"].asString();
    string usertel = m_val["user_tel"].asString();
    string passwd = m_val["user_passwd"].asString();
    
    log_info("User registration attempt: " + usertel + " (" + username + ")");
    
    // 操作数据库
    MysqlClient cli;
    if (!cli.Connect_MysqlServer())
    {
        log_error("Registration failed: database connection error for user " + usertel);
        Send_Err("数据库连接失败，请稍后重试");
        return;
    }

    if (!cli.Db_User_Register(usertel, username, passwd))
    {
        log_error("Registration failed for user " + usertel);
        Send_Err("注册失败，该手机号已被注册");
        return;
    }

    log_info("User registration successful: " + usertel + " (" + username + ")");
    Send_Ok();
    return;
}
void ConSocket::User_Login()
{
    // 从json对象获取各个字段
    string usertel = m_val["user_tel"].asString();
    string passwd = m_val["user_passwd"].asString();
    string username;
    
    log_info("User login attempt: " + usertel);
    
    // 操作数据库
    MysqlClient cli;
    if (!cli.Connect_MysqlServer())
    {
        log_error("Login failed: database connection error for user " + usertel);
        Send_Err("数据库连接失败，请稍后重试");
        return;
    }

    if (!cli.Db_User_Login(usertel, username, passwd))
    {
        log_error("Login failed: invalid credentials for user " + usertel);
        Send_Err("账号或密码错误");
        return;
    }

    log_info("User login successful: " + usertel + " (" + username + ")");
    
    Json::Value res;
    res["status"] = "OK";
    res["user_name"] = username;
    Send_JsonObj(res);
    return;
}
void ConSocket::Show_Ticket() // 查看预约信息
{
    Json::Value res;
    MysqlClient cli;
    if (!cli.Connect_MysqlServer())
    {
        Send_Err();
        return;
    }

    if (!cli.Db_Show_Ticket(res))
    {
        Send_Err();
        return;
    }

    send(m_fd, res.toStyledString().c_str(), strlen(res.toStyledString().c_str()), 0);
}
void ConSocket::Yd_Ticket() // 预定
{
    string user_tel = m_val["user_tel"].asString();
    string tk_id = m_val["ticket_id"].asString();

    log_info("User " + user_tel + " attempting to book ticket: " + tk_id);

    // op mysql
    MysqlClient cli;
    if (!cli.Connect_MysqlServer())
    {
        log_error("Booking failed: database connection error for user " + user_tel);
        Send_Err("数据库连接失败，请稍后重试");
        return;
    }

    if (!cli.Db_Yd_Ticket(user_tel, tk_id))
    {
        log_error("Booking failed for user " + user_tel + ", ticket: " + tk_id);
        Send_Err("预约失败，票已售罄或不存在");
        return;
    }

    log_info("Booking successful: user " + user_tel + ", ticket: " + tk_id);
    Send_Ok();
    return;
}
void ConSocket::My_Ticket() // 我的预约
{
    string user_tel = m_val["user_tel"].asString();
    
    log_info("User " + user_tel + " checking my tickets");

    Json::Value res;
    MysqlClient cli;
    if (!cli.Connect_MysqlServer())
    {
        log_error("My_Ticket failed: database connection error for user " + user_tel);
        Send_Err("数据库连接失败，请稍后重试");
        return;
    }

    if (!cli.Db_My_Ticket(user_tel, res))
    {
        log_error("My_Ticket failed for user " + user_tel);
        Send_Err("查询预约失败");
        return;
    }

    Send_JsonObj(res);
    return;
}
void ConSocket::Cancel_Ticket() // 取消预约
{
    string user_tel = m_val["user_tel"].asString();
    string yd_id = m_val["yd_id"].asString();       // 预约ID（优先使用）
    string ticket_id = m_val["ticket_id"].asString(); // 票ID（兼容旧版）

    // 优先使用预约ID，如果没有则使用票ID
    if (!yd_id.empty()) {
        log_info("User " + user_tel + " attempting to cancel reservation by yd_id: " + yd_id);
        
        MysqlClient cli;
        if (!cli.Connect_MysqlServer())
        {
            log_error("Cancel failed: database connection error for user " + user_tel);
            Send_Err("数据库连接失败，请稍后重试");
            return;
        }

        if (!cli.Db_Cancel_Ticket(user_tel, yd_id))
        {
            log_error("Cancel failed for user " + user_tel + ", reservation: " + yd_id);
            Send_Err("取消失败，预约不存在或已过期");
            return;
        }

        log_info("Cancel successful: user " + user_tel + ", reservation: " + yd_id);
        Send_Ok();
        return;
    } else if (!ticket_id.empty()) {
        // 兼容旧版：使用票ID取消（会取消该用户所有该票的预约）
        log_info("User " + user_tel + " attempting to cancel ticket: " + ticket_id);
        
        MysqlClient cli;
        if (!cli.Connect_MysqlServer())
        {
            log_error("Cancel failed: database connection error for user " + user_tel);
            Send_Err("数据库连接失败，请稍后重试");
            return;
        }

        if (!cli.Db_Cancel_Ticket_By_TkId(user_tel, ticket_id))
        {
            log_error("Cancel failed for user " + user_tel + ", ticket: " + ticket_id);
            Send_Err("取消失败，预约不存在或已过期");
            return;
        }

        log_info("Cancel successful: user " + user_tel + ", ticket: " + ticket_id);
        Send_Ok();
        return;
    } else {
        Send_Err("请提供预约ID或票ID");
        return;
    }
}
void ConSocket::Handle_Data()
{
    char buff[1024] = {0}; // json字符串
    int n = recv(m_fd, buff, 1023, 0);
    if (n <= 0)
    {
        cout << "client close" << endl;
        delete this;
        return;
    }
    Get_OpType(buff);

    switch (m_OpType)
    {
    case ZC:
        User_Register();
        break;
    case DL:
        User_Login();
        break;
    case CKYY:
        Show_Ticket();
        break;
    case YD:
        Yd_Ticket();
        break;
    case WDYY:
        My_Ticket();
        break;
    case QXYY:
        Cancel_Ticket();
        break;

    default:
        break;
    }

    RestEvent();
}
bool TcpSer::Ser_Init()
{
    if (!create_socket())
    {
        return false;
    }

    m_epfd = epoll_create1(0); // 创建内核事件表
    if (m_epfd == -1)
    {
        return false;
    }

    // 线程池类 创建初始化

    return true;
}

void TcpSer::do_events()
{
    for (int i = 0; i < m_num; i++)
    {
        if (m_evs[i].events & EPOLLIN)
        {
            m_pool.Add_Task((Socket *)m_evs[i].data.ptr);
        }
    }
}
void TcpSer::Run()
{
    log_info("Starting server...");
    
    if (!Ser_Init())
    {
        log_error("Server initialization failed");
        cout << "服务初始化失败" << endl;
        return;
    }
    
    log_info("Server initialized successfully");

    LisSocket *sock = new LisSocket(m_sockfd, m_epfd);
    if (sock == nullptr)
    {
        log_error("Failed to create LisSocket");
        cout << "create LisSocket err" << endl;
        return;
    }

    struct epoll_event ev;
    ev.data.ptr = sock;                 // 用户数据，是处理该描述符的类的对象的地址(指针)
    ev.events = EPOLLIN | EPOLLONESHOT; // 读事件
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_sockfd, &ev) == -1)
    {
        log_error("Failed to add socket to epoll");
        cout << "epoll ctl add err" << endl;
        return;
    }

    m_pool.Thread_Pool_Init(); // 初始化线程池
    m_pool.Start_Thread();
    log_info("Thread pool started with " + to_string(m_conf->threadnum) + " threads");

    while (true)
    {
        m_num = epoll_wait(m_epfd, m_evs, EV_MAX, 5000);
        if (m_num == -1) // epoll_wait失败
        {
            cout << "epoll wait err" << endl;
        }
        else if (m_num == 0) // 超时
        {
            cout << "time out" << endl;
        }
        else // 就绪事件
        {
            do_events();
        }
    }
}
bool TcpSer::create_socket()
{
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd == -1)
    {
        return false;
    }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(m_conf->port);
    saddr.sin_addr.s_addr = inet_addr(m_conf->ips.c_str());
    if (bind(m_sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    {
        log_error("Port " + to_string(m_conf->port) + " is already in use");
        return false;
    }

    if (listen(m_sockfd, m_conf->lismax) == -1)
    {
        return false;
    }
    return true;
}
int main()
{
    // 获取单例配置实例
    SerConf* conf = SerConf::getInstance();
    
    // 读取配置文件
    if (!conf->ReadConf("my.conf"))
    {
        exit(1);
    }

    // 启动 LogFile 日志系统
    if (!InitLogSystem()) {
        std::cerr << "日志系统启动失败" << std::endl;
        exit(1);
    }

    // 初始化数据库连接池
    LOG_INFO << "Initializing connection pool...";
    bool pool_ok = ConnectionPool::getInstance().init(
        conf->db_ip,
        conf->db_port,
        conf->db_username,
        conf->db_passwd,
        conf->db_name,
        conf->pool_min_size,  // 最小连接数（从配置文件读取）
        conf->pool_max_size   // 最大连接数（从配置文件读取）
    );
    if (!pool_ok) {
        LOG_ERROR << "连接池初始化失败";
        exit(1);
    }
    LOG_INFO << "连接池初始化成功";

    conf->PrintInfo();

    LOG_INFO << "服务器启动中...";

    TcpSer ser(conf);
    ser.Run();

    // 停止日志系统
    logfile::LogStarter::getInstance().stop();

    exit(0);
}