# Chatroom

#### 介绍
1. 该仓库为基于socket的C++多线程聊天室实现.
2. 更新日志见Log.md.

#### 架构:
1. chatroom_client/chatroom_server: 聊天室的实现, 其中包含chatroom_base类及子类chatroom_client/server; chatroom_base类的作用是初始化WSA和socket, 而子类负责聊天室的主要业务.
2. chatroom_account: 账号及其相关机制的实现.
3. server_account/client_account: 登录机制的相互通信实现. 
4. client/server : 对chatroom_client/chatroom_server的调用.
5. client/server_command : 指令系统实现.
6. client/server_file_transport : 文件传输系统实现.

#### 命令:
1. /exit : client端为退出; server端为退出当前通信, 但不关闭进程, 而是等待下一次连接.
2. /filesd : 发送文件至对方本地.
3. /filept : 修改文件保存路径.
4. /passwd : 修改账号密码.
5. /accdel : 删除账号.
6. /p : 私聊 (仅在mult chatroom中使用).
在终端使用/help可查看命令帮助, 在mult chatroom中server不支持指令.

#### 最近更新:
##### 2022/2/15
1. 重写架构, 支持多人通信. 该架构下server端仅负责数据的转发和账号管理服务; client端负责数据的发送与接收以及相关服务. 其实现在mult chatroom下, single chatroom下为一对一的旧聊天室架构. 
2. 新增了针对多人通信的私聊指令 "/p".


