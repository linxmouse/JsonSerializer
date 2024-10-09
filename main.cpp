#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include "JsonSerializer.h"
#include "TestPagedPerson.h"

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	TestPagedPerson pagedPerson;
	TestPageInfo page;
	page.set_totalNumber(80);
	page.set_totalPage(4);
	page.set_currentPage(1);
	page.set_pageSize(80 / 4);
	pagedPerson.set_page(page);
	TestPerson person1;
	person1.set_age(18);
	person1.set_name("A");
	person1.set_hobbies({ "running", "TV" });
	TestPerson person2;
	person2.set_age(16);
	person2.set_name("B");
	person2.set_hobbies({ "reading", "swimming" });
	TestPerson person3;
	person3.set_age(21);
	person3.set_name("C");
	person3.set_hobbies({ "gaming", "swimming" });
	pagedPerson.set_persons({ person1, person2, person3 });
	auto rawJson = pagedPerson.toRawJson();
	qDebug().noquote() << rawJson;

	QFile wf("example.json");
	if (!wf.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qCritical() << "Can not open file to write!";
		return -1;
	}
	QTextStream wts(&wf);
	wts << rawJson;
	wf.close();
	
	QFile rf("example.json");
	if (!rf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qCritical() << "Can not open file to read!";
		return -1;
	}
	QTextStream rts(&rf);
	auto jsonStr = rts.readAll();
	rf.close();

	TestPagedPerson newPagedPerson;
	newPagedPerson.fromJson(jsonStr.toUtf8());

	return a.exec();
}
