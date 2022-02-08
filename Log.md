### 2022/1/28
1. 该聊天室支持一对一的多线程通信(即发送与接收线程分离).

### 2022/2/3
1. 更新了账号系统与登录机制: 服务端的chatroom_account定义了用户账号操作及登录验证, server_account针对登录机制实现客户端的通信; 客户端的client_account针对登录机制实现服务端的通信.

### 2022/2/8
1. 更新了指令系统, 其实现在server/client_command.hpp下, 在对应终端使用/help可查看指令.
2. 更新了文件传输功能, 其实现在server/client_file_transport.hpp下, 使用/filesd (path)可传输文件至对方本地. 