#include "MString.h"
#include <cstdarg>
#include <iomanip>
#include <chrono>
#ifdef _MSC_VER
#include <cctype>
#endif // _MSC_VER


// 构造函数
MString::MString(const char* s) noexcept {
    len_ = strlen(s);
    std::unique_ptr<char[]> temp = std::move(data_); // 将data的所有权转,超出范围自动释放
    data_.reset(new char[len_ + 1]);
    strcpy_s(data_.get(), len_, s);
}

// 拷贝构造函数
MString::MString(const MString& other) noexcept {
    len_ = other.len_;
    std::unique_ptr<char[]> temp = std::move(data_); // 将data的所有权转,超出范围自动释放
    data_.reset(new char[len_ + 1]);
    strcpy_s(data_.get(), len_, other.data_.get());
}

// 移动构造函数
MString::MString(MString&& other) noexcept
        : data_(std::move(other.data_)), len_(other.len_) {
    other.len_ = 0; // 重置源对象的长度
}

// 拷贝赋值运算符
MString& MString::operator=(const MString& other) noexcept {
    if (this == &other) {  // 处理自我赋值
        return *this;
    }

    // 释放原有资源
    std::unique_ptr<char[]> temp = std::move(data_); // 将data的所有权转,超出范围自动释放

    // 分配新的内存并拷贝数据
    len_ = other.len_;
    data_.reset(new char[len_ + 1]);
    strcpy_s(data_.get(), len_, other.data_.get());

    return *this;  // 返回当前对象的引用，以支持链式赋值
}

// 移动赋值操作符
MString& MString::operator=(MString&& other) noexcept {
    if (this != &other) {
        data_ = std::move(other.data_); // 使用std::move转移智能指针的所有权
        len_ = other.len_;
        other.len_ = 0; // 重置源对象的长度
    }
    return *this;
}

// 析构函数
MString::~MString() {
}

// 添加 c_str() 方法
const char* MString::getData() const {
    return data_.get();
}

// std::string -> MString
MString MString::fromStdString(const std::string& str) {
    return MString(str.c_str());
}

// MString -> std::string
std::string MString::toStdString() const {
    return std::string(this->getData());
}

// 字符串连接
MString MString::operator+(const MString& other) const {
    size_t newLength = len_ + other.len_;
    std::unique_ptr<char[]> newData(new char[newLength + 1]);
    strcpy_s(newData.get(), newLength, data_.get());
    strcat_s(newData.get(), newLength, other.data_.get());

    MString newString(newData.get());
    return newString;
}

// += 运算符重载，支持 const char* 类型
MString& MString::operator+=(const char* str) {
    size_t strLen = strlen(str);
    size_t newLen = len_ + strLen;

    // 为新字符串分配内存
    std::unique_ptr<char[]> newData(new char[newLen + 1]);

    // 拷贝原始数据和附加的字符串
    strcpy_s(newData.get(), newLen, data_.get());
    strcat_s(newData.get(), newLen, str);

    // 更新数据和长度
    data_ = std::move(newData);
    len_ = newLen;

    return *this;
}

// 获取长度
size_t MString::length() const {
    return len_;
}

// 字符串比较
int MString::compareTo(const MString& other) const {
    return strcmp(data_.get(), other.data_.get());
}

// 字符串查找
int MString::indexOf(const char* substr) const {
    return find(substr);
}

// 字符串反向查找
int MString::lastIndexOf(const char* substr) const {
    return rfind(substr);
}

// 字符串替换
MString MString::replace(const char* find, const char* replace) const {
    std::string result = data_.get();
    size_t pos = 0;
    while ((pos = result.find(find, pos)) != std::string::npos) {
        result.replace(pos, strlen(find), replace);
        pos += strlen(replace);
    }
    return MString(result.c_str());
}

// 字符串分割
std::vector<MString> MString::split(const char* delimiter) const {
    std::vector<MString> parts;
    char* context = nullptr;
    char* token = strtok_s(data_.get(), delimiter, &context);
    while (token != nullptr) {
        parts.push_back(MString(token));
        token = strtok_s(nullptr, delimiter, &context);
    }
    return parts;
}

// 字符串拼接
MString MString::join(const std::vector<MString>& strings, const MString& separator) {
    std::string result;
    if (!strings.empty()) {
        result.append(strings[0].data_.get());  // 添加第一个元素
        for (size_t i = 1; i < strings.size(); i++) {
            result.append(separator.data_.get());  // 添加分隔符
            result.append(strings[i].data_.get());  // 添加当前元素
        }
    }
    return MString(result.c_str());  // 将 std::string 转换回 MString 类型
}

// 字符串修剪
MString MString::trim() const {
    const char* start = data_.get();
    while (isspace(*start)) start++;

    const char* end = data_.get() + strlen(data_.get()) - 1;
    while (end > start && isspace(*end)) end--;

    size_t trimLen = end - start + 1;
    std::unique_ptr<char[]> trimStr(new char[trimLen + 1]);
    strncpy_s(trimStr.get(), trimLen, start, trimLen);
    trimStr[trimLen] = '\0';

    MString trimmed(trimStr.get());
    return trimmed;
}

// 查找子字符串并返回开始位置
int MString::find(const char* substr, size_t startPos) const {
    if (!substr) return false;

    size_t patternLen = strlen(substr);
    if (patternLen == 0 || startPos >= len_) {
        return -1;  // 子串为空或起始位置无效
    }

    // 预处理坏字符规则
    std::vector<int> badChar(256, -1); // 假设字符集是ASCII
    for (size_t i = 0; i < patternLen; ++i) {
        badChar[static_cast<unsigned char>(substr[i])] = i;
    }

    // 从 startPos 开始进行查找
    size_t i = startPos;
    while (i <= len_ - patternLen) {
        size_t j = patternLen - 1;

        // 从后向前匹配
        while (j >= 0 && data_[i + j] == substr[j]) {
            --j;
        }

        // 如果匹配成功
        if (j == -1) {
            return i;  // 返回匹配的起始位置
        }

        // 根据坏字符规则，计算跳跃距离
        size_t badCharShift = j - badChar[static_cast<unsigned char>(data_[i + j])];
        size_t shift = (badCharShift > 0) ? badCharShift : 1;

        // 更新i的值，跳过不必要的字符
        i += shift;
    }

    // 如果没有找到
    return -1;
}

// 反向查找子字符串并返回开始位置 (从尾部查找)
int MString::rfind(const char* substr, size_t startPos) const {
    if (!substr) return -1;

    size_t patternLen = strlen(substr);
    if (patternLen == 0 || startPos < 0) {
        return -1;
    }

    // 默认从字符串末尾开始查找
    if (startPos == std::string::npos || startPos >= len_) {
        startPos = len_ - 1;
    }

    // 从startPos开始反向查找
    for (size_t i = startPos; i >= patternLen - 1; --i) {
        bool match = true;
        for (size_t j = 0; j < patternLen; ++j) {
            if (data_[i - j] != substr[patternLen - j - 1]) {
                match = false;
                break;
            }
        }
        if (match) {
            return i - patternLen + 1;
        }
    }

    return -1;
}

// 判断是否包含特定子字符串的函数
bool MString::contains(const char* substr) const {
    return find(substr) != -1;
}

// 返回从指定位置开始的指定数量的字符
MString MString::mid(size_t pos, size_t n) const {
    if (pos > len_) {
        return MString("");  // 超出字符串长度，返回空字符串
    }
    size_t numChars = (pos + n > len_) ? len_ - pos : n;
    std::unique_ptr<char[]> sub(new char[numChars + 1]);
    std::memcpy(sub.get(), data_.get() + pos, numChars);
    sub[numChars] = '\0';
    MString result(sub.get());
    return result;
}

// 返回字符串左侧的指定数量的字符
MString MString::left(size_t n) const {
    return mid(0, n);
}

// 返回字符串右侧的指定数量的字符
MString MString::right(size_t n) const {
    size_t start = (n > len_) ? 0 : len_ - n;
    return mid(start, n);
}

// 转换为大写
MString MString::toUpperCase() const {
    std::unique_ptr<char[]> upper(new char[len_ + 1]);
    for (size_t i = 0; i < len_; i++) {
        upper[i] = std::toupper(data_[i]);
    }
    upper[len_] = '\0';
    return MString(upper.get());
}

// 转换为小写
MString MString::toLowerCase() const {
    std::unique_ptr<char[]> lower(new char[len_ + 1]);
    for (size_t i = 0; i < len_; i++) {
        lower[i] = std::tolower(data_[i]);
    }
    lower[len_] = '\0';
    return MString(lower.get());
}

// 判断是否为空
bool MString::isEmpty() const {
    return len_ == 0;
}

// 输出运算符重载
std::ostream& operator<<(std::ostream& os, const MString& obj) {
    os << obj.data_.get();
    return os;
}

// 重载赋值运算符
MString& MString::operator=(const char* str) {
    if (data_.get() != str) {
        len_ = strlen(str);
        std::unique_ptr<char[]> temp = std::move(data_); // 将data的所有权转,超出范围自动释放
        data_.reset(new char[len_ + 1]);
        strcpy_s(data_.get(), len_, str);
    }
    return *this;
}

// 是否为UTF-8编码：true->UTF-8;false->ANSI(GBK)
bool MString::isUtf8Encoding(const char* str) {
    int count = 0;
    unsigned char single;
    bool allAscii = true;
    int len_ = strlen(str);

    //0x00-0x7F为ASCII码范围
    for (int uIndex = 0; uIndex < len_; ++uIndex) {
        single = str[uIndex];

        if ((single & 0x80) != 0)
            allAscii = false;

        if (count == 0) {
            if (single >= 0x80) {
                if (single >= 0xFC && single <= 0xFD)
                    count = 6;
                else if (single >= 0xF8)
                    count = 5;
                else if (single >= 0xF0)
                    count = 4;
                else if (single >= 0xE0)
                    count = 3;
                else if (single >= 0xC0)
                    count = 2;
                else
                    return false;

                count--;
            }
        } else {
            if ((single & 0xC0) != 0x80)//在UTF-8中，以位模式10开始的所有字节是多字节序列的后续字节
                return false;

            count--;
        }
    }

    if (count > 0)
        return false;

    if (allAscii)
        return false;

    return true;
}

// 获取编码格式
StringEncoding MString::getStringEncoding(const char* str) {
    /*
    ANSI                     无格式定义						对于中文编码格式是GB2312;
    Unicode little endian    文本里前两个字节为FF FE			字节流是little endian
    Unicode big endian       文本里前两个字节为FE FF			字节流是big endian
    UTF-8带BOM               前两字节为EF BB，第三字节为BF		带BOM
    UTF-8不带BOM             无格式定义,需另加判断				不带BOM
    */

    int size = strlen(str);
    if (size == 0) {
        return StringEncoding_Empty;
    }
    if (size == 1) {
        return StringEncoding_ANSI;
    }
    StringEncoding se;
    int nHead = ((unsigned char)str[0] << 8) + (unsigned char)str[1];

    switch (nHead) {
        case 0xFFFE:
            se = StringEncoding_UTF16_LE;
            break;
        case 0xFEFF:
            se = StringEncoding_UTF16_BE;
            break;
        case 0xEFBB:
            se = StringEncoding_UTF8_BOM;
            break;
        default: {
            if (isUtf8Encoding(str))
                se = StringEncoding_UTF8;
            else
                se = StringEncoding_ANSI;
            break;
        }
    }
    return se;
}

// 获取编码，以16进制字符串表示
MString MString::getHexString() const {
    static const char* hexChars = "0123456789ABCDEF";
    size_t hexLen = len_ * 3 - 1; // two hex digits per byte plus spaces
    std::unique_ptr<char[]> hexData(new char[hexLen + 1]);

    for (size_t i = 0, j = 0; i < len_; ++i) {
        if (i != 0) {
            hexData[j++] = ' ';
        }
        unsigned char byte = static_cast<unsigned char>(data_[i]);
        hexData[j++] = hexChars[(byte >> 4) & 0x0F];
        hexData[j++] = hexChars[byte & 0x0F];
    }
    hexData[hexLen] = '\0'; // null terminate the string

    return MString(hexData.get());
}

// 获取当前时间
MString MString::getCurrentTime() {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto duration = now_ms.time_since_epoch();

    // 获取秒数部分
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) - std::chrono::duration_cast<std::chrono::seconds>(duration);

    // 获取当前时间点
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // 使用 stringstream 来格式化时间
    std::stringstream ss;
    std::tm tm;
    localtime_s(&tm , &currentTime);
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();  // 追加毫秒部分

    return ss.str();
}

// 重载 == 运算符
bool MString::operator==(const MString& other) const {
    // 首先比较长度
    if (this->len_ != other.len_) {
        return false;
    }
    // 然后逐字符比较
    return strcmp(this->data_.get(), other.data_.get()) == 0;
}

// 重载 != 运算符
bool MString::operator!=(const MString& other) const {
    return !( * this == other);
}

// 判断是否为数字
bool MString::isNumber(const MString& s) {
    return isNumber(s.getData());
}

// 判断是否为数字
bool MString::isNumber(const char* s) {
    std::string newValue(s);
    std::istringstream iss(newValue);
    double doubleValue;

    // 尝试从字符串中提取浮点数
    if (iss >> doubleValue) {
        // 检查提取后是否到达字符串末尾
        char c;
        if (iss >> c) {
            // 如果提取后仍有字符，则说明不是有效的浮点数格式
            return false;
        } else {
            // 提取成功且到达字符串末尾，说明是有效的浮点数格式
            return true;
        }
    } else {
        // 提取失败，说明不是有效的浮点数格式
        return false;
    }
}