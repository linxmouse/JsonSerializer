// File: JsonSerializer
// Author: linxmouse@gmail.com
// Creation: 2024/09/29
#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <type_traits>

/* META OBJECT SYSTEM */
#include <QVariant>
#include <QMetaProperty>
#include <QMetaObject>
#include <QMetaType>

/* CONTAINER TYPE */
#include <QVector>
#include <QList>
#include <QMap>
#include <QHash>
#include <vector>
#include <map>

// Forward declarations
template<typename T, typename Enable = void>
struct Serializer;

// Primary template for QJsonValue conversion
template<typename T>
struct ToJsonValue {
	static QJsonValue convert(const T& value) {
		return QJsonValue::fromVariant(QVariant::fromValue(value));
	}
};

// Specialization for QJsonValue
template<>
struct ToJsonValue<QJsonValue> {
	static QJsonValue convert(const QJsonValue& value) {
		return value;
	}
};

// Serializer for primitive types
template<typename T>
struct Serializer<T, typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, QString>::value>::type> {
	static QJsonValue toJson(const T& value) {
		return ToJsonValue<T>::convert(value);
	}

	static T fromJson(const QJsonValue& json) {
		return json.toVariant().value<T>();
	}
};

// Serializer for QList and QVector
template<template<typename> class Container, typename T>
struct Serializer<Container<T>, typename std::enable_if<std::is_same<Container<T>, QList<T>>::value || std::is_same<Container<T>, QVector<T>>::value>::type> {
	static QJsonValue toJson(const Container<T>& container) {
		QJsonArray array;
		for (const auto& item : container) {
			array.append(Serializer<T>::toJson(item));
		}
		return array;
	}

	static Container<T> fromJson(const QJsonValue& json) {
		Container<T> result;
		if (json.isArray()) {
			QJsonArray array = json.toArray();
			for (const auto& item : array) {
				result.append(Serializer<T>::fromJson(item));
			}
		}
		return result;
	}
};

// Serializer for std::vector
template<typename T>
struct Serializer<std::vector<T>> {
	static QJsonValue toJson(const std::vector<T>& container) {
		QJsonArray array;
		for (const auto& item : container) {
			array.append(Serializer<T>::toJson(item));
		}
		return array;
	}

	static std::vector<T> fromJson(const QJsonValue& json) {
		std::vector<T> result;
		if (json.isArray()) {
			QJsonArray array = json.toArray();
			for (const auto& item : array) {
				result.push_back(Serializer<T>::fromJson(item));
			}
		}
		return result;
	}
};

// Serializer for QMap and QHash
template<template<typename, typename> class Map, typename K, typename V>
struct Serializer<Map<K, V>, typename std::enable_if<std::is_same<Map<K, V>, QMap<K, V>>::value || std::is_same<Map<K, V>, QHash<K, V>>::value>::type> {
	static QJsonValue toJson(const Map<K, V>& map) {
		QJsonObject obj;
		for (auto it = map.begin(); it != map.end(); ++it) {
			obj.insert(ToJsonValue<K>::convert(it.key()).toString(), Serializer<V>::toJson(it.value()));
		}
		return obj;
	}

	static Map<K, V> fromJson(const QJsonValue& json) {
		Map<K, V> result;
		if (json.isObject()) {
			QJsonObject obj = json.toObject();
			for (auto it = obj.begin(); it != obj.end(); ++it) {
				result.insert(ToJsonValue<K>::convert(it.key()).toVariant().value<K>(),
					Serializer<V>::fromJson(it.value()));
			}
		}
		return result;
	}
};

// Serializer for std::map
template<typename K, typename V>
struct Serializer<std::map<K, V>> {
	static QJsonValue toJson(const std::map<K, V>& map) {
		QJsonObject jsonObject;
		for (const auto& pair : map) {
			QString key = ToJsonValue<K>::convert(pair.first).toString();
			QJsonValue value = Serializer<V>::toJson(pair.second);
			jsonObject.insert(key, value);
		}
		return jsonObject;
	}

	static std::map<K, V> fromJson(const QJsonValue& json) {
		std::map<K, V> result;
		if (json.isObject()) {
			QJsonObject jsonObject = json.toObject();
			for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
				K key = ToJsonValue<K>::convert(it.key()).toVariant().value<K>();
				V value = Serializer<V>::fromJson(it.value());
				result.insert({ key, value });
			}
		}
		return result;
	}
};

#define JSON_SERIALIZABLE \
    virtual const QMetaObject* metaObject() const { \
    \
        return &this->staticMetaObject; \
    }

// Base class for serializable objects
class JsonSerializable
{
	Q_GADGET
public:
	virtual ~JsonSerializable() = default;
	// Convert QJsonValue in QJsonDocument as QByteArray.
	static QByteArray toByteArray(const QJsonValue& value) {
		return QJsonDocument(value.toObject()).toJson();
	}
	// Serialize all accessed JSON propertyes for this object.
	QJsonObject toJson() const {
		QJsonObject json;
		auto metaObject = this->metaObject();
		int propCount = metaObject->propertyCount();
		for (int i = 0; i < propCount; i++)
		{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			if (QString(metaObject->property(i).typeName()) != QMetaType::typeName(qMetaTypeId<QJsonValue>())) {
				continue;
			}
#else
			if (metaObject->property(i).metaType().id() != QMetaType::QJsonValue) {
				continue;
			}
#endif

			auto key = metaObject->property(i).name();
			auto value = metaObject->property(i).readOnGadget(this).toJsonValue();
			json.insert(key, value);
		}
		return json;
	}

	// Returns QByteArray representation this object using json-serialization.
	QByteArray toRawJson() const {
		return toByteArray(toJson());
	}

	// Deserialize all accessed XML propertyes for this object.
	void fromJson(const QJsonValue& val) {
		if (val.isObject())
		{
			QJsonObject json = val.toObject();
			QStringList keys = json.keys();
			auto metaObject = this->metaObject();
			int propCount = metaObject->propertyCount();
			for (int i = 0; i < propCount; i++)
			{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				if (QString(metaObject->property(i).typeName()) != QMetaType::typeName(qMetaTypeId<QJsonValue>())) {
					continue;
				}
#else
				if (metaObject->property(i).metaType().id() != QMetaType::QJsonValue) {
					continue;
				}
#endif

				QString propertyName = metaObject->property(i).name();
				for (const auto& key : json.keys())
				{
					// 读取JSON属性不区分大小写
					if (key.compare(propertyName, Qt::CaseInsensitive) == 0)
					{
						metaObject->property(i).writeOnGadget(this, json.value(key));
						break;
					}
				}
			}
		}
	}
	// Deserialize all accessed JSON propertyes for this object.
	void fromJson(const QByteArray& data) {
		fromJson(QJsonDocument::fromJson(data).object());
	}

protected:
	virtual const QMetaObject* metaObject() const = 0;
};

// Serializer for custom types inheriting from JsonSerializable
template<typename T>
struct Serializer<T, typename std::enable_if<std::is_base_of<JsonSerializable, T>::value>::type> {
	static QJsonValue toJson(const T& value) {
		return value.toJson();
	}

	static T fromJson(const QJsonValue& json) {
		T result;
		result.fromJson(json.toObject());
		return result;
	}
};

// Macros for easy property declaration
#define JSON_PROPERTY(type, name) \
    Q_PROPERTY(QJsonValue name READ get_json_##name WRITE set_json_##name) \
    private: \
        type m_##name; \
        QJsonValue get_json_##name() const { return Serializer<type>::toJson(m_##name); } \
        void set_json_##name(const QJsonValue& value) { m_##name = Serializer<type>::fromJson(value); } \
    public: \
        type name() const { return m_##name; } \
        void set_##name(const type& value) { m_##name = value; }

#endif // JSON_SERIALIZER_H