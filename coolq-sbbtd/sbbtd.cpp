#include "stdafx.h"
#include "sbbtd.h"

/*检测文件是否存在并返回读文件流*/
fstream* openfile(const char* filename)
{
	ofstream* ofs = new ofstream(filename, ios::app);
	if (!ofs->is_open())return nullptr;
	else ofs->close();
	delete ofs;
	fstream* filest = new fstream(filename, ios::in);
	if (!filest->is_open())return nullptr;
	filest->seekg(ios::beg);
	return filest;
}

/*读单列数字文件*/
int readfile(fstream* stm, int64_t* list)
{
	int n = 0;
	while (!stm->eof()) {
		int64_t data = 0;
		*stm >> data;
		if (data > 0)list[n++] = data;
	}
	list[n] = -1;
	stm->close();
	delete stm;
	return n;
}

/*生成写入文件流*/
ofstream* writefile(const char* filename)
{
	ofstream* ofs = new ofstream(filename, ios::app);
	if (!ofs->is_open())return nullptr;
	else ofs->close();
	delete ofs;
	ofs = new ofstream(filename, ios::out);
	if (!ofs->is_open())return nullptr;
	return ofs;
}

/*检测数字是否在某以-1结束的数字数组中*/
bool is_NumInList(int64_t qq, int64_t* list)
{
	int n = 0;
	while (list[n] != -1)
		if (qq == list[n++])
			return true;
	return false;
}

/*判断一个字符是否是字母或数字（A-Z,a-z,0-9）*/
bool is_abc123(char chr)
{
	if ((chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z') || (chr >= '0' && chr <= '9'))return true;
	return false;
}

/*弃用
monitorKey * getMonitor(monitorKeyList list, char * key, char* msg)
{
	monitorKey* p = list;
	while (p->next != nullptr) {
		if (strstr(msg, p->key)) {
			return p;
		}

	}
	return nullptr;
}*/

/*
添加线报关键词
返回值说明
0-成功
1-长度错误
2-有包含关系且已更新
3-黑名单词汇
default-内部错误
*/
int addXianbaoKeyword(monitorKeyList list, char* key, int64_t group, int64_t qq)
{
	if (strlen(key) > keyword_Length - 2 || strlen(key) < 1)return 1;
	if (!strcmp(key, "全部"))return 3;
	monitorKey* p = list;
	while (p->next != nullptr) {
		if (p->group == group && p->qq == qq) {
			if (strstr(p->key, key)) {
				strcpy(p->key, key);
				return 2;
			}
			else if (strstr(key, p->key)) {
				return 2;
			}
		}
		p = p->next;
	}
	if (group > 0 && qq > 0) {
		strcpy(p->key, key);
		p->group = group;
		p->qq = qq;
		p->next = new monitorKey;
		p->next->pre = p;
		p = p->next;
		p->group = p->qq = 0;
		p->key[0] = 0;
		p->next = nullptr;
	}
	else return -1;
	return 0;
}

/*删除线报，delall=true时忽略key参数，此时请任意传入一个char*
返回值说明
0-移除单个成功
1-长度错误
2-未找到关键词
3-移除全部成功
default-内部错误
*/
int delXianbaoKeyword(monitorKeyList& list, char* key, int64_t group, int64_t qq, bool delall)
{
	if (strlen(key) > keyword_Length - 2 || strlen(key) < 1)return 1;
	int rtn = 2;
	auto p = list;
	while (p->next != nullptr) {
		//判断节点是否需要删除
		if (p->group == group && p->qq == qq && (delall || !strcmp(p->key, key))) {
			if (p->pre) {
				p->pre->next = p->next;
				p->next->pre = p->pre;
				auto q = p;
				p = p->next;
				delete q;
			}
			else {
				list = p->next;
				p->next->pre = nullptr;
				delete p;
				p = list;
			}
			rtn = 0;
		}
		else {
			p = p->next;
		}
	}
	return delall ? 3 : rtn;
}

/*将线报链表写入文件*/
int saveXianbaoKeyword(monitorKeyList list)
{
	auto p = list;
	ofstream* ofs = writefile(iniFileName_monitor);
	if (ofs) {
		while (p->next != nullptr) {
			if (p->group > 0 && p->qq > 0) {
				*ofs << p->key << ' ' << p->group << ' ' << p->qq << '\n';
			}
			p = p->next;
		}
	}
	ofs->close();
	delete ofs;
	return 0;
}
