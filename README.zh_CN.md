## 概述

`JsonSerializer` 框架提供了一种方便的方式，将 C++ 对象序列化和反序列化为 JSON 格式。框架基于 Qt 的 `QJsonDocument`、`QJsonObject`、`QJsonArray` 和 `QJsonValue`，支持原始数据类型、Qt 容器、标准容器和自定义类的序列化和反序列化。

## 功能特性

- **序列化**：将多种数据类型（原始类型、Qt 容器、标准容器、自定义类等）转换为 `QJsonValue`。
- **反序列化**：从 JSON 数据中重建对象，方便数据的存储和读取。
- **Qt 元对象系统集成**：自定义类继承自 `JsonSerializable` 并使用 `JSON_PROPERTY` 宏即可快速实现序列化。
- **支持自定义类型**：可通过实现或特化 `Serializer` 模板来扩展对新数据类型的支持。

## 依赖要求

- **Qt 5.0** 或更高版本
- **C++11** 或更高版本（使用模板元编程）

## 核心组件

### 1. **Serializer**

一个基于模板的系统，处理不同数据类型与 JSON 格式的相互转换。支持以下数据类型：

- **原始类型**：如 `int`、`double`、`bool`、`QString` 等。
- **Qt 容器**：如 `QList`、`QVector`、`QMap`、`QHash`。
- **标准容器**：如 `std::vector`、`std::map`。
- **自定义类型**：继承自 `JsonSerializable` 的自定义类。

### 2. **JsonSerializable**

提供了 `toJson` 和 `fromJson` 等方法，方便类与 JSON 数据相互转换：

- `toJson()`：将对象转换为 `QJsonObject`。
- `fromJson()`：从 `QJsonObject` 中重建对象。
- `toRawJson()`：返回对象的 JSON 字符串表示。

### 3. **宏定义**

- **`JSON_SERIALIZABLE`**：标记一个类为可序列化。
- **`JSON_PROPERTY`**：定义 JSON 属性，提供对应的 getter 和 setter，自动处理属性的序列化与反序列化。

## 示例类

### 1. **TestPerson** 类：表示一个人的简单信息

```cpp
class TestPerson final: public JsonSerializable
{
    Q_GADGET
    JSON_SERIALIZABLE
public:
    JSON_PROPERTY(QString, name)
    JSON_PROPERTY(int, age)
    JSON_PROPERTY(QList<QString>, hobbies)
};
```

### 2. **TestPageInfo** 类：表示分页信息

```cpp
class TestPageInfo final: public JsonSerializable
{
    Q_GADGET
    JSON_SERIALIZABLE
public:
    JSON_PROPERTY(int, totalNumber)
    JSON_PROPERTY(int, totalPage)
    JSON_PROPERTY(int, pageSize)
    JSON_PROPERTY(int, currentPage)
};
```

### 3. **TestPagedPerson** 类：包含分页信息和人员列表

```cpp
class TestPagedPerson final: public JsonSerializable
{
    Q_GADGET
    JSON_SERIALIZABLE
public:
    JSON_PROPERTY(TestPageInfo, page)
    JSON_PROPERTY(std::vector<TestPerson>, persons)
};
```

## 使用示例

### 序列化

以下示例演示如何将 `TestPagedPerson` 对象序列化并写入到文件：

```cpp
TestPagedPerson pagedPerson;
TestPageInfo page;
page.set_totalNumber(80);
page.set_totalPage(4);
page.set_currentPage(1);
page.set_pageSize(20);
pagedPerson.set_page(page);

TestPerson person1;
person1.set_age(18);
person1.set_name("A");
person1.set_hobbies({ "running", "TV" });

pagedPerson.set_persons({ person1 });
auto rawJson = pagedPerson.toRawJson();
qDebug().noquote() << rawJson;

QFile wf("example.json");
if (!wf.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qCritical() << "无法打开文件进行写入！";
}
QTextStream wts(&wf);
wts << rawJson;
wf.close();
```

生成的 `example.json` 文件将包含序列化后的 JSON 数据。

### 反序列化

以下示例展示如何从 JSON 文件中读取数据并反序列化回对象：

```cpp
QFile rf("example.json");
if (!rf.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qCritical() << "无法打开文件进行读取！";
}
QTextStream rts(&rf);
auto jsonStr = rts.readAll();
rf.close();

TestPagedPerson newPagedPerson;
newPagedPerson.fromJson(jsonStr.toUtf8());
```

## 扩展方法

### 支持新类型的序列化

要为新的类型添加序列化支持，可以特化 `Serializer` 模板。例如，支持自定义容器：

```cpp
template<typename T>
struct Serializer<MyContainer<T>> {
    static QJsonValue toJson(const MyContainer<T>& container) {
        QJsonArray array;
        for (const auto& item : container) {
            array.append(Serializer<T>::toJson(item));
        }
        return array;
    }

    static MyContainer<T> fromJson(const QJsonValue& json) {
        MyContainer<T> result;
        if (json.isArray()) {
            QJsonArray array = json.toArray();
            for (const auto& item : array) {
                result.add(Serializer<T>::fromJson(item));
            }
        }
        return result;
    }
};
```

## 许可证

此代码以 **as-is** 提供。如有任何问题或建议，请联系作者：[linxmouse@gmail.com](mailto:linxmouse@gmail.com)。

