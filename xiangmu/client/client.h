#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>

using namespace std;
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

class TcpClient
{
public:
    TcpClient(string ser_ips, short ser_port) : ips(ser_ips), port(ser_port)
    {
        runing = true;
        login_status = false;
        op_type = -1;
    }

    TcpClient()
    {
        ips = "127.0.0.1";
        port = 6000;
        runing = true;
        login_status = false;
        op_type = -1;
    }
    ~TcpClient()
    {
        close(sockfd);
    }

    bool Socket_Init();
    bool Check_Connection();

    void Run();

private:
    void Print_info();
    bool Validate_Phone(const string &phone);
    bool Validate_Password(const string &passwd);
    void User_Register();
    void User_Login();
    void Show_Ticket();//显示可预约的信息
    void Yd_Ticket();//预定功能
    void My_Ticket();//我的预约
    void Cancel_Ticket();//取消预约

private:
    string ips; // 服务器ip
    short port; // 服务器端口

    int sockfd;        //
    bool runing;       // 是否继续运行
    bool login_status; // 有没有登陆

    int op_type; // 用户选择的操作类型

    string username; // 用户名
    string usertel;  // 用户手机号码
};