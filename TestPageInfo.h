#ifndef TEST_PAGE_INFO_H
#define TEST_PAGE_INFO_H

#include "JsonSerializer.h"

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
#endif // !TEST_PAGE_INFO_H
