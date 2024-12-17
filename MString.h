#ifndef MSTRING_H
#define MSTRING_H

#include <cstring>
#include <vector>
#include <iterator>
#include <sstream>
#include <memory>

enum StringEncoding {
	StringEncoding_Empty = -2,
	StringEncoding_Unknown = -1,
	StringEncoding_ANSI = 0,
	StringEncoding_UTF8,
	StringEncoding_UTF8_BOM,
	StringEncoding_UTF16_LE,
	StringEncoding_UTF16_BE,
};

class MString {
private:
	std::unique_ptr<char[]> data;
	size_t len;
public:
	// 构造函数
	MString(const char* s = "") noexcept;

	// 构造函数
	template<class T>
	MString(const T& value) noexcept {
		*this = value;
	}

	// 拷贝构造函数
	MString(const MString& other) noexcept;

	// 移动构造函数
	MString(MString&& other) noexcept;

	// 拷贝赋值运算符
	MString& operator=(const MString& other) noexcept;

	// 移动赋值操作符
	MString& operator=(MString&& other) noexcept;

	// 析构函数
	~MString();

	// 添加 c_str() 方法
	const char* getData() const;

	// std::string -> MString
	static MString fromStdString(const std::string& str);

	// MString -> std::string
	std::string toStdString() const;

	// 字符串连接
	MString operator+(const MString& other) const;

	// 重载+=
	MString& operator+=(const char* str);

	// 获取长度
	size_t length() const;

	// 字符串比较
	int compareTo(const MString& other) const;

	// 字符串查找
	int indexOf(const char* substr) const;

	// 字符串反向查找
	int lastIndexOf(const char* substr) const;

	// 字符串替换
	MString replace(const char* find, const char* replace) const;

	// 字符串分割
	std::vector<MString> split(const char* delimiter) const;

	// 字符串拼接
	static MString join(const std::vector<MString>& strings, const MString& separator);

	// 字符串修剪
	MString trim() const;

	// 查找子字符串并返回开始位置
	int find(const char* substr, size_t startPos = 0) const;

	// 反向查找子字符串并返回开始位置 (从尾部查找)
	int rfind(const char* substr, size_t startPos = std::string::npos) const;

	// 判断是否存在字串
	bool contains(const char* substr) const;

	// 返回从指定位置开始的指定数量的字符
	MString mid(size_t pos, size_t n) const;

	// 返回字符串左侧的指定数量的字符
	MString left(size_t n) const;

	// 返回字符串右侧的指定数量的字符
	MString right(size_t n) const;

	// 转换为大写
	MString toUpperCase() const;

	// 转换为小写
	MString toLowerCase() const;

	// 判断是否为空
	bool isEmpty() const;

	// 输出运算符重载
	friend std::ostream& operator<<(std::ostream & os, const MString & obj);

	// 格式化字符串并返回 MString
	template <typename... Args>
	static MString format(const std::string& format, Args&&... args) {
		// 估算需要的空间，提前分配
		size_t estimated_size = format.size() + (sizeof...(args) * 32);  // 大致预估每个参数占32字符空间
		std::vector<char> buffer;
		buffer.reserve(estimated_size);

		// 开始格式化过程
		formatImpl(buffer, format, std::forward<Args>(args)...);

		// 使用构造函数创建 MString 对象
		return MString(std::string(buffer.begin(), buffer.end()));
	}
private:
	// 基本类型支持（对所有类型支持输出到流）
	template <typename T>
	static void appendToBuffer(std::vector<char>& buffer, const T& value) {
		std::ostringstream oss;
		oss << value;
		std::string str = oss.str();
		buffer.insert(buffer.end(), str.begin(), str.end());
	}

	// 递归函数，将格式字符串与参数进行匹配并输出到缓冲区
	static void formatImpl(std::vector<char>& buffer, const std::string& format) {
		buffer.insert(buffer.end(), format.begin(), format.end());
	}

	// 主递归实现，处理每个参数并替换{}
	template <typename T, typename... Args>
	static void formatImpl(std::vector<char>& buffer, const std::string& format, T&& value, Args&&... args) {
		size_t pos = format.find("{}");  // 查找第一个 {}
		if (pos != std::string::npos) {
			// 插入 '{}' 前的部分
			buffer.insert(buffer.end(), format.begin(), format.begin() + pos);

			// 插入当前参数
			appendToBuffer(buffer, std::forward<T>(value));

			// 递归处理剩余部分
			formatImpl(buffer, format.substr(pos + 2), std::forward<Args>(args)...);
		} else {
			// 没有更多的 {}，直接插入剩余的格式字符串
			formatImpl(buffer, format);
		}
	}
public:// 编码相关
	// 是否为UTF-8编码：true->UTF-8;false->ANSI(GBK)
	static bool isUtf8Encoding(const char* str);

	// 获取编码格式
	static StringEncoding getStringEncoding(const char* str);

	// 获取编码，以16进制字符串表示
	MString getHexString() const;

	// 获取当前时间
	static MString getCurrentTime();
public:// 基础类型与MString之间转换
	// 重载 == 运算符
	bool operator==(const MString& other) const;

	// 重载 != 运算符
	bool operator!=(const MString& other) const;

	// 判断是否为数字
	static bool isNumber(const MString& s);

	// 判断是否为数字
	static bool isNumber(const char* s);

	// 字符串转数值
	template<typename T>
	static T convert(const char* value) {
		try {
			return std::stod(std::string(value));
		} catch (const std::invalid_argument& e) {
			return static_cast<T>(0.0);
		} catch (const std::out_of_range& e) {
			return static_cast<T>(0.0);
		}
	}

	// 数值 -> MString
	template<typename T>
	static MString convert(const T& value) {
		std::stringstream ss;
		ss << value;
		return fromStdString((std::string)ss.str());
	}

	// 类型转换运算符/转换构造函数: int x = str; 隐式调用int x = operator int();
	template<typename T>
	operator T()const {
		return convert<T>(data.get());
	}

	// 重载赋值运算符
	template<typename T>
	MString& operator=(const T& num) {
		*this = convert<T>(num);
		return *this;
	}

	// 重载赋值运算符
	MString& operator=(const char* str);
public:// 迭代器
	class Iterator {
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = char;
		using difference_type = std::ptrdiff_t;
		using pointer = char*;
		using reference = char&;

		// 构造函数
		Iterator(pointer ptr) : ptr(ptr) {}

		// 解引用操作符
		reference operator*() const {
			return *ptr;
		}

		pointer operator->() {
			return ptr;
		}

		// 前缀自增
		Iterator& operator++() {
			ptr++;
			return *this;
		}

		// 后缀自增
		Iterator operator++(int) {
			Iterator tmp = *this;
			++(*this);
			return tmp;
		}

		// 相等比较
		friend bool operator==(const Iterator& a, const Iterator& b) {
			return a.ptr == b.ptr;
		}

		// 不相等比较
		friend bool operator!=(const Iterator& a, const Iterator& b) {
			return a.ptr != b.ptr;
		}

	private:
		pointer ptr;
	};

	Iterator begin() {
		return Iterator(data.get());
	}

	Iterator end() {
		return Iterator(data.get() + len);
	}
};

#endif