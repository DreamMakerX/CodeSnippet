#ifndef MSTRING_SLOGGER_HPP
#define MSTRING_SLOGGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <mutex>

//SLogger& _logger = SLogger::getInstance();

class SLogger {
private:
	std::ofstream file_;
	std::string filename_;
	static const size_t MAX_BUFFER_SIZE_ = 1024; // 1KB，单次输出的最大长度
	static const size_t MAX_FILE_SIZE_ = 50 * 1024 * 1024; // 50MB，日志文件最大长度
	std::mutex logMutex_;

	// 构造函数私有化
	SLogger(const std::string& filename = "logs/log.txt") : filename_(filename) {
		openLogFile();
	}

	// 私有化拷贝构造、赋值、移动构造、移动赋值
	SLogger(const SLogger&);
	SLogger& operator=(const SLogger&);
	SLogger(SLogger&&);
	SLogger& operator=(SLogger&&);

	// 获取当前时间的字符串
	std::string getCurrentTime() {
		time_t now = time(NULL);
		struct tm* tm_info = localtime(&now);

		char timeStr[20];
		strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);

		return std::string(timeStr);
	}

	// 检查日志文件大小，超过最大值时重新创建日志文件
	void checkLogSize() {
		struct stat fileStatus;
		if (stat(filename_.c_str(), &fileStatus) == 0) {
			if (fileStatus.st_size > MAX_FILE_SIZE_) {
				file_.close();
				remove(filename_.c_str());
				openLogFile();
			}
		}
	}

	// 打开日志文件
	void openLogFile() {
		file_.open(filename_.c_str(), std::ios::out | std::ios::app);
	}

public:
	static SLogger& getInstance() {
		static SLogger instance;  // 静态局部变量
		return instance;
	}

	~SLogger() {
		if (file_.is_open()) {
			file_.close();
		}
	}

	// 打印日志（可变参数）
	void log(const char* format, ...) {
		std::lock_guard<std::mutex> lock(logMutex_);

		checkLogSize();

		file_ << "[" << getCurrentTime() << "] ";

		va_list args;
		va_start(args, format);

		char buffer[MAX_BUFFER_SIZE_];
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		buffer[MAX_BUFFER_SIZE_ - 1] = 0x00;  // 防止打印溢出

		file_ << buffer << std::endl;
	}

	// 打印日志
	void log(const std::string& message) {
		std::lock_guard<std::mutex> lock(logMutex_);

		checkLogSize();

		file_ << "[" << getCurrentTime() << "] " << message << std::endl;
	}
};

#endif //MSTRING_SLOGGER_HPP
