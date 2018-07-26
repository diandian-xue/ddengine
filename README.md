# ddengine
Dark Dragon Engine

参照 skynet 核心代码和设计，加入自己的设计和跨平台封装。
这个项目代表我进一年来学习的成果，放在这里表示我的技术和能力。

## 目录
	3rd/  所有第三方库
	cmake/ 核心项目 cmake 模块配置
	ddcl/ core libaray
	example/ 一些例子
	lddcl/ core libaray 的 lua binding

	
## 项目功能
	和 skynet 一样的消息队列设计和 actor 模型
	lua coroutine + 消息队列封装，组成并发阻塞调用
	接管网络层，网络层支持多种(select, epoll, kevent)模型的非阻塞 io
	移植 skynet 核心高性能 timer 计时器。
	支持自建线程消息队列，也可以创建 工作线程接管的消息队列