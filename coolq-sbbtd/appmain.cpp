/*
* CoolQ Demo for VC++
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/

#include "stdafx.h"
#include "string"
#include "cqp.h"
#include "appmain.h" //Ӧ��AppID����Ϣ������ȷ��д�������Q�����޷�����
#include "sbbtd.h"

using namespace std;

int ac = -1; //AuthCode ���ÿ�Q�ķ���ʱ��Ҫ�õ�
bool enabled = false;

bool bansaohuo = false;

//����������-1��ʾ�����β
int64_t blockQQList[blockQQList_Size];
int64_t xbGroupList[xbGroupList_Size];

monitorKeyList KeyList = new monitorKey;

/*
* ����Ӧ�õ�ApiVer��Appid������󽫲������
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/*
* ����Ӧ��AuthCode����Q��ȡӦ����Ϣ��������ܸ�Ӧ�ã���������������������AuthCode��
* ��Ҫ�ڱ��������������κδ��룬���ⷢ���쳣���������ִ�г�ʼ����������Startup�¼���ִ�У�Type=1001����
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 ��Q����
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q������ִ��һ�Σ���������ִ��Ӧ�ó�ʼ�����롣
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
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
		char key[20];
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
* Type=1002 ��Q�˳�
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q�˳�ǰִ��һ�Σ���������ִ�в���رմ��롣
* ������������Ϻ󣬿�Q���ܿ�رգ��벻Ҫ��ͨ���̵߳ȷ�ʽִ���������롣
*/
CQEVENT(int32_t, __eventExit, 0)() {

	return 0;
}

/*
* Type=1003 Ӧ���ѱ�����
* ��Ӧ�ñ����ú󣬽��յ����¼���
* �����Q����ʱӦ���ѱ����ã�����_eventStartup(Type=1001,��Q����)�����ú󣬱�����Ҳ��������һ�Ρ�
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventEnable, 0)() {
	enabled = true;
	return 0;
}


/*
* Type=1004 Ӧ�ý���ͣ��
* ��Ӧ�ñ�ͣ��ǰ�����յ����¼���
* �����Q����ʱӦ���ѱ�ͣ�ã��򱾺���*����*�����á�
* ���۱�Ӧ���Ƿ����ã���Q�ر�ǰ��������*����*�����á�
*/
CQEVENT(int32_t, __eventDisable, 0)() {
	enabled = false;
	return 0;
}


/*
* Type=21 ˽����Ϣ
* subType �����ͣ�11/���Ժ��� 1/��������״̬ 2/����Ⱥ 3/����������
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t msgId, int64_t fromQQ, const char* msg, int32_t font) {

	//���Ҫ�ظ���Ϣ������ÿ�Q�������ͣ��������� return EVENT_BLOCK - �ضϱ�����Ϣ�����ټ�������  ע�⣺Ӧ�����ȼ�����Ϊ"���"(10000)ʱ������ʹ�ñ�����ֵ
	//������ظ���Ϣ������֮���Ӧ��/�������������� return EVENT_IGNORE - ���Ա�����Ϣ
	return EVENT_IGNORE;
}


/*
* Type=2 Ⱥ��Ϣ
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t msgId, int64_t fromGroup, int64_t fromQQ, const char* fromAnonymous, char* msg, int32_t font) {
	if (is_NumInList(fromQQ, blockQQList))return EVENT_BLOCK;

	char msg_f[msg_maxLength];
	int msglen = strlen(msg);
	int index = 0;
	bool delhalfchar = false;

	for (int i = 0; i < msglen; i++) {
		//���ӻ��Կ����Ƿ������������ַ�������
		if (delhalfchar && msg[i] < 0) {
			//�ж��Կ�������ģ�����
			if (i < msglen - 1 && msg[i] == -93 && msg[i + 1] == -92) {
				delhalfchar = false;
				i++; continue;
			}
			delhalfchar = false;
		}
		if (delhalfchar)continue;
		//�����㡱ת��Ϊ'0'
		if (i < msglen - 1 && msg[i] == -63 && msg[i + 1] == -29) {
			msg[index++] = '0';
			continue;
		}
		//�ж�http(s)��ͷ����
		else if (i < msglen - 3 && msg[i] == 'h' && msg[i + 1] == 't' && msg[i + 2] == 't' && msg[i + 3] == 'p') {
			delhalfchar = true;
			continue;
		}
		//�ж��Կ��ͷ�ģ�����
		else if (i < msglen - 2 && msg[i] == -93 && msg[i + 1] == -92) {
			delhalfchar = true;
			i++; continue;
		}
		//��дתСд
		if (msg[i] >= 'A' && msg[i] <= 'Z') {
			msg_f[index++] = msg[i] + 'a' - 'A';
			continue;
		}
		//�����ַ�����ԭ��
		else msg_f[index++] = msg[i];
	}
	msg_f[index++] = '\0';

	if (is_NumInList(fromGroup, xbGroupList)) {
		char groupName[20];
		switch (fromGroup) {
		//Ⱥ���ƣ���������9������
		case 945583797:strcpy_s(groupName, "�����ѧ�߱�"); break;
		case 367943101:strcpy_s(groupName, "������߱�Ⱥ"); break;
		case 699788908:strcpy_s(groupName, "��ͫ�����"); break;
		case 782790346:strcpy_s(groupName, "Ben���߱�Ⱥ"); break;
		case 970458851:strcpy_s(groupName, "King�߱��"); break;
		case 740897949:strcpy_s(groupName, "����vip����"); break;
		case 707965661:strcpy_s(groupName, "debug��Ⱥ"); break;
		default:strcpy_s(groupName, to_string(fromGroup).c_str()); break;
		}
		auto p = KeyList;
		while (p->next != nullptr) {
			if (strstr(msg_f, p->key)) {
				char str[tmpstr_Length];
				sprintf_s(str, "[CQ:at,qq=%lld]\n�߱���%s������%s\n%s", p->qq, p->key, groupName, msg);
				CQ_sendGroupMsg(ac, p->group, str);
			}
			p = p->next;
		}
	}
	//����Ⱥ������ش���
	if (fromGroup == 683750159) {
		if (bansaohuo && fromQQ == 593424943) {
			char str[tmpstr_Length];
			int time = 10;
			sprintf_s(str, "[CQ:at,qq=%lld]������1��/%d�롣\n�ѽ���%ds��", fromQQ, time, time);
			//CQ_sendPrivateMsg(ac, fromQQ, "ɧ����Ʊƣ��ٱƱ����⣡");
			CQ_setGroupBan(ac, fromGroup, fromQQ, time);
			CQ_sendGroupMsg(ac, fromGroup, str);
		}
		else if (fromQQ == 1224807613) {
			if (!strcmp(msg, "on")) {
				bansaohuo = true;
				CQ_sendGroupMsg(ac, fromGroup, "����ɧ������");
			}
			else if (!strcmp(msg, "off")) {
				bansaohuo = false;
				CQ_sendGroupMsg(ac, fromGroup, "�ر�ɧ������");
			}
		}
	}

	//�߱��ؼ��ʴ���
	char command_add[10] = "�߱�";
	char command_del[10] = "�Ƴ��߱�";
	char command_list[10] = "��ǰ�߱�";
	if (strstr(msg, command_add) == msg) {//����߱�
		char str[tmpstr_Length];
		int t = addXianbaoKeyword(KeyList, msg + strlen(command_add), fromGroup, fromQQ);
		saveXianbaoKeyword(KeyList);
		switch (t) {
		case 0:sprintf_s(str, "[CQ:at,qq=%lld]\n������߱��ؼ��ʡ�%s������ת���߱�����ǰȺ����֪ͨQQ", fromQQ, msg + strlen(command_add)); break;
		case 1:sprintf_s(str, "����߱��ؼ���ʧ�ܣ�ӦΪ����1��8��"); break;
		case 2:sprintf_s(str, "�����йؼ����а�����ϵ���Ѹ���Ϊ�϶̹ؼ���"); break;
		case 3:sprintf_s(str, "���ʧ�ܣ��������ʻ�"); break;
		default:sprintf_s(str, "����߱��ؼ���ʧ�ܣ��ڲ�����"); break;
		}
		CQ_sendGroupMsg(ac, fromGroup, str);
	}
	else if (strstr(msg, command_del) == msg) {//ɾ���߱�
		int t;
		char str[tmpstr_Length];
		if (!strcmp(msg + strlen(command_del), "ȫ��"))t = delXianbaoKeyword(KeyList, "null", fromGroup, fromQQ, true);
		else t = delXianbaoKeyword(KeyList, msg + strlen(command_del), fromGroup, fromQQ);
		saveXianbaoKeyword(KeyList);
		switch (t) {
		case 0:sprintf_s(str, "[CQ:at,qq=%lld]\n���Ƴ����ڵ�ǰȺ���߱��ؼ��ʡ�%s��", fromQQ, msg + strlen(command_del)); break;
		case 1:sprintf_s(str, "�Ƴ��߱��ؼ���ʧ�ܣ�ӦΪ����1��8��"); break;
		case 2:sprintf_s(str, "δ���ҵ����ڵ�ǰȺ��׷�ٵ��߱��ؼ��ʡ�%s�����ɳ��Է��͡��Ƴ��߱�ȫ�����Ƴ����ڵ�ǰȺ�ڵ�����׷�ٴ�", msg + strlen(command_del)); break;
		case 3:sprintf_s(str, "[CQ:at,qq=%lld]\n���Ƴ����ڵ�ǰȺ��ȫ���߱��ؼ���", fromQQ); break;
		default:sprintf_s(str, "�Ƴ��߱��ؼ���ʧ�ܣ��ڲ�����"); break;
		}
		CQ_sendGroupMsg(ac, fromGroup, str);
	}
	else if (strstr(msg, command_list) == msg) {//��ѯ��ǰ�߱�
		auto p = KeyList;
		char str1[tmpstr_Length];
		sprintf_s(str1, "[CQ:at,qq=%lld]\n���ڵ�ǰȺ���߱�Ϊ��", fromQQ);
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
		CQ_setGroupCard(ac, fromGroup, 1974416999, "442�����");
	}*/
	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=4 ��������Ϣ
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t msgId, int64_t fromDiscuss, int64_t fromQQ, const char* msg, int32_t font) {
	if (is_NumInList(fromQQ, blockQQList))return EVENT_BLOCK;
	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=101 Ⱥ�¼�-����Ա�䶯
* subType �����ͣ�1/��ȡ������Ա 2/�����ù���Ա
*/
CQEVENT(int32_t, __eventSystem_GroupAdmin, 24)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t beingOperateQQ) {

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=102 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/ȺԱ�뿪 2/ȺԱ���� 3/�Լ�(����¼��)����
* fromQQ ������QQ(��subTypeΪ2��3ʱ����)
* beingOperateQQ ������QQ
*/
CQEVENT(int32_t, __eventSystem_GroupMemberDecrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {
	if (fromGroup == 683750159) {
		char str[tmpstr_Length];
		switch (subType) {
		case 1:
			sprintf_s(str, "��أ�%lld�˳�Ⱥ��\n��Դ�������뿪", beingOperateQQ);
			CQ_sendGroupMsg(ac, fromGroup, str);
			break;
		case 2:
			sprintf_s(str, "��أ�%lld�˳�Ⱥ��\n��Դ������Ա%lld", beingOperateQQ, fromQQ);
			CQ_sendGroupMsg(ac, fromGroup, str);
			break;
		}
	}
	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=103 Ⱥ�¼�-Ⱥ��Ա����
* subType �����ͣ�1/����Ա��ͬ�� 2/����Ա����
* fromQQ ������QQ(������ԱQQ)
* beingOperateQQ ������QQ(����Ⱥ��QQ)
*/
CQEVENT(int32_t, __eventSystem_GroupMemberIncrease, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, int64_t beingOperateQQ) {
	char str[tmpstr_Length];
	sprintf_s(str, "��ӭ [CQ:at,qq=%lld] ��Ⱥ��\nQQ��%lld\n����%lld", beingOperateQQ, beingOperateQQ, fromQQ);
	CQ_sendGroupMsg(ac, fromGroup, str);
	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=201 �����¼�-���������
*/
CQEVENT(int32_t, __eventFriend_Add, 16)(int32_t subType, int32_t sendTime, int64_t fromQQ) {
	CQ_sendPrivateMsg(ac, fromQQ, "���߹������ �����б� ��������Ϸ�ɣ�");
	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=301 ����-�������
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char* msg, const char* responseFlag) {

	//CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=302 ����-Ⱥ���
* subType �����ͣ�1/����������Ⱥ 2/�Լ�(����¼��)������Ⱥ
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char* msg, const char* responseFlag) {

	//if (subType == 1) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPADD, REQUEST_ALLOW, "");
	//} else if (subType == 2) {
	//	CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	//}

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* �˵������� .json �ļ������ò˵���Ŀ��������
* �����ʹ�ò˵������� .json ���˴�ɾ�����ò˵�
*/
CQEVENT(int32_t, __menuA, 0)() {
	MessageBoxA(NULL, "����menuA�����������봰�ڣ����߽�������������", "", 0);
	return 0;
}

CQEVENT(int32_t, __menuB, 0)() {
	MessageBoxA(NULL, "����menuB�����������봰�ڣ����߽�������������", "", 0);
	return 0;
}
