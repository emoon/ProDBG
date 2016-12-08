#pragma once

#include <QString>
#include <QObject>
#include <QProcess>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmigaUAE : public QObject
{
	Q_OBJECT

public:
	AmigaUAE();
	~AmigaUAE();

	void runExecutable(const QString& filename);
	bool validateSettings();

private:

	Q_SLOT void started();
	Q_SLOT void errorOccurred(QProcess::ProcessError error);

	void readSettings();
	void launchUAE();

	bool m_running = false;

	QString m_uaeExe;
	QString m_config;
	QString m_cmdLineArgs;
	QString m_dh0Path;
	QString m_fileToRun;
	bool m_copyFiles;

	QProcess* m_uaeProcess;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
