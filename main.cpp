#include <chrono>
#include <thread>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <functional>
#include <unordered_map>
#include "MString.h"
#include "ClubMember.h"
#include "Logger.h"
#include "SLogger.hpp"

void MStringFullTest() {
	// 创建成员
	ClubMember zcc(1, "congcong.zhang", 18, "hedong, tianjin", "28810331651");
	ClubMember xyx(2, "yexin.xie", 18, "yubei, chongqing", "28810331652");
	ClubMember cpa(3, "pingan.chen", 18, "yuanyang, henan", "28810331653");

	// 创建容器
	std::unordered_map<int, ClubMember> clubMembers;
	clubMembers.insert(std::make_pair(zcc.getID(), zcc));
	clubMembers.insert(std::make_pair(xyx.getID(), xyx));
	clubMembers.insert(std::make_pair(cpa.getID(), cpa));

	// 修改属性
	zcc.showMessage();
	zcc.setPhone("28810331660");
	zcc.showMessage();

	// 过滤信息
	for(auto member : clubMembers) {
		if(member.second.getAddr().contains("yuanyang")) {
			member.second.showMessage();
		}
	}

	// MString类型转换
	long long phone = zcc.getPhone();// MString -> 数值
	MString newPhone = phone + 1000;// 数值 -> MString 构造时赋值，等价与MString newPhone(phone + 1000);
	newPhone = phone + 2000;// 数值 -> MString
	std::string stdPhone = newPhone.toStdString();// MString -> std::string
	newPhone = MString::fromStdString(stdPhone);// std::string -> MString
	zcc.setPhone(newPhone.getData());
	zcc.showMessage();

	// MString格式化
	MString addr = MString::format("{}, Beijing {}", "chaoyang", 19);
	zcc.setAddr(addr.getData());
	zcc.showMessage();

	// MString拼接
	addr += ", china ";
	zcc.setAddr(addr.getData());
	zcc.showMessage();

	// MString去空格
	addr = addr.trim();
	zcc.setAddr(addr.getData());
	zcc.showMessage();

	// 查找子串位置
	addr += ", china ";
	int substrPos = addr.find("china");
	substrPos = addr.rfind("china");

	// 替换子串
	addr = addr.replace("china", "america");

	// 大小写转换
	addr = addr.toUpperCase();
	addr = addr.toLowerCase();

	// 截取子串
	MString leftSubstr = addr.left(5);
	MString rightSubstr = addr.right(5);
	MString midSubstr = addr.mid(5, 3);

	// 分割&拼接
	std::vector<MString> splitStrs = addr.split(" ");
	splitStrs.emplace_back("nanshan");
	addr = MString::join(splitStrs, " # ");

	// 比较
	int compareRes = addr.compareTo(addr);
	compareRes = addr.compareTo("abc");
	compareRes = addr.compareTo("zzz");

	// 获取编码，以16进制字符串表示
	MString showCode = "你好，世界";
	MString hexCode = showCode.getHexString();
	showCode = "125";
	hexCode = showCode.getHexString();
}

void sleep(int milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// 返回当前时间的时间戳，精确到毫秒
uint64_t getCurrentTimeMillis() {
	using namespace std::chrono;
	// 获取当前时间点
	auto now = system_clock::now();
	// 转换为time_point_cast，以毫秒为单位
	auto duration = now.time_since_epoch();
	// 将duration转换为毫秒
	auto millis = duration_cast<milliseconds>(duration).count();
	return millis;
}

// 性能能测试
template <typename Func>
void performanceTest(Func&& func, uint64_t count = 1000000) {
	uint64_t startTime = getCurrentTimeMillis();

	// 执行传入的函数
	for (auto i = 0; i < count; ++i) {
		func();  // 直接调用传入的函数
	}

	uint64_t stopTime = getCurrentTimeMillis();
	std::cout << MString::format("The execution of {} times takes {} milliseconds | {} times per millisecond",
	                             count, stopTime - startTime, count / (stopTime - startTime)) << std::endl;
}

void loggerPerformanceTest() {
	// 同步/异步日志测试
	Logger logger("ClionProjectLogs", Logger::LogLevel::INFO, true);
	auto loggerLambda = [&logger]() {
		return logger.log("Hello World", Logger::LogLevel::INFO);
	};
	for(auto i = 0; i < 10; ++i) {
		performanceTest(loggerLambda);
	}

//	std::string hello = "Hello World";
//	SLogger& logger = SLogger::getInstance();
//	auto loggerLambda = [&logger, &hello]() {
//		return logger.log(hello);
//	};
//	performanceTest(loggerLambda);

//	// Logger测试
//	Logger logger("ClionProject", Logger::LogLevel::INFO, true);
//	logger.setLogLevel(Logger::LogLevel::DEBUG);
//	while(true) {
//		sleep(15000);
//		static int i = 0;
//		logger.log(MString::format("Get Tick Count: {}", (++i) * 15).getData(), Logger::LogLevel::INFO);
//	}
}

void stringFormatPerformanceTest() {
	// MString format性能测试
	auto formatLambda = []() {
		return MString::format("{} {}: {}", "Set", "Index", 123);  // 执行格式化
	};
	performanceTest(formatLambda);
}

int main() {
	stringFormatPerformanceTest();
	return 0;
}