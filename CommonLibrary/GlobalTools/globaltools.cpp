#include "globaltools.h"
#include <filesystem>

#include <QRegularExpressionMatch>
#include <QJsonDocument>
#include <QDate>
#include <QMutex>

void CleanExpiredFiles(const QString &dir_path, const int expire_days)
{
    // 当前日期
    const QDate current_date = QDate::currentDate();
    QDir dir(dir_path);
    dir.setFilter(QDir::Files); // 只找文件
    dir.setSorting(QDir::Time | QDir::Reversed); // 按照修改时间从小到大排序
    const QFileInfoList file_infos = dir.entryInfoList();
    for (const auto &file_info : file_infos) {
        if (file_info.lastModified().date().daysTo(current_date) < expire_days) {
            // 之后的都是未过期的
            return;
        }
        // 如果距离今天超过expire_days, 就删掉
        dir.remove(file_info.fileName());
    }
}

// 日志工具
void MyMessageOutput(QtMsgType msg_type, const QMessageLogContext &context, const QString &msg)
{
    // 日志信息类型
    QString text;
    switch (msg_type) {
    case QtInfoMsg:
        text = "[Info]";
        break;
    case QtDebugMsg:
        text = "[Debug]";
        break;
    case QtWarningMsg:
        text = "[Warning]";
        break;
    case QtCriticalMsg:
        text = "[Critical]";
        break;
    case QtFatalMsg:
        text = "[Fatal]";
        break;
    }
    // 时间
    text.append(QTime::currentTime().toString("[hh:mm:ss]"));
    // 给日志添加上下文信息, 比如哪个文件, 哪一行, 哪一个函数
    text.append(QString("[%1 %2] %3\n")
                .arg(QFileInfo(context.file).fileName())
                .arg(context.line)
                .arg(msg));
//                .arg(CaptureContent(context.function, " ([\\S]+?)\\(")) // 提取函数名 -> 有了文件名和行数, 函数名就不需要了
    // 日志文件所在目录
    static const QString LOG_DIR = "./logs/";
    const QDir dir(LOG_DIR);
    if (!dir.exists()) {
        // 如果不存在该路径, 创建到该路径的所有文件夹
        dir.mkpath(".");
    }
    // 根据日期构造日志文件路径
    const QString path_forLogFile = QString("%1%2")
                                    .arg(LOG_DIR,
                                         QDate::currentDate().toString("yyyy-MM-dd'.txt'"));

    // 互斥量, 以防有多线程调用
    static QMutex mutex_forlogFile;
    {
        QMutexLocker locker(&mutex_forlogFile); // locker构造, 自动加锁
        // 写入文件
        if (QFile logFile(path_forLogFile);
                logFile.open(QIODevice::WriteOnly //只写
                             | QIODevice::Append // 添加而不是覆盖
                             | QIODevice::Text)) { // 如果是Linux系统, 将\r\n转换为\n; 如果是Window系统, 将\n转换为\r\n. 其实这处理也不是很完美, 但总好过没有;)
            // 如果打开成功, 就写入将内容文件
            QTextStream textStream(&logFile);
            textStream << text;
        } // textStream析构, 自动flush到logFile; 然后logFile析构, 自动close, 从而flush到log文件
    } // locker析构, 自动解锁

    // 删掉LOG_DIR中修改日期至今超过expire_days的文件
    CleanExpiredFiles(LOG_DIR, 60);
}

/* 文件夹相关 */
// 创建直到目标文件的多级目录, 输入是目标文件的路径
void MakeMultiLevelDir(const QString &file_path)
{
    //返回文件路径，不包含文件名
    const QString dir_path = QFileInfo(file_path).path();

    QDir dir(dir_path);
    if (!dir.exists()) {
        // 如果文件夹路径不存在, 就创建到达该路径的所有文件夹
        qWarning().noquote() << "Directory not existed:" << dir_path;
        if (!dir.mkpath(".")) {
            qWarning().noquote() << "Couldn't make path:" << dir_path;
        }
    }
}
// 静默版本
bool MakeMultiLevelDirSilently(const QString &file_path)
{
    // QFileInfo(file_path).path()返回文件路径，不包含文件名
    // If the path already exists when this function is called, it will return true.
    // 如果不存在该目录, 就创建该目录, 然后返回成功与否
    QDir dir;
    return dir.mkpath(QFileInfo(file_path).path());
}



/* 文件相关 */
// 从file_path读取文件并把内容放在content
bool ReadFile(QString &content, const QString &file_path)
{
    // 读取文件
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly
                   | QIODevice::Text)) {
        qWarning().noquote() << "Open file failed: " << file_path;
        return false;
    }
    content = file.readAll();
    // 由于file是局部对象, 所以返回之后就自动析构, 析构时会调用close(), close()会调用flush(), 所以这里不用自己调用
    return true;
}
// 直接返回内容, 不返回是否成功, 此函数比较少用
QByteArray ReadFile(const QString &file_path)
{
    // 读取文件
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly
                   | QIODevice::Text)) {
        qWarning().noquote() << "Open file failed: " << file_path;
        return "";
    }
    return file.readAll();
    // 由于file是局部对象, 所以返回之后就自动析构, 析构时会调用close(), close()会调用flush(), 所以这里不用自己调用
}
// 将content保存在file_path
bool WriteFile(const QString &filePath, const QString &content)
{
    // 创建直到目标文件的多级目录
    MakeMultiLevelDir(filePath);
    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly
                   | QIODevice::Text)) {
        qWarning().noquote() << "Open file failed:" << filePath;
        return false;
    }
    if (-1 == file.write(content.toUtf8())) {
        qWarning().noquote() << "Write file failed:" << filePath;
        return false;
    }
    // 由于file是局部对象, 所以返回之后就自动析构, 析构时会调用close(), close()会调用flush(), 所以这里不用自己调用
    return true;
}

bool AppendFile(const QString &file_path, const QString &content)
{
    // 创建直到目标文件的多级目录
    MakeMultiLevelDir(file_path);
    // 写入文件
    QFile file(file_path);
    if (!file.open(QIODevice::Append
                   | QIODevice::Text)) {
        qWarning().noquote() << "Open file failed:" << file_path;
        return false;
    }
    if (-1 == file.write(content.toUtf8())) {
        qWarning().noquote() << "Write file failed:" << file_path;
        return false;
    }
    // 由于file是局部对象, 所以返回之后就自动析构, 析构时会调用close(), close()会调用flush(), 所以这里不用自己调用
    return true;
}

/* 字符串转码相关 */
QString gbk_to_utf8(const std::string &vs_string)
{
    return QString::fromLocal8Bit(vs_string.c_str());
}

std::string utf8_to_gbk(const QString &qt_string)
{
    return qt_string.toLocal8Bit().toStdString();
}


/*
 * 文件内容相关
*/
/* 正则表达式 */
QString MakePattern(const DataExchangFormat &text_format, const QString &key)
{
    // 在目标匹配组中, 由于只想要非空的内容, 所以用量词+而不是*; 由于只想要忽略优先(非贪婪匹配), 所以后面是?
    // TODO: 构造的重点应该是目标内容可能有哪些字符??
    switch (text_format) {
    case DataExchangFormat::JSON: {
        // json中的引号有双引号和单引号, :两边可能有空白字符 TODO: 引号中间应该不允许回车?
        return QString(R"delimeter(['\"]%1['\"]\s*:\s*['\"]([^'\"\n]+?)['\"])delimeter").arg(key);
    }
    case DataExchangFormat::XML: {
        // xml中的目标内容两边可能有空白字符
        return QString(R"delimeter(<%1[^>]*>\s*([\s\S]+?)\s*</%1>)delimeter").arg(key);
    }
    }
    return "";
}

// 利用正则表达式从文本(xml或json)中获取内容(获取第一个), 直接根据构造搜索模式
const QString CaptureContent(const QString &text, const QString &pattern)
{
    // 构造正则表达式
    QRegularExpression regular_expression(pattern);
    // 搜索目标内容
    QRegularExpressionMatch match = regular_expression.match(text);
    // 字符串, 用于存储返回值
    QString content;
    if (match.hasMatch()) {
        // 如果匹配到了
        content = match.captured(1);
    } else {
        // 如果没有匹配到, 输出日志提示
        qInfo().noquote() << "Nothing matched:"
//                          << "\n\ttext=" << text
                          << "\n\tpattern=" << pattern;
    }
    // 如果匹配到了, 会返回对应的内容, 如果没有匹配到, 返回的是一个null的QString
    return content;
}

// 利用正则表达式从文本(xml或json)中获取内容(获取第一个), 根据数据交换类型和关键字来构造搜索模式
const QString CaptureContent(const QString &text, const DataExchangFormat &text_format,
                             const QString &key)
{
    // 构造搜索模式
    QString pattern = MakePattern(text_format, key);
    return CaptureContent(text, pattern);
}

// 利用正则表达式从文本(xml或json)中获取内容(获取所有), 直接根据构造搜索模式
const QStringList CaptureContents(const QString &text, const QString &pattern)
{
    // 构造正则表达式
    QRegularExpression regular_expression(pattern);
    // 搜索目标内容
    QRegularExpressionMatchIterator it = regular_expression.globalMatch(text);
    // 字符串列表, 用于存储返回值
    QStringList contents;
    if (it.hasNext()) {
        // 如果匹配到了
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            const QString content = match.captured(1);
            contents << content;
        }
    } else {
        // 如果没有匹配到, 输出日志提示
        qInfo().noquote() << "Nothing matched:"
//                          << "\n\ttext=" << text
                          << "\n\tpattern=" << pattern;
    }
    // 如果没有匹配到, 返回的是一个空的字符串列表
    return contents;
}

// 利用正则表达式从文本(xml或json)中获取内容(获取所有)
const QStringList CaptureContents(const QString &text, const DataExchangFormat &text_format,
                                  const QString &key)
{
    // 构造搜索模式
    QString pattern = MakePattern(text_format, key);
    return CaptureContents(text, pattern);
}

/* json相关 */
// 从QByteArray中解析json
bool ParseJson(QHash<QString, QString> &data, const QString &content)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &error);
    if (jsonDocument.isNull() || QJsonParseError::NoError != error.error) {
        qWarning() << QStringLiteral("解析json{%1}失败{%2}!")
                   .arg(content, error.errorString());
        return false;
    }

    QVariantHash result = jsonDocument.toVariant().toHash();
    foreach (const QString &key, result.uniqueKeys()) {
        data.insert(key, result.value(key).toString());
    }
    return true;
}

// 从QByteArray中解析json
QVariantHash ParseJson(const QString &content)
{

    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &error);
    if (jsonDocument.isNull() || QJsonParseError::NoError != error.error) {
        qWarning() << QStringLiteral("解析json{%1}失败{%2}!")
                   .arg(content, error.errorString());
    }
    return jsonDocument.toVariant().toHash();
}

bool ParseJson(QJsonArray &jsonArray, const QString &content)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &error);
    if (jsonDocument.isNull() || QJsonParseError::NoError != error.error) {
        qWarning() << QStringLiteral("解析json{%1}失败{%2}!")
                   .arg(content, error.errorString());
        return false;
    }
    if (!jsonDocument.isArray()) {
        qWarning() << QStringLiteral("json{%1}不是数组{%2}!")
                   .arg(content, error.errorString());
        return false;
    }
    jsonArray = jsonDocument.toVariant().toJsonArray();
    return true;
}

// 通过QVariantHash生成json
bool GenerateJson(QString &content, const QVariantHash &data)
{
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(data);
    if (jsonDocument.isNull()) {
        qWarning() << "json document is Null";
        return false;
    }
    content = jsonDocument.toJson(QJsonDocument::Compact);
    return true;
}

// 通过QJsonObject生成json
//bool GenerateJson(QString &content, const QJsonObject &jsonObject)
//{
//    QJsonDocument jsonDocument(jsonObject);
//    if (jsonDocument.isNull()) {
//        qWarning() << "json document is Null";
//        return false;
//    }
//    content = jsonDocument.toJson(QJsonDocument::Compact); // 紧凑输出
//    return true;
//}

// 通过QJsonObject生成json
QByteArray GenerateJson(const QJsonObject &jsonObject)
{
    QJsonDocument jsonDocument(jsonObject);
    if (jsonDocument.isNull()) {
        qWarning() << "json document is Null";
        return "";
    }
    return jsonDocument.toJson(QJsonDocument::Compact); // 紧凑输出
}

QByteArray GenerateJson(const QJsonArray &jsonArray)
{
    QJsonDocument jsonDocument(jsonArray);
    if (jsonDocument.isNull()) {
        qWarning() << "json document is Null";
        return "";
    }
    return jsonDocument.toJson(QJsonDocument::Compact); // 紧凑输出
}

QByteArray GenerateJson(const QVariantHash &data)
{
    QJsonDocument jsonDocument = QJsonDocument::fromVariant(data);
    if (jsonDocument.isNull()) {
        qWarning() << "json document is Null";
        return "";
    }
    return jsonDocument.toJson(QJsonDocument::Compact); // 紧凑输出
}


