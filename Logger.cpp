#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>
#include <functional>
#include <cstdarg>
#include <regex>

Logger::Logger(const std::string& baseName, LogLevel level, bool async, uint64_t logCycle, bool daily, int retentionDays, size_t maxSize)
	: foldername_(baseName), logLevel_(level), async_(async), logCycle_(logCycle),
	  daily_(daily), retentionDays_(retentionDays), maxSize_(maxSize), fileSize_(0), exit_(false),
	  currentFileIndex_(getMaxLogSequence() + 1) {

	if (async_) {
		logThread_ = std::thread(&Logger::logThreadFunction, this);
	}

	checkThread_ = std::thread(&Logger::checkThreadFunction, this);
}

Logger::~Logger() {
	exit_ = true;
	if (async_ && logThread_.joinable()) {
		logThread_.join();
	}
	if (checkThread_.joinable()) {
		checkThread_.join();
	}
}

void Logger::setLogLevel(LogLevel level) {
	logLevel_ = level;
}

void Logger::log(const std::string& message, LogLevel level) {
	log(message.c_str(), level);
}

void Logger::log(const char* message, LogLevel level) {
	if (level < logLevel_ || message == nullptr) return;

	std::stringstream logStream;
	logStream << "[" << getCurrentTime() << " " << logLevelToString(level) << "] " << message;

	if (async_) {
		{
			std::lock_guard<std::mutex> lock(logQueueMutex_);
			logQueue_.push_back(logStream.str());
		}
	} else {
		writeToFile(logStream.str());
	}
}

void Logger::log(LogLevel level, const char* format, ...) {

	va_list args;
	va_start(args, format);

	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	buffer[1024 - 1] = 0x00;  // 防止打印溢出

	log(buffer, level);
}

std::string Logger::getCurrentTime() const {
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto duration = now_ms.time_since_epoch();
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) - std::chrono::duration_cast<std::chrono::seconds>(duration);

	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
	ss << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();
	return ss.str();
}

std::string Logger::getCurrentDateHour() const {
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	std::tm tm = *std::localtime(&time);
	std::stringstream ss;
	if (daily_) {
		ss << std::put_time(&tm, "%Y%m%d");
	} else {
		ss << std::put_time(&tm, "%Y%m%d%H");
	}
	return ss.str();
}

std::string Logger::getLogFileName() const {
	std::stringstream fileName;
	fileName << foldername_ << "/" << getCurrentDateHour() << "_" << currentFileIndex_ << ".log";
	return fileName.str();
}

void Logger::writeToFile(const std::string& message) {
	std::lock_guard<std::mutex> lock(logMutex_);
	if (!logFile_.is_open()) {
		logFile_.open(getLogFileName(), std::ios::out | std::ios::app);
	}

	if (logFile_.is_open()) {
		logFile_ << message << std::endl;
		fileSize_ += message.size() + 2;

		if (fileSize_ >= maxSize_) {
			logFile_.close();
			currentFileIndex_++;
			fileSize_ = 0;
			logFile_.open(getLogFileName(), std::ios::out | std::ios::app);
		}
	}
}

void Logger::cleanOldLogs() const {
	std::string logDir = foldername_ + "/";
	DIR* dir = opendir(logDir.c_str());
	if (dir == nullptr) {
		std::cerr << "Unable to open<<" << foldername_ << ">> directory!" << std::endl;
		return;
	}

	struct dirent * entry;
	while ((entry = readdir(dir)) != nullptr) {
		// 只处理日志文件
		std::string fileName = entry->d_name;
		if (fileName.find("_") != std::string::npos && fileName.find(".log") != std::string::npos) {
			std::string fullPath = logDir + fileName;
			struct stat fileStat;
			if (stat(fullPath.c_str(), &fileStat) == 0) {
				std::time_t lastModifiedTime = fileStat.st_mtime;
				std::chrono::system_clock::time_point fileTimePoint = std::chrono::system_clock::from_time_t(lastModifiedTime);
				auto duration = std::chrono::system_clock::now() - fileTimePoint;
				int daysOld = std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24;

				if (daysOld > retentionDays_) {
					std::cout << "Deleting old log file: " << fullPath << std::endl;
					remove(fullPath.c_str());
				}
			}
		}
	}
	closedir(dir);
}

int Logger::getMaxLogSequence() {
	int maxSequence = -1;  // -1 表示没有找到符合条件的日志文件
	std::string currentDateHour = getCurrentDateHour();
	DIR* dir = opendir(foldername_.c_str());

	if (dir == nullptr) {
		std::cerr << "Failed to open directory: " << foldername_ << std::endl;
		return -1;
	}

	struct dirent * entry;
	std::regex pattern(currentDateHour + R"(_(\d+)\.log)");  // 正则表达式：日期+_+序号.log

	// 遍历目录中的所有文件
	while ((entry = readdir(dir)) != nullptr) {
		const std::string filename = entry->d_name;

		// 如果文件名匹配当前日期和序号模式
		std::smatch match;
		if (std::regex_match(filename, match, pattern)) {
			// 提取序号并转换为整数
			int sequence = std::stoi(match[1].str());
			maxSequence = std::max(maxSequence, sequence);
		}
	}

	closedir(dir);  // 关闭目录
	return maxSequence;
}

void Logger::resetFileIndex() {
	static std::string lastDateHour = getCurrentDateHour();
	if (lastDateHour != getCurrentDateHour()) {
		std::lock_guard<std::mutex> lock(logMutex_);
		currentFileIndex_ = 0;
		if (!logFile_.is_open()) {
			logFile_.open(getLogFileName(), std::ios::out | std::ios::app);
		} else {
			logFile_.close();
			fileSize_ = 0;
			logFile_.open(getLogFileName(), std::ios::out | std::ios::app);
		}
		lastDateHour = getCurrentDateHour();
	}
}

void Logger::logThreadFunction() {
	static auto lastWriteTime = std::chrono::system_clock::now();
	while (!exit_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::deque<std::string> logsToWrite;
		{
			auto currentTime = std::chrono::system_clock::now();
			std::lock_guard<std::mutex> lock(logQueueMutex_);
			if(logQueue_.size() >= 100000 || currentTime > lastWriteTime + logCycle_) {
				//std::cout << getCurrentTime() << ": " << logQueue_.size() << std::endl;
				logQueue_.swap(logsToWrite);
				lastWriteTime = currentTime;
			}
		}

		while (!logsToWrite.empty()) {
			writeToFile(logsToWrite.front());
			logsToWrite.pop_front();
		}
	}

	flush();
}

void Logger::flush() {
	std::deque<std::string> logsToWrite;
	{
		std::lock_guard<std::mutex> lock(logMutex_);
		logQueue_.swap(logsToWrite);
	}

	while (!logsToWrite.empty()) {
		writeToFile(logsToWrite.front());
		logsToWrite.pop_front();
	}
}

void Logger::checkThreadFunction() {
	cleanOldLogs();
	auto lastCleanTime = getCurrentTimeMillis();
	while (!exit_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		resetFileIndex();
		ExecuteTaskPeriodically(lastCleanTime, 24 * 60 * 60 * 1000, std::bind(&Logger::cleanOldLogs, this));
	}
}

std::string Logger::logLevelToString(LogLevel level) const {
	switch (level) {
	case LogLevel::DEBUG:
		return "DEBUG";
	case LogLevel::INFO:
		return "INFO";
	case LogLevel::WARNING:
		return "WARNING";
	case LogLevel::ERROR:
		return "ERROR";
	}
	return "UNKNOWN";
}

uint64_t Logger::getCurrentTimeMillis() {
	// 获取当前时间点
	auto now = std::chrono::system_clock::now();
	// 转换为time_point_cast，以毫秒为单位
	auto duration = now.time_since_epoch();
	// 将duration转换为毫秒
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return millis;
}
