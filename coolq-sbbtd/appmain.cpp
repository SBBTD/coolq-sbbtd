/*
* CoolQ Demo for VC++
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/

#include "stdafx.h"
#include "string"
#include "regex"
#include "cqp.h"
#include "appmain.h" //应用AppID等信息，请正确填写，否则酷Q可能无法加载
#include "sbbtd.h"

using namespace std;

int ac = -1; //AuthCode 调用酷Q的方法时需要用到
bool enabled = false;

bool bansaohuo = false;

//这两个均以-1表示数组结尾
int64_t blockQQList[blockQQList_Size];
int64_t xbGroupList[xbGroupList_Size];

monitorKeyList KeyList = new monitorKey;

/*
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/*
* 接收应用AuthCode，酷Q读取应用信息后，如果接受该应用，将会调用这个函数并传递AuthCode。
* 不要在本函数处理其他任何代码，以免发生异常情况。如需执行初始化代码请在Startup事件中执行（Type=1001）。
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 酷Q启动
* 无论本应用是否被启用，本函数都会在酷Q启动后执行一次，请在这里执行应用初始化代码。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventStartup, 0)() {
	memset(blockQQList, 0, sizeof(int64_t) * blockQQList_Size);
	blockQQList[0] = -1;
	memset(xbGroupList, 0, sizeof(int64_t) * xbGroupList_Size);
	xbGroupList[0] = -1;
	KeyList->group = KeyList->qq = KeyList->key[0] = 0;
	KeyList->pre = KeyList->next = nullptr;

	readfile(openfile(iniFileName_blockqq), blockQQList);
	readfile(openfile(iniFileName_xianbaogroup), xbGroupList);

	fstream* filest = openfile(iniFileName_monitor);
	int n = 0;
	monitorKeyList p = KeyList;
	while (!filest->eof()) {
		char key[keyword_Length];
		int64_t group = 0;
		int64_t qq = 0;
		*filest >> key >> group >> qq;
		addXianbaoKeyword(KeyList, key, group, qq);
	}
	filest->close();
	delete filest;
	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {

	return 0;
}

/*
* Type=1003 应用已被启用
* 当应用被启用后，将收到此事件。
* 如果酷Q载入时应用已被启用，则在_eventStartup(Type=1001,酷Q启动)被调用后，本函数也将被调用一次。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventEnable, 0)() {
	enabled = true;
	return 0;
}


/*
* Type=1004 应用将被停用
* 当应用被停用前，将收到此事件。
* 如果酷Q载入时应用已被停用，则本函数*不会*被调用。
* 无论本应用是否被启用，酷Q关闭前本函数都*不会*被调用。
*/
CQEVENT(int32_t, __eventDisable, 0)() {
	enabled = false;
	return 0;
}


/*
* Type=21 私聊消息
* subType 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char* msg, int32_t font) {

	//如果要回复消息，请调用酷Q方法发送，并且这里 return EVENT_BLOCK - 截断本条消息，不再继续处理  注意：应用优先级设置为"最高"(10000)时，不得使用本返回值
	//如果不回复消息，交由之后的应用/过滤器处理，这里 return EVENT_IGNORE - 忽略本条消息
	return EVENT_IGNORE;
}


/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char* fromAnonymous, char* msg, int32_t font) {
	if (is_NumInList(fromQQ, blockQQList))return EVENT_BLOCK;

	char msg_f[msg_maxLength];
	int msglen = strlen(msg);
	int index = 0;
	bool delhalfchar = false;

	for (int i = 0; i < msglen; i++) {
		//链接或淘口令是否结束，即半角字符串结束
		if (delhalfchar && msg[i] < 0) {
			//判断淘口令结束的￥符号
			if (i < msglen - 1 && msg[i] == -93 && msg[i + 1] == -92) {
				i++;
			}
			delhalfchar = false;
			continue;
		}
		if (delhalfchar)continue;
		//将“零”转换为'0'
		if (i < msglen - 1 && msg[i] == -63 && msg[i + 1] == -29) {
			msg[index++] = '0';
			i++;  continue;
		}
		//判断http(s)开头链接
		else if (i < msglen - 3 && msg[i] == 'h' && msg[i + 1] == 't' && msg[i + 2] == 't' && msg[i + 3] == 'p') {
			delhalfchar = true;
			continue;
		}
		//判断淘口令开头的￥符号
		else if (i < msglen - 2 && msg[i] == -93 && msg[i + 1] == -92) {
			delhalfchar = true;
			i++; continue;
		}
		//大写转小写
		if (msg[i] >= 'A' && msg[i] <= 'Z') {
			msg_f[index++] = msg[i] - 'A' + 'a';
			continue;
		}
		//其他字符保持原样
		else msg_f[index++] = msg[i];
	}
	msg_f[index++] = '\0';

	if (is_NumInList(fromGroup, xbGroupList)) {
		char groupName[20];
		switch (fromGroup) {
			//群名称，必须少于9个汉字
		case 945583797:strcpy_s(groupName, "哈佛大学线报"); break;
		case 367943101:strcpy_s(groupName, "三表哥线报群"); break;
		case 699788908:strcpy_s(groupName, "冷瞳活动分享"); break;
		case 782790346:strcpy_s(groupName, "Ben笨线报群"); break;
		case 970458851:strcpy_s(groupName, "King线报活动"); break;
		case 740897949:strcpy_s(groupName, "疯子vip共享"); break;
		case 707965661:strcpy_s(groupName, "debug群"); break;
		default:strcpy_s(groupName, to_string(fromGroup).c_str()); break;
		}
		auto p = KeyList;
		while (p->next != nullptr) {
			if ((p->key[0] == '/' && regex_search(msg_f, regex(p->key + 1))) || strstr(msg_f, p->key)) {
				char str[tmpstr_Length];
				sprintf_s(str, "[CQ:at,qq=%lld]\n线报“%s”来自%s\n%s", p->qq, p->key, groupName, msg);
				CQ_sendGroupMsg(ac, p->group, str);
			}
			p = p->next;
		}
	}
	//宿舍群管理相关处理
	if (fromGroup == 683750159) {
		if (bansaohuo && fromQQ == 593424943) {
			char str[tmpstr_Length];
			int time = 10;
			sprintf_s(str, "[CQ:at,qq=%lld]发言限1条/%d秒。\n已禁言%ds！", fromQQ, time, time);
			//CQ_sendPrivateMsg(ac, fromQQ, "骚货别逼逼！再逼逼永封！");
			CQ_setGroupBan(ac, fromGroup, fromQQ, time);
			CQ_sendGroupMsg(ac, fromGroup, str);
		}
		else if (fromQQ == 1224807613) {
			if (!strcmp(msg, "on")) {
				bansaohuo = true;
				CQ_sendGroupMsg(ac, fromGroup, "开启骚货禁言");
			}
			else if (!strcmp(msg, "off")) {
				bansaohuo = false;
				CQ_sendGroupMsg(ac, fromGroup, "关闭骚货禁言");
			}
		}
	}

	//线报关键词处理
	char command_add[] = "线报";
	char command_del[] = "移除线报";
	char command_list[] = "当前线报";
	if (strstr(msg, command_add) == msg) {//添加线报
		char str[tmpstr_Length];
		int t = addXianbaoKeyword(KeyList, msg + strlen(command_add), fromGroup, fromQQ);
		saveXianbaoKeyword(KeyList);
		switch (t) {
		case 0:sprintf_s(str, "[CQ:at,qq=%lld]\n已添加线报关键词“%s”，将转发线报至当前群，并通知QQ", fromQQ, msg + strlen(command_add)); break;
		case 1:sprintf_s(str, "添加线报失败，长度应为1至38字节，一个汉字占2字节"); break;
		case 2:sprintf_s(str, "与现有关键词有包含关系，已更新为较短关键词"); break;
		case 3:sprintf_s(str, "添加失败，黑名单词汇"); break;
		default:sprintf_s(str, "添加线报关键词失败，内部错误"); break;
		}
		CQ_sendGroupMsg(ac, fromGroup, str);
	}
	else if (strstr(msg, command_del) == msg) {//删除线报
		int t;
		char str[tmpstr_Length];
		if (!strcmp(msg + strlen(command_del), "全部"))t = delXianbaoKeyword(KeyList, "null", fromGroup, fromQQ, true);
		else t = delXianbaoKeyword(KeyList, msg + strlen(command_del), fromGroup, fromQQ);
		saveXianbaoKeyword(KeyList);
		switch (t) {
		case 0:sprintf_s(str, "[CQ:at,qq=%lld]\n已移除你在当前群的线报关键词“%s”", fromQQ, msg + strlen(command_del)); break;
		case 1:sprintf_s(str, "移除线报失败，长度应为1至38个字节，一个汉字占2字节"); break;
		case 2:sprintf_s(str, "未能找到你在当前群内追踪的线报关键词“%s”，可尝试发送“当前线报”来查询，或发送“移除线报全部”移除你在当前群内的所有追踪词", msg + strlen(command_del)); break;
		case 3:sprintf_s(str, "[CQ:at,qq=%lld]\n已移除你在当前群的全部线报关键词", fromQQ); break;
		default:sprintf_s(str, "移除线报关键词失败，内部错误"); break;
		}
		CQ_sendGroupMsg(ac, fromGroup, str);
	}
	else if (strstr(msg, command_list) == msg) {//查询当前线报
		auto p = KeyList;
		char str1[tmpstr_Length];
		sprintf_s(str1, "[CQ:at,qq=%lld]\n你在当前群的线报为：", fromQQ);
		string str2(str1);
		while (p->next != nullptr) {
			if (p->group == fromGroup && p->qq == fromQQ) {
				str2.append("\n");
				str2.append(p->key);
			}
			p = p->next;
		}
		CQ_sendGroupMsg(ac, fromGroup, str2.c_str());
	}
	/*if (fromGroup == 683750159) {
		CQ_setGroupCard(ac, fromGroup, 1974416999, "442搞比利");
	}*/
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=4 讨论组消息
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char* msg, int32_t font) {
	if (is_NumInList(fromQQ, blockQQList))return EVENT_BLOCK;
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=101 群事件-管理员变动
* subType 子类型，1/被取消管理员 2/被设置管理员
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=102 群事件-群成员减少
* subType 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
* fromQQ 操作者QQ(仅subType为2、3时存在)
* beingOperateQQ 被操作QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {
	if (fromGroup == 683750159) {
		char str[tmpstr_Length];
		switch (subType) {
		case 1:
			sprintf_s(str, "监控：%lld退出群聊\n来源：自行离开", beingOperateQQ);
			CQ_sendGroupMsg(ac, fromGroup, str);
			break;
		case 2:
			sprintf_s(str, "监控：%lld退出群聊\n来源：管理员%lld", beingOperateQQ, fromQQ);
			CQ_sendGroupMsg(ac, fromGroup, str);
			break;
		}
	}
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=103 群事件-群成员增加
* subType 子类型，1/管理员已同意 2/管理员邀请
* fromQQ 操作者QQ(即管理员QQ)
* beingOperateQQ 被操作QQ(即加群的QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {
	char str[tmpstr_Length];
	if (fromGroup != 723334097) {
		sprintf_s(str, "欢迎 [CQ:at,qq=%lld] 加群！\nQQ：%lld\n管理：%lld", beingOperateQQ, beingOperateQQ, fromQQ);
		CQ_sendGroupMsg(ac, fromGroup, str);
	}
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=201 好友事件-好友已添加
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {
	//CQ_sendPrivateMsg(ac, fromQQ, "你好吖，发送 任务列表 跟我玩游戏吧！");
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=301 请求-好友添加
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char* msg, const char* responseFlag) {

	//CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=302 请求-群添加
* subType 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char* msg, const char* responseFlag) {

	//if (subType == 1) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
	//} else if (subType == 2) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	//}

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* 菜单，可在 .json 文件中设置菜单数目、函数名
* 如果不使用菜单，请在 .json 及此处删除无用菜单
*/
CQEVENT(int32_t, __menuA, 0)() {
	MessageBoxA(NULL, "这是menuA，在这里载入窗口，或者进行其他工作。", "", 0);
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
	MessageBoxA(NULL, "这是menuB，在这里载入窗口，或者进行其他工作。", "", 0);
	return 0;
}
