# Chatroom

#### 介绍
1. 该仓库为基于socket的C++多线程聊天室实现.
2. 更新日志见Log.md.

#### 架构:
1. chatroom_client/chatroom_server: 聊天室的实现, 其中包含chatroom_base类及子类chatroom_client/server; chatroom_base类的作用是初始化WSA和socket, 而子类负责聊天室的主要业务.
2. chatroom_account: 账号及其相关机制的实现.
3. server_account/client_account: 登录机制的相互通信实现. 
4. client/server : 对chatroom_client/chatroom_server的调用.

#### 命令:
1. /exit : client端为退出; server端为退出当前通信, 但不关闭进程, 而是等待下一次连接.

#### 最近更新:
##### 2022/2/3
1. 更新了账号系统与登录机制: 服务端的chatroom_account定义了用户账号操作及登录验证, server_account针对登录机制实现客户端的通信; 客户端的client_account针对登录机制实现服务端的通信.


