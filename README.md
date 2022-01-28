# Chatroom

#### 介绍
1. 该仓库为基于socket的C++多线程聊天室实现.
2. 更新日志见Log.md.

#### 架构:
1. chatroom_client/chatroom_server: 聊天室的实现, 其中包含chatroom_base类及子类chatroom_client/server; chatroom_base类的作用是初始化WSA和socket, 而子类负责聊天室的主要业务.
2. client/server : 对chatroom_client/chatroom_server的调用.

#### 命令:
1./exit : client端为退出; server端为退出当前通信, 但不关闭进程, 而是等待下一次连接.


