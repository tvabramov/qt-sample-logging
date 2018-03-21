#include <QTextStream>
#include "logfilewriter.h"

LogFileWriter::LogFileWriter(QString _prefix, qint64 _maxsize, QObject *_parent) :
	QObject(_parent),
	mPrefix(_prefix),
	mMaxFileSize(_maxsize),
	mAbort(false)
{
}

LogFileWriter::~LogFileWriter()
{
	QMutexLocker locker(&mMutex);

	mAbort = true;

	mCondition.wakeAll();
}


void LogFileWriter::push(QDateTime _timestamp, QtMsgType _type, QString _category, QString _msg)
{
	QMutexLocker locker(&mMutex);

	mData.enqueue(LogQuantum(_timestamp, _type, _category, _msg));

	mCondition.wakeAll();
}

void LogFileWriter::abort()
{
	QMutexLocker locker(&mMutex);

	mAbort = true;
	mData.enqueue(LogQuantum(QDateTime::currentDateTime(), QtInfoMsg, "LogFileWriter", "Abort requested"));

	mCondition.wakeAll();
}

void LogFileWriter::mainLoop()
{
	forever {

		mMutex.lock();

		if (mData.isEmpty() && !mAbort)
			mCondition.wait(&mMutex);

		bool abort = mAbort;
		QQueue<LogQuantum> lqq = QQueue<LogQuantum>(std::move(mData));

		mMutex.unlock();

		while (!lqq.isEmpty()) {

			LogQuantum lq = lqq.dequeue();

			// Если файл не открыт, открываем
			if (!mFile.isOpen()) {

				mFile.setFileName(QString("./%1_%2.log").arg(mPrefix).arg(lq.timestamp.toString("yyyy-MM-dd_hh-mm-ss")));
				mFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
				if (mFile.pos() == 0) {

					QTextStream out(&mFile);
					out << "Time\tLevel\tSource\tMessage" << endl;
					out.flush();
				}
			}

			// Если файл слишком большой, переоткрываем его
			if (mFile.size() > mMaxFileSize) {

				mFile.close();
				mFile.remove();
				mFile.setFileName(QString("./%1_%2.log").arg(mPrefix).arg(lq.timestamp.toString("yyyy-MM-dd_hh-mm-ss")));
				mFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

				QTextStream out(&mFile);
				out << "Time\tLevel\tSource\tMessage" << endl;
				out.flush();
			}

			// Открываем поток записи в файл
			QTextStream out(&mFile);
			// Записываем дату записи
			out << lq.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz") << "\t";
			// По типу определяем, к какому уровню относится сообщение
			switch (lq.msgtype)
			{
				case QtInfoMsg:     out << "INF"; break;
				case QtDebugMsg:    out << "DBG"; break;
				case QtWarningMsg:  out << "WRN"; break;
				case QtCriticalMsg: out << "CRT"; break;
				case QtFatalMsg:    out << "FTL"; break;
			}
			out << "\t";

			out << lq.category << "\t";

			// Записываем в вывод категорию сообщения и само сообщение
			out << lq.msg << endl;

			out.flush();
		}

		if (abort) {
			emit finished();
			return;
		}
	}
}
