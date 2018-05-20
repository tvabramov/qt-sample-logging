#include "mainwindow.h"
#include "loggercategories.h"
#include "logfilewriter.h"
#include <QApplication>
#include <QThread>

QThread *logthread;
LogFileWriter *logwriter;

// Обработчик сообщений qDebug, qInfo, и т.д.
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	logwriter->push(QDateTime::currentDateTime(), type, QString(context.category), msg);
}

int main(int argc, char *argv[])
{
	// Поток и класс для логгирования
	logthread = new QThread();
	logwriter = new LogFileWriter("st-sample-logging", 2048LL * 1024LL * 1024LL);

	logwriter->moveToThread(logthread);
	QObject::connect(logthread, SIGNAL(started()), logwriter, SLOT(mainLoop()));
	// Класс LogFileWriter использует свой цикл mainLoop(), т.е. не выходит из этой функции -> event loop
	// потока оказывается блокированным. Поэтому нужно использовать Qt::DirectConnection
	QObject::connect(logwriter, SIGNAL(finished()), logthread, SLOT(quit()), Qt::DirectConnection);
	logthread->start();

	// Устанавливаем обработчик
	qInstallMessageHandler(messageHandler);

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	int retval = a.exec();

	// Подчищаем за логгером
	qInstallMessageHandler(0);	// Если не сделать, то можем отправить сообщение
								// несуществующему объекту
	logwriter->abort();
	logthread->wait(5000);		// По некоторым причинам, можно ждать очень долго
								// Лучше ограничивать время ожидания
	delete logthread;
	delete logwriter;

	return retval;
}
