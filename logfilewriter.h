#ifndef LOGFILEWRITER_H
#define LOGFILEWRITER_H

#include <QObject>
#include <QMutex>
#include <QFile>
#include <QQueue>
#include <QWaitCondition>
#include <QDateTime>

/** Потокобезопасный класс для записи файла лога.
 *  Рекомендуется перемещать его в поток.
 */

class LogFileWriter : public QObject
{
	Q_OBJECT
public:
	explicit LogFileWriter(QString _prefix = "",
		qint64 _maxsize = 100 * 1024 * 1024,
		QObject *_parent = 0);
	~LogFileWriter();
	void push(QDateTime _timestamp, QtMsgType _type, QString _category, QString _msg);
	void abort();

signals:
	void finished();

public slots:
	void mainLoop();

private:
	struct LogQuantum {
		QDateTime timestamp;
		QtMsgType msgtype;
		QString category;
		QString msg;
		LogQuantum() : timestamp(QDateTime::currentDateTime()), msgtype(QtDebugMsg),
			category(""), msg("") {}
		LogQuantum(QDateTime _timestamp, QtMsgType _type, QString _category, QString _msg) :
			timestamp(_timestamp), msgtype(_type), category(_category), msg(_msg) {}
	};

	QString mPrefix;
	qint64 mMaxFileSize;
	bool mAbort;
	QFile mFile;
	QQueue<LogQuantum> mData;
	QMutex mMutex;
	QWaitCondition mCondition;
};

#endif // LOGFILEWRITER_H
