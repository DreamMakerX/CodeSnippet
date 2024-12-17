#ifndef MSTRING_CLUBMEMBER_H
#define MSTRING_CLUBMEMBER_H
#include <iostream>
#include "MString.h"

class ClubMember {
public:
	// 构造函数
	ClubMember(int id, const char* name, int age, const char* addr, const char* phone, int level = 1);

	// 成员函数
	bool isEmpty(const char* str);
	void showMessage() const;
	bool setName(const char* name);
	bool setAge(int age);
	bool setAddr(const char* addr);
	bool setPhone(const char* phone);
	bool setLevel(int level);
	bool setInfo(const char* name, int age, const char* addr, const char* phone, int level);

	// 获取函数
	int getID();
	MString getName();
	int getAge();
	MString getAddr();
	MString getPhone();
	int getLevel();
	MString getUpdateTime();

private:
	// 成员变量
	int id_;
	MString name_;
	int age_;
	MString addr_;
	MString phone_;
	int level_;
	MString updateTime_;
};

#endif //MSTRING_CLUBMEMBER_H
