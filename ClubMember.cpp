#include "ClubMember.h"

// 构造函数定义
ClubMember::ClubMember(int id, const char* name, int age, const char* addr, const char* phone, int level)
	: id_(id), name_(name), age_(age), addr_(addr), phone_(phone), level_(level) {
	updateTime_ = MString::getCurrentTime();
}

// isEmpty函数定义
bool ClubMember::isEmpty(const char* str) {
	return str == nullptr || str[0] == '\0';
}

// showMessage函数定义
void ClubMember::showMessage() const {
	std::cout << "-----------------------------------------------------" << std::endl;
	std::cout << "# ClubMember infomation of id(" << id_ << "):" << std::endl;
	std::cout << "name: " << name_ << std::endl;
	std::cout << "age: " << age_ << std::endl;
	std::cout << "address: " << addr_ << std::endl;
	std::cout << "telephone number: " << phone_ << std::endl;
	std::cout << "skill level: " << level_ << std::endl;
	std::cout << "update time: " << updateTime_ << std::endl;
	std::cout << "-----------------------------------------------------" << std::endl;
}

// setName函数定义
bool ClubMember::setName(const char* name) {
	if (isEmpty(name)) {
		return false;
	}
	name_ = name;
	updateTime_ = MString::getCurrentTime();
	return true;
}

// setAge函数定义
bool ClubMember::setAge(int age) {
	if (age < 0) {
		return false;
	}
	age_ = age;
	updateTime_ = MString::getCurrentTime();
	return true;
}

// setAddr函数定义
bool ClubMember::setAddr(const char* addr) {
	if (isEmpty(addr)) {
		return false;
	}
	addr_ = addr;
	updateTime_ = MString::getCurrentTime();
	return true;
}

// setPhone函数定义
bool ClubMember::setPhone(const char* phone) {
	if (isEmpty(phone)) {
		return false;
	}
	phone_ = phone;
	updateTime_ = MString::getCurrentTime();
	return true;
}

// setLevel函数定义
bool ClubMember::setLevel(int level) {
	if (level < 0 || level > 100) {
		return false;
	}
	level_ = level;
	updateTime_ = MString::getCurrentTime();
	return true;
}

// setInfo函数定义
bool ClubMember::setInfo(const char* name, int age, const char* addr, const char* phone, int level) {
	return setName(name) && setAge(age) && setAddr(addr) && setPhone(phone) && setLevel(level);
}

// getID函数定义
int ClubMember::getID() {
	return id_;
}

// getName函数定义
MString ClubMember::getName() {
	return name_;
}

// getAge函数定义
int ClubMember::getAge() {
	return age_;
}

// getAddr函数定义
MString ClubMember::getAddr() {
	return addr_;
}

// getPhone函数定义
MString ClubMember::getPhone() {
	return phone_;
}

// getLevel函数定义
int ClubMember::getLevel() {
	return level_;
}

// getUpdateTime函数定义
MString ClubMember::getUpdateTime() {
	return updateTime_;
}