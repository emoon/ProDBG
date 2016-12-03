#pragma once

#include <QString>

class QProcess;

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AmigaUAE
{
public:
	AmigaUAE();
	~AmigaUAE();

	void runExecutable(const QString& filename);

private:

	void readSettings();
	bool validateSettings();

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
