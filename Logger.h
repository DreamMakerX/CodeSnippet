#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <atomic>
#include <condition_variable>

class Logger {
public:
	enum class LogLevel {
		DEBUG,
		INFO,
		WARNING,
		ERROR
	};

	// 构造函数
	Logger(const std::string& baseName, LogLevel level = LogLevel::INFO, bool async = false, uint64_t logCycle = 10,
	       bool daily = false, int retentionDays = 30, size_t maxSize = 50 * 1024 * 1024);

	// 析构函数
	~Logger();

	// 设置日志级别
	void setLogLevel(LogLevel level);

	// 同步日志
	void log(const std::string& message, LogLevel level = LogLevel::INFO);

	// 同步日志
	void log(const char* message, LogLevel level = LogLevel::INFO);

	// 同步日志（可变参数）
	void log(LogLevel level, const char* format, ...);
private:
	std::string foldername_;
	LogLevel logLevel_;
	bool async_;
	bool daily_;
	std::atomic<bool> exit_;
	int retentionDays_;
	size_t maxSize_;
	std::ofstream logFile_;
	std::thread logThread_;
	std::thread checkThread_;
	std::mutex logMutex_;
	std::mutex logQueueMutex_;
	std::deque<std::string> logQueue_;
	size_t fileSize_;
	int currentFileIndex_; // 每天或每小时的文件编号
	std::chrono::seconds logCycle_;// 日志刷新周期，单位s

	// 获取当前时间的字符串格式
	std::string getCurrentTime() const;

	// 获取当前日期和小时
	std::string getCurrentDateHour() const;

	// 获取日志文件名，基于当前时间和文件编号
	std::string getLogFileName() const;

	// 将日志消息写入文件
	void writeToFile(const std::string& message);

	// 清理过期的日志文件
	void cleanOldLogs() const;

	// 获取当前最大序号
	int getMaxLogSequence();

	// 重置文件编号
	void resetFileIndex();

	// 异步线程工作函数
	void logThreadFunction();

	//确保在关机前写入所有剩余日志
	void flush();

	// 检测线程工作函数
	void checkThreadFunction();

	// 将日志级别转换为字符串
	std::string logLevelToString(LogLevel level) const;

	// 返回 Unix 纪元时间，精确到毫秒
	static uint64_t getCurrentTimeMillis();

	// 周期执行任务
	template <typename Func>
	static void ExecuteTaskPeriodically(uint64_t & lastTime, uint64_t cycle, Func func) {
		uint64_t nowTime = getCurrentTimeMillis();
		if (nowTime > lastTime + cycle) {
			func();
			lastTime = nowTime;
		}
	}
};

#endif // LOGGER_H
