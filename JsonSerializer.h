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

/**
 * @brief 通用序列化器模板
 * @tparam T 待序列化的数据类型
 * @tparam Enable 用于模板特化的 SFINAE 辅助类型
 * @details
 * 前置声明通用序列化器模板，提供一个通用的序列化和反序列化接口
 * 使用 SFINAE（替换失败不是错误）技术实现类型特化
 * 默认实现为空，需要为特定类型提供特化版本
 */
template <typename T, typename Enable = void>
struct Serializer;

/**
 * @brief JSON 值转换器的基本模板
 * @tparam T 待转换的数据类型
 * @details 提供将任意类型转换为 QJsonValue 的默认实现
 */
template <typename T>
struct ToJsonValue
{
	/**
	 * @brief 将值转换为 QJsonValue
	 * @param value 待转换的原始值
	 * @return QJsonValue 转换后的 JSON 值
	 */
	static QJsonValue convert(const T &value)
	{
		return QJsonValue::fromVariant(QVariant::fromValue(value));
	}
};

/**
 * @brief QJsonValue 的特殊转换特化
 * @details 对于 QJsonValue 类型，直接返回原值，无需额外转换
 */
template <>
struct ToJsonValue<QJsonValue>
{
	static QJsonValue convert(const QJsonValue &value)
	{
		return value;
	}
};

/**
 * @brief QJsonValue 类型的序列化器特化
 * @details
 * 对于 QJsonValue 类型，直接返回原值，不需要额外转换
 */
template <>
struct Serializer<QJsonValue>
{
	static QJsonValue toJson(const QJsonValue &value)
	{
		return value;
	}

	static QJsonValue fromJson(const QJsonValue &json)
	{
		return json;
	}
};

/**
 * @brief 原始类型（数值和字符串）的序列化器特化
 * @tparam T 待序列化的原始类型
 * @details
 * 使用 std::enable_if 限制模板实例化的类型范围
 * 支持数值类型和 QString
 */
template <typename T>
struct Serializer<T, typename std::enable_if<std::is_arithmetic<T>::value || std::is_same<T, QString>::value>::type>
{
	/**
	 * @brief 将原始类型转换为 QJsonValue
	 * @param value 原始类型的值
	 * @return QJsonValue 转换后的 JSON 值
	 */
	static QJsonValue toJson(const T &value)
	{
		return ToJsonValue<T>::convert(value);
	}

	/**
	 * @brief 从 QJsonValue 还原为原始类型
	 * @param json JSON 值
	 * @return T 还原后的原始类型值
	 */
	static T fromJson(const QJsonValue &json)
	{
		return json.toVariant().value<T>();
	}
};

/**
 * @brief Qt 容器（QList 和 QVector）的序列化器特化
 * @tparam Container 容器类型（QList 或 QVector）
 * @tparam T 容器中元素的类型
 * @details
 * 支持将 QList 和 QVector 序列化为 QJsonArray
 * 使用递归方式序列化容器中的每个元素
 */
template <template <typename> class Container, typename T>
struct Serializer<Container<T>, typename std::enable_if<std::is_same<Container<T>, QList<T>>::value || std::is_same<Container<T>, QVector<T>>::value>::type>
{
	/**
	 * @brief 将容器转换为 QJsonArray
	 * @param container 待序列化的容器
	 * @return QJsonValue 转换后的 JSON 数组
	 */
	static QJsonValue toJson(const Container<T> &container)
	{
		QJsonArray array;
		for (const auto &item : container)
		{
			array.append(Serializer<T>::toJson(item));
		}
		return array;
	}

	/**
	 * @brief 从 QJsonArray 还原为容器
	 * @param json JSON 数组值
	 * @return Container<T> 还原后的容器
	 */
	static Container<T> fromJson(const QJsonValue &json)
	{
		Container<T> result;
		if (json.isArray())
		{
			QJsonArray array = json.toArray();
			for (const auto &item : array)
			{
				result.append(Serializer<T>::fromJson(item));
			}
		}
		return result;
	}
};

/**
 * @brief std::vector 容器的序列化器特化
 * @tparam T 容器中元素的类型
 * @details
 * 支持将 std::vector 序列化为 QJsonArray
 * 与 Qt 容器的序列化器实现类似，但针对 std::vector
 */
template <typename T>
struct Serializer<std::vector<T>>
{
	/**
	 * @brief 将 std::vector 转换为 QJsonArray
	 * @param container 待序列化的 std::vector
	 * @return QJsonValue 转换后的 JSON 数组
	 */
	static QJsonValue toJson(const std::vector<T> &container)
	{
		QJsonArray array;
		for (const auto &item : container)
		{
			array.append(Serializer<T>::toJson(item));
		}
		return array;
	}

	/**
	 * @brief 从 QJsonArray 还原为 std::vector
	 * @param json JSON 数组值
	 * @return std::vector<T> 还原后的 std::vector
	 */
	static std::vector<T> fromJson(const QJsonValue &json)
	{
		std::vector<T> result;
		if (json.isArray())
		{
			QJsonArray array = json.toArray();
			for (const auto &item : array)
			{
				result.push_back(Serializer<T>::fromJson(item));
			}
		}
		return result;
	}
};

/**
 * @brief Qt 关联容器（QMap 和 QHash）的序列化器特化
 * @tparam Map 容器类型（QMap 或 QHash）
 * @tparam K 键的类型
 * @tparam V 值的类型
 * @details
 * 支持将 QMap 和 QHash 序列化为 QJsonObject
 * 使用键的字符串表示作为 JSON 对象的键
 */
template <template <typename, typename> class Map, typename K, typename V>
struct Serializer<Map<K, V>, typename std::enable_if<std::is_same<Map<K, V>, QMap<K, V>>::value || std::is_same<Map<K, V>, QHash<K, V>>::value>::type>
{
	/**
	 * @brief 将 QMap/QHash 转换为 QJsonObject
	 * @param map 待序列化的映射容器
	 * @return QJsonValue 转换后的 JSON 对象
	 */
	static QJsonValue toJson(const Map<K, V> &map)
	{
		QJsonObject obj;
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			obj.insert(ToJsonValue<K>::convert(it.key()).toString(), Serializer<V>::toJson(it.value()));
		}
		return obj;
	}

	/**
	 * @brief 从 QJsonObject 还原为 QMap/QHash
	 * @param json JSON 对象值
	 * @return Map<K, V> 还原后的映射容器
	 */
	static Map<K, V> fromJson(const QJsonValue &json)
	{
		Map<K, V> result;
		if (json.isObject())
		{
			QJsonObject obj = json.toObject();
			for (auto it = obj.begin(); it != obj.end(); ++it)
			{
				result.insert(ToJsonValue<K>::convert(it.key()).toVariant().value<K>(),
							  Serializer<V>::fromJson(it.value()));
			}
		}
		return result;
	}
};

/**
 * @brief std::map 容器的序列化器特化
 * @tparam K 键的类型
 * @tparam V 值的类型
 * @details
 * 支持将 std::map 序列化为 QJsonObject
 * 与 Qt 关联容器的序列化器实现类似，但针对 std::map
 */
template <typename K, typename V>
struct Serializer<std::map<K, V>>
{
	/**
	 * @brief 将 std::map 转换为 QJsonObject
	 * @param map 待序列化的映射容器
	 * @return QJsonValue 转换后的 JSON 对象
	 */
	static QJsonValue toJson(const std::map<K, V> &map)
	{
		QJsonObject jsonObject;
		for (const auto &pair : map)
		{
			QString key = ToJsonValue<K>::convert(pair.first).toString();
			QJsonValue value = Serializer<V>::toJson(pair.second);
			jsonObject.insert(key, value);
		}
		return jsonObject;
	}

	/**
	 * @brief 从 QJsonObject 还原为 std::map
	 * @param json JSON 对象值
	 * @return std::map<K, V> 还原后的映射容器
	 */
	static std::map<K, V> fromJson(const QJsonValue &json)
	{
		std::map<K, V> result;
		if (json.isObject())
		{
			QJsonObject jsonObject = json.toObject();
			for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it)
			{
				K key = ToJsonValue<K>::convert(it.key()).toVariant().value<K>();
				V value = Serializer<V>::fromJson(it.value());
				result.insert({key, value});
			}
		}
		return result;
	}
};

/**
 * @brief JSON 可序列化标记宏
 * @details 为类添加元对象支持，简化元对象方法的实现
 */
#define JSON_SERIALIZABLE                         \
	virtual const QMetaObject *metaObject() const \
	{                                             \
                                                  \
		return &this->staticMetaObject;           \
	}

/**
 * @brief 可序列化基类
 * @details 提供通用的 JSON 序列化和反序列化方法
 * 子类需要实现 metaObject() 方法
 */
class JsonSerializable
{
	Q_GADGET
public:
	virtual ~JsonSerializable() = default;
	/**
	 * @brief 将 QJsonValue 转换为 JSON 字节数组
	 * @param value 待转换的 JSON 值
	 * @return QByteArray JSON 的字节数组表示
	 */
	static QByteArray toByteArray(const QJsonValue &value)
	{
		return QJsonDocument(value.toObject()).toJson();
	}
	
	/**
	 * @brief 序列化对象的所有 JSON 属性
	 * @return QJsonObject 包含对象属性的 JSON 对象
	 */
	QJsonObject toJson() const
	{
		QJsonObject json;
		auto metaObject = this->metaObject();
		int propCount = metaObject->propertyCount();
		for (int i = 0; i < propCount; i++)
		{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			if (QString(metaObject->property(i).typeName()) != QMetaType::typeName(qMetaTypeId<QJsonValue>()))
			{
				continue;
			}
#else
			if (metaObject->property(i).metaType().id() != QMetaType::QJsonValue)
			{
				continue;
			}
#endif

			auto key = metaObject->property(i).name();
			auto value = metaObject->property(i).readOnGadget(this).toJsonValue();
			json.insert(key, value);
		}
		return json;
	}

	/**
	 * @brief 返回对象的 JSON 原始字节数据
	 * @return QByteArray JSON 的原始字节数据
	 */
	QByteArray toRawJson() const
	{
		return toByteArray(toJson());
	}

	/**
	 * @brief 从 JSON 值反序列化对象属性
	 * @param val 包含属性的 JSON 值
	 */
	void fromJson(const QJsonValue &val)
	{
		if (val.isObject())
		{
			QJsonObject json = val.toObject();
			QStringList keys = json.keys();
			auto metaObject = this->metaObject();
			int propCount = metaObject->propertyCount();
			for (int i = 0; i < propCount; i++)
			{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				if (QString(metaObject->property(i).typeName()) != QMetaType::typeName(qMetaTypeId<QJsonValue>()))
				{
					continue;
				}
#else
				if (metaObject->property(i).metaType().id() != QMetaType::QJsonValue)
				{
					continue;
				}
#endif

				QString propertyName = metaObject->property(i).name();
				for (const auto &key : json.keys())
				{
					// Reading JSON properties is case-insensitive
					if (key.compare(propertyName, Qt::CaseInsensitive) == 0)
					{
						metaObject->property(i).writeOnGadget(this, json.value(key));
						break;
					}
				}
			}
		}
	}
	
	/**
	 * @brief 从 JSON 字节数组反序列化对象
	 * @param data JSON 的字节数组
	 */
	void fromJson(const QByteArray &data)
	{
		fromJson(QJsonDocument::fromJson(data).object());
	}

protected:
	virtual const QMetaObject *metaObject() const = 0;
};

/**
 * @brief 自定义类型（继承自 JsonSerializable）的序列化器特化
 * @tparam T 继承自 JsonSerializable 的自定义类型
 * @details
 * 针对继承自 JsonSerializable 的类型提供特化的序列化和反序列化支持
 * 调用对象的 toJson() 和 fromJson() 方法
 */
template <typename T>
struct Serializer<T, typename std::enable_if<std::is_base_of<JsonSerializable, T>::value>::type>
{
	/**
	 * @brief 将自定义对象转换为 QJsonValue
	 * @param value 待序列化的对象
	 * @return QJsonValue 转换后的 JSON 值
	 */
	static QJsonValue toJson(const T &value)
	{
		return value.toJson();
	}

	/**
	 * @brief 从 QJsonValue 还原为自定义对象
	 * @param json JSON 值
	 * @return T 还原后的自定义对象
	 */
	static T fromJson(const QJsonValue &json)
	{
		T result;
		result.fromJson(json.toObject());
		return result;
	}
};

/**
 * @brief JSON 属性声明宏、使用Serializer<T>进行展开
 * @details 简化 JSON 属性的声明、获取和设置
 * @param type 属性的数据类型
 * @param name 属性名称
 */
#define JSON_PROPERTY(type, name)                                                                   \
	Q_PROPERTY(QJsonValue name READ get_json_##name WRITE set_json_##name)                          \
private:                                                                                            \
	type m_##name;                                                                                  \
	QJsonValue get_json_##name() const { return Serializer<type>::toJson(m_##name); }               \
	void set_json_##name(const QJsonValue &value) { m_##name = Serializer<type>::fromJson(value); } \
                                                                                                    \
public:                                                                                             \
	type name() const { return m_##name; }                                                          \
	void set_##name(const type &value) { m_##name = value; }

#endif // JSON_SERIALIZER_H