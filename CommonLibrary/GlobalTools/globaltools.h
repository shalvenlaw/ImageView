#pragma once

#include <QDebug>
#include <QDir>
#include <QDate>
#include <QTime>
#include <QSettings>
#include <QLineEdit>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
/* 软件版本信息 */
// 主版本号
const QString MAJOR_VERSION = "1.1";
// 次要版本号
const QDate BUILD_DATE = QLocale(QLocale::English).toDate(QString(__DATE__).replace("  ", " 0"),
                                                          "MMM dd yyyy");
const QTime BUILD_TIME = QTime::fromString(__TIME__, "hh:mm:ss");
const QString MINOR_VERSION = QString("%1.%2").arg(BUILD_DATE.toString("yyyyMMdd"),
                                                   BUILD_TIME.toString("hhmmss"));
// 版本号
const QString VERSION = QString("%1.%2").arg(MAJOR_VERSION, MINOR_VERSION);

// 清理文件夹dir中的修改日期至今超过expire_days的文件,
void CleanExpiredFiles(const QString &dir_path, const int expire_days);
// 日志工具
void MyMessageOutput(QtMsgType msg_type, const QMessageLogContext &context,
                     const QString &msg);

/* 文件夹相关 */
// 创建直到目标文件的多级目录, 输入是目标文件的路径
void MakeMultiLevelDir(const QString &file_path);
// 静默版本
bool MakeMultiLevelDirSilently(const QString &file_path);

/* 文件相关 */
// 从file_path读取文件并把内容放在content
bool ReadFile(QString &content, const QString &file_path);
// 直接返回内容, 如果失败会返回空字符串, 此函数比较少用
QByteArray ReadFile(const QString &file_path);
// 将content保存在file_path
bool WriteFile(const QString &file_path, const QString &content);
// 向文件追加内容
bool AppendFile(const QString &file_path, const QString &content);

/* 字符串转码相关 */
QString gbk_to_utf8(const std::string &vs_string);

std::string utf8_to_gbk(const QString &qt_string);

/*
 * 文件内容相关
*/
/* 正则表达式 */
// 数据交换格式
enum DataExchangFormat {
    JSON = 0,   // JSON
    XML         // XML
};

// 利用正则表达式从文本(xml或json)中获取内容(获取第一个)
const QString CaptureContent(const QString &text,
                             const DataExchangFormat &text_format, const QString &key);
// 利用正则表达式从文本(xml或json)中获取内容(获取第一个), 直接根据构造搜索模式
const QString CaptureContent(const QString &text, const QString &pattern);
// 利用正则表达式从文本(xml或json)中获取内容(获取所有)
const QStringList CaptureContents(const QString &text,
                                  const DataExchangFormat &text_format, const QString &key);
// 利用正则表达式从文本(xml或json)中获取内容(获取所有), 直接根据构造搜索模式
const QStringList CaptureContents(const QString &text, const QString &pattern);

/* json相关 */
// 从QByteArray中解析json, 返回QHash<QString, QString>
bool ParseJson(QHash<QString, QString> &data, const QString &content);
// 从QByteArray中解析json, 返回QVariantHash
QVariantHash ParseJson(const QString &content);
//
bool ParseJson(QJsonArray &jsonArray, const QString &content);
// 通过QHash生成json
bool GenerateJson(QString &content, const QVariantHash &data);
// 通过QJsonObject生成json
QByteArray GenerateJson(const QJsonObject &jsonObject);
// 通过QJsonArray生成json
QByteArray GenerateJson(const QJsonArray &jsonArray);
// 通过QVariantHash生成json
QByteArray GenerateJson(const QVariantHash &data);
//bool GenerateJson(QString &content, const QJsonObject &jsonObject);

#include <iostream>
#define NameOf(x) #x
// 打印容器
template <typename T>
void printC(const T &container)
{
    std::cout << NameOf(container)": ";
    for (const auto &item : container) {
        std::cout << item << "\t";
    }
    std::cout << std::endl;
}
// 将变量名与其值连接成为一个字符串, 通常用于输出日志
#define NameValueOf(x) QString(NameOf(x)"{%1}").arg(x)

// 如果x为真, 则直接函数返回(意味着满足条件就中断处理过程)
#define ReturnIf(x) \
if (x) { \
    qWarning() << #x; \
    return; \
}

#include <QMetaObject>
#include <QMetaEnum>
// 将用QENUM定义过的枚举, 转换成字符串
template <class QEnum>
static inline QString qEnumToString(QEnum value)
{
    const QMetaObject *metaObject = qt_getEnumMetaObject(value);
    const QMetaEnum me = metaObject->enumerator(metaObject->indexOfEnumerator(qt_getEnumName(value)));
    if (const char *key = me.valueToKey(int(value))) {
//        metaObject->className(); // 定义该枚举的类的名字
//        me.enumName(); // 枚举名
        return QString("%1::%2")
               .arg(metaObject->className()).arg(key);
    } else {
        return QString(value);
    }
}

