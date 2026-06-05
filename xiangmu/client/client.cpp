#include "client.h"

bool TcpClient::Socket_Init()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        cout << "❌ 创建socket失败" << endl;
        return false;
    }

    struct sockaddr_in saddr; // 服务器
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ips.c_str());

    int res = connect(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if (res == -1)
    {
        cout << "❌ 连接服务器失败，请确保服务器已启动" << endl;
        return false;
    }

    cout << "✅ 成功连接到服务器" << endl;
    return true;
}

bool TcpClient::Check_Connection()
{
    if (sockfd <= 0)
    {
        return false;
    }
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    
    int ret = select(sockfd + 1, &readfds, NULL, NULL, &tv);
    if (ret < 0)
    {
        return false;
    }
    else if (ret == 0)
    {
        return true;
    }
    else
    {
        char buff[1];
        int n = recv(sockfd, buff, 1, MSG_PEEK);
        if (n == 0)
        {
            return false;
        }
        return true;
    }
}
bool TcpClient::Validate_Phone(const string &phone)
{
    if (phone.length() != 11) return false;
    for (char c : phone) {
        if (!isdigit(c)) return false;
    }
    return true;
}

bool TcpClient::Validate_Password(const string &passwd)
{
    if (passwd.length() < 6) return false;
    return true;
}

void TcpClient::User_Register()
{
    cout << endl << "╔════════════════════════════════╗" << endl;
    cout << "║           用户注册             ║" << endl;
    cout << "╚════════════════════════════════╝" << endl;
    
    while (true) {
        cout << "请输入注册的手机号码：";
        cin >> usertel;
        if (Validate_Phone(usertel)) break;
        cout << "❌ 手机号格式错误，请输入11位数字！" << endl;
    }
    
    while (true) {
        cout << "请输入用户名：";
        cin >> username;
        if (!username.empty()) break;
        cout << "❌ 用户名不能为空！" << endl;
    }
    
    string passwd;
    while (true) {
        cout << "请输入密码（至少6位）：";
        cin >> passwd;
        if (Validate_Password(passwd)) break;
        cout << "❌ 密码长度不足6位！" << endl;
    }

    Json::Value val;
    val["type"] = ZC;
    val["user_tel"] = usertel;
    val["user_name"] = username;
    val["user_passwd"] = passwd;

    // 发送登陆json格式字符串到服务器
    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);

    char buff[256] = {0}; // 接受服务器返回的状态信息
    int n = recv(sockfd, buff, 255, 0);
    if (n <= 0)
    {
        cout << "ser close" << endl;
        return;
    }

    Json::Value rval;
    Json::Reader Read;
    if (!Read.parse(buff, rval))
    {
        cout << "json格式解析错误,注册失败" << endl;
        return;
    }

    string rstatus = rval["status"].asString();
    if (rstatus.compare("OK") != 0)
    {
        string err_msg = rval["msg"].asString();
        if (err_msg.empty()) {
            cout << "❌ 注册失败" << endl;
        } else {
            cout << "❌ " << err_msg << endl;
        }
        return;
    }

    cout << "✅ 注册成功" << endl;
    login_status = true;
}
void TcpClient::User_Login()
{
    cout << "请输入手机号码：" << endl;
    cin >> usertel;
    cout << "请输入密码:" << endl;
    string passwd;
    cin >> passwd;

    Json::Value val;
    val["type"] = DL;
    val["user_tel"] = usertel;
    val["user_passwd"] = passwd;
    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);

    char buff[256] = {0};
    int n = recv(sockfd, buff, 255, 0);
    if (n <= 0)
    {
        cout << "ser close" << endl;
        return;
    }

    Json::Value root;
    Json::Reader Read;
    if (!Read.parse(buff, root))
    {
        cout << "json解析失败" << endl;
        return;
    }

    string s = root["status"].asString();
    if (s.compare("OK") != 0)
    {
        string err_msg = root["msg"].asString();
        if (err_msg.empty()) {
            cout << "❌ 登录失败" << endl;
        } else {
            cout << "❌ " << err_msg << endl;
        }
        return;
    }

    username = root["user_name"].asString();
    login_status = true;
    cout << "✅ 登录成功，欢迎 " << username << "!" << endl;
}
void TcpClient::Show_Ticket() // 显示可预约的信息
{
    Json::Value val;
    val["type"] = CKYY;
    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);

    // 接受服务器回复的数据
    char buff[1024] = {0};
    if (recv(sockfd, buff, 1023, 0) <= 0)
    {
        cout << "ser close" << endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if (!Read.parse(buff, res_val))
    {
        cout << "json 解析错误" << endl;
        return;
    }

    string s = res_val["status"].asString();
    if (s.compare("OK") != 0)
    {
        cout << "查看预约信息失败" << endl;
        return;
    }

    int num = res_val["num"].asInt();
    if (num == 0)
    {
        cout << "暂时没有可预约的信息" << endl;
        return;
    }

    cout << "╔══════╦══════════╦════════╦════════╦════════════════╗" << endl;
    cout << "║ 票ID ║   名称   ║ 总票数 ║ 已预定 ║    时间        ║" << endl;
    cout << "╠══════╬══════════╬════════╬════════╬════════════════╣" << endl;
    for (int i = 0; i < num; i++)
    {
        printf("║ %4s ║  %-10s ║ %6s ║ %6s ║ %13s  ║\n",
               res_val["ticket_arr"][i]["ticket_id"].asString().c_str(),
               res_val["ticket_arr"][i]["ticket_name"].asString().c_str(),
               res_val["ticket_arr"][i]["ticket_max"].asString().c_str(),
               res_val["ticket_arr"][i]["ticket_count"].asString().c_str(),
               res_val["ticket_arr"][i]["day_time"].asString().c_str());
    }
    cout << "╚══════╩══════════╩════════╩════════╩════════════════╝" << endl;

    return;
}
void TcpClient::Yd_Ticket() // 预定功能
{
    Show_Ticket(); // 显示可预约的信息
    cout << "请输入要预定的序号" << endl;
    int index;
    cin >> index; // 有效性的检查

    Json::Value val;
    val["type"] = YD;
    val["user_tel"] = usertel;
    val["ticket_id"] = to_string(index);

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);

    char buff[128] = {0};
    int n = recv(sockfd, buff, 127, 0);
    if (n <= 0)
    {
        cout << "ser close" << endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if (!Read.parse(buff, res_val))
    {
        cout << "json 无法解析" << endl;
        return;
    }

    string s = res_val["status"].asString();
    if (s.compare("OK") != 0)
    {
        string err_msg = res_val["msg"].asString();
        if (err_msg.empty()) {
            cout << "❌ 预定失败" << endl;
        } else {
            cout << "❌ " << err_msg << endl;
        }
        return;
    }

    cout << "✅ 预定成功" << endl;
    return;
}
void TcpClient::My_Ticket() // 我的预约
{
    Json::Value val;
    val["type"] = WDYY;
    val["user_tel"] = usertel;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);

    char buff[1024] = {0};
    if (recv(sockfd, buff, 1023, 0) <= 0)
    {
        cout << "ser close" << endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if (!Read.parse(buff, res_val))
    {
        cout << "json 解析错误" << endl;
        return;
    }

    string s = res_val["status"].asString();
    if (s.compare("OK") != 0)
    {
        string err_msg = res_val["msg"].asString();
        if (err_msg.empty()) {
            cout << "❌ 查看我的预约失败" << endl;
        } else {
            cout << "❌ " << err_msg << endl;
        }
        return;
    }

    int num = res_val["num"].asInt();
    if (num == 0)
    {
        cout << "您暂时没有预约信息" << endl;
        return;
    }

    cout << "╔════════╦═════════╦═════════════════════╦══════╗" << endl;
    cout << "║ 预约ID ║   名称  ║     预约时间        ║ 票ID ║" << endl;
    cout << "╠════════╬═════════╬═════════════════════╬══════╣" << endl;
    for (int i = 0; i < num; i++)
    {
        printf("║ %5s  ║ %-10s ║ %15s ║ %4s ║\n",
               res_val["yd_arr"][i]["yd_id"].asString().c_str(),
               res_val["yd_arr"][i]["tk_name"].asString().c_str(),
               res_val["yd_arr"][i]["yd_time"].asString().c_str(),
               res_val["yd_arr"][i]["tk_id"].asString().c_str());
    }
    cout << "╚════════╩═════════╩═════════════════════╩══════╝" << endl;

    return ;
}
void TcpClient::Cancel_Ticket() // 取消预约
{
    My_Ticket(); // 先显示我的预约

    cout << endl << "╔════════════════════════════════╗" << endl;
    cout << "║        取消预约操作            ║" << endl;
    cout << "╚════════════════════════════════╝" << endl;
    cout << "请输入要取消的预约ID：";
    string yd_id;
    cin >> yd_id;

    Json::Value val;
    val["type"] = QXYY;
    val["user_tel"] = usertel;
    val["yd_id"] = yd_id;

    send(sockfd, val.toStyledString().c_str(), strlen(val.toStyledString().c_str()), 0);

    char buff[128] = {0};
    int n = recv(sockfd, buff, 127, 0);
    if (n <= 0)
    {
        cout << "ser close" << endl;
        return;
    }

    Json::Value res_val;
    Json::Reader Read;
    if (!Read.parse(buff, res_val))
    {
        cout << "json 无法解析" << endl;
        return;
    }

    string s = res_val["status"].asString();
    if (s.compare("OK") != 0)
    {
        string err_msg = res_val["msg"].asString();
        if (err_msg.empty()) {
            cout << "❌ 取消预约失败" << endl;
        } else {
            cout << "❌ " << err_msg << endl;
        }
        return;
    }

    cout << "✅ 取消预约成功" << endl;
    return;
}
void TcpClient::Run()
{
    while (runing)
    {
        Print_info();
        
        if (op_type != TC)
        {
            if (!Check_Connection())
            {
                cout << endl << "❌ 与服务器断开连接，请重启客户端" << endl;
                runing = false;
                break;
            }
        }
        
        switch (op_type)
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
        case TC:
            runing = false;
            break;

        default:
            cout << "❌ 无效操作" << endl;
            break;
        }
    }
}

void TcpClient::Print_info()
{
    cout << endl;
    
    if (!login_status)
    {
        cout << "╔══════════════════════════════════════════╗" << endl;
        cout << "║         欢迎使用预约系统 - 未登陆        ║" << endl;
        cout << "╠══════════════════════════════════════════╣" << endl;
        cout << "║  【1】登录    【2】注册     【3】退出    ║" << endl;
        cout << "╚══════════════════════════════════════════╝" << endl;
        cout << "请输入操作序号：";
        cin >> op_type;
        if (op_type == 3)
        {
            op_type = TC;
        }
    }
    else
    {
        cout << "╔════════════════════════════════════════════╗" << endl;
        cout << "║      欢迎回来：" << username << " - 已登录               ║    " << endl;
        cout << "╠════════════════════════════════════════════╣" << endl;
        cout << "║  【1】查看可预约  【2】预约   【3】我的预约║" << endl;
        cout << "║  【4】取消预约    【5】退出                ║  " << endl;
        cout << "╚════════════════════════════════════════════╝" << endl;
        cout << "请输入操作序号：";
        cin >> op_type;
        op_type = op_type + 2;
    }
    cout << endl;
}

int main()
{
    TcpClient cli;
    if (!cli.Socket_Init())
    {
        exit(1);
    }
    cli.Run();
}