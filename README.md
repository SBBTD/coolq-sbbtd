### 已迁移至coolq-sbbtd-golang项目，现有bug不再修复

自用酷Q插件简介
--------

1. 线报追踪系统，方便我和舍友等用户买到心仪的便宜商品，而不必每天盯着一堆线报群筛选。

配置文件实时保存。已实测连续1月、累计1年以上稳如老狗的运行。

2. 拦截舍友的机器人的消息，阻止后续消息处理，防止两个机器人死循环打起来。。。

配置文件说明
--------

1. `settings_blockqq.ini`  每行一个qq号，拦截消息的列表

2. `settings_xianbaogroup.ini` 每行一个群号，线报群的列表

3. `settings_monitor.ini`  每行为`关键词 所在群号 qq号`正在追踪的线报关键词列表

其他功能
--------
在自己宿舍群里为所欲为，指令快速禁言舍友（Σ(ŎдŎ|||)ﾉﾉ）

更多功能自己想到会加，还有很多需要优化的地方，请多指教


以下为原SDK说明（有删改）
--------
酷Q V9

cqsdk-vc来自[`https://github.com/CoolQ/cqsdk-vc`](https://github.com/CoolQ/cqsdk-vc)

文件说明
--------

您可以编译为 `com.sbbtd.sbbtd.sbbtd.dll`，与 `coolq-sbbtd/com.sbbtd.sbbtd.sbbtd.json` 一起放置在 酷Q 的 app 目录下测试

`coolq-sbbtd/com.sbbtd.sbbtd.sbbtd.json` - 样例应用的对应信息文件，包含应用的基础信息、事件列表等，请放置在 酷Q 的 app 目录下

`coolq-sbbtd/cqp.h` - 酷Q SDK 头文件，通常无需修改

`coolq-sbbtd/CQP.lib` - CQP.dll 的动态连接 .lib 文件，便于 C、C++ 等调用 酷Q 的方法。

酷Q官方网站
--------
主站：https://cqp.cc

文库：https://d.cqp.me
