#ifndef TEST_PAGED_PERSON_H
#define TEST_PAGED_PERSON_H

#include "JsonSerializer.h"
#include "TestPageInfo.h"
#include "TestPerson.h"

class TestPagedPerson final: public JsonSerializable
{
	Q_GADGET
	JSON_SERIALIZABLE
public:
	JSON_PROPERTY(TestPageInfo, page)
	//JSON_PROPERTY(QVector<TestPerson>, persons)
	JSON_PROPERTY(std::vector<TestPerson>, persons)
};
#endif // !TEST_PAGED_PERSON_H
