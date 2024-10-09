### Overview
The `JsonSerializer` framework provides a convenient way to serialize and deserialize C++ objects into JSON format using Qt's `QJsonDocument`, `QJsonObject`, `QJsonArray`, and `QJsonValue`. The system is flexible, supporting serialization for primitive types, Qt containers, standard containers, and custom objects that inherit from the `JsonSerializable` base class.

### Features
- **Serialization**: Convert various data types (primitives, Qt containers, standard containers, custom classes) into `QJsonValue`.
- **Deserialization**: Rebuild objects from JSON format, making it easy to read and store complex data.
- **Qt Meta-Object System Integration**: Custom classes can easily be made serializable by inheriting from `JsonSerializable` and using macros like `JSON_PROPERTY`.
- **Support for Custom Types**: Extensible to support new data types by implementing or specializing the `Serializer` template.

### Requirements
- Qt 5.0 or later.
- C++11 or higher for template meta-programming.

### Key Components

1. **Serializer**: A template-based system that handles the conversion of various data types to and from JSON. It supports:
    - **Primitive types**: `int`, `double`, `bool`, `QString`, etc.
    - **Qt containers**: `QList`, `QVector`, `QMap`, `QHash`.
    - **Standard containers**: `std::vector`, `std::map`.
    - **Custom types**: Custom classes inheriting from `JsonSerializable`.
  
2. **JsonSerializable**: A base class that facilitates the integration with Qt's meta-object system. It provides:
    - `toJson()`: Converts an object to a `QJsonObject`.
    - `fromJson()`: Rebuilds an object from a `QJsonObject`.
    - `toRawJson()`: Returns a `QByteArray` representation of the object in JSON format.

3. **Macros**:
    - `JSON_SERIALIZABLE`: Marks a class as serializable.
    - `JSON_PROPERTY`: Declares a JSON property for a class, providing getter and setter methods that serialize/deserialize the property.

### Example Classes

1. **TestPerson**: A simple class representing a person with a name, age, and hobbies.
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

2. **TestPageInfo**: A class representing pagination information.
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

3. **TestPagedPerson**: A paginated list of persons.
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

### Usage Example

#### Serialization
In the `main.cpp` file, we demonstrate how to serialize and write a `TestPagedPerson` object to a file:
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
    qCritical() << "Can not open file to write!";
}
QTextStream wts(&wf);
wts << rawJson;
wf.close();
```
This will create a JSON file (`example.json`) containing the serialized data.

#### Deserialization
To read the JSON file and convert it back to a `TestPagedPerson` object:
```cpp
QFile rf("example.json");
if (!rf.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qCritical() << "Can not open file to read!";
}
QTextStream rts(&rf);
auto jsonStr = rts.readAll();
rf.close();

TestPagedPerson newPagedPerson;
newPagedPerson.fromJson(jsonStr.toUtf8());
```

### How to Extend

To add serialization support for new types:
1. Specialize the `Serializer` template for the type.
2. Implement `toJson()` and `fromJson()` methods for converting between the type and `QJsonValue`.

For example, adding support for a new container type:
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

### License
This code is provided as-is. For any questions or issues, feel free to contact the author at linxmouse@gmail.com.