#ifndef TEST_PERSON_H
#define TEST_PERSON_H

#include "JsonSerializer.h"

class TestPerson final: public JsonSerializable
{
	Q_GADGET
	JSON_SERIALIZABLE
public:
	JSON_PROPERTY(QString, name)
	JSON_PROPERTY(int, age)
	JSON_PROPERTY(QList<QString>, hobbies)
};
#endif // !PERSON_TEST_H
