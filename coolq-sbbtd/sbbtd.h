#pragma once

#include <stdint.h>
#include <fstream>
using namespace std;

#define iniFileName_blockqq "settings_blockqq.ini"
#define iniFileName_xianbaogroup "settings_xianbaogroup.ini"
#define iniFileName_monitor "settings_monitor.ini"
#define blockQQList_Size 256
#define xbGroupList_Size 256
#define tmpstr_Length 1000

/*线报链表节点*/
struct monitorKey {
	char key[20];
	int64_t group;
	int64_t qq;
	struct monitorKey* pre;
	struct monitorKey* next;
};
typedef monitorKey* monitorKeyList;

fstream* openfile(const char* filename);
int readfile(fstream* stm, int64_t* list);
ofstream* writefile(const char* filename);

bool is_NumInList(int64_t qq, int64_t * list);

int addXianbaoKeyword(monitorKeyList list, char*keyword, int64_t fromGroup, int64_t fromQQ);
int delXianbaoKeyword(monitorKeyList &list, char* key, int64_t group, int64_t qq, bool delall = false);
int saveXianbaoKeyword(monitorKeyList list);