#include "RecentExecutables.h"
#include <QFileInfo>
#include <QAction>
#include <QSettings>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RecentExecutables::RecentExecutables()
{
    readSettings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RecentExecutables::~RecentExecutables()
{
    writeSettings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int findFileInList(const QVector<RecentExecutables::Executable>& files, const QString& filename) {
    for (int i = 0, c = files.count(); i < c; ++i) {
        if (files[i].filename == filename) {
            return i;
        }
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::setCurrentFile(const QString& filename, BackendType type)
{
    int existingEntry = findFileInList(m_files, filename);

    if (existingEntry != -1) {
        m_files.removeAt(existingEntry);
    }

    Executable exe = { filename, type };

    // slow for vector but we have 8 files so we will live :)
    m_files.prepend(exe);

    // If we are above our limit we remove the last entry
    if (m_files.count() > MaxFiles_Count) {
        m_files.removeLast();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::setFile(QVector<QAction*>& actionList, const QString& filename, BackendType type)
{
    setCurrentFile(filename, type);
    updateActionList(actionList);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::putFileOnTop(QVector<QAction*>& actionList, const QString& filename)
{
    int index = findFileInList(m_files, filename);

    if (index == -1) {
        return;
    }

    BackendType type = m_files[index].type;

    setFile(actionList, filename, type);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::updateActionList(QVector<QAction*>& actionList)
{
    int end = 0;

    if (m_files.count() <= MaxFiles_Count) {
        end = m_files.count();
    } else {
        end = MaxFiles_Count;
    }

    for (int i = 0; i < end; ++i) {
        QString strippedName = QFileInfo(m_files[i].filename).fileName();
        actionList[i]->setShortcut(QKeySequence(QStringLiteral("Ctrl+") + QString::number(i + 1)));
        actionList[i]->setText(strippedName);
        actionList[i]->setData(m_files[i].filename);
        actionList[i]->setVisible(true);
    }

    for (int i = end; i < MaxFiles_Count; ++i) {
        actionList[i]->setVisible(false);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::writeSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("RecentFiles"));
    settings.beginWriteArray(QStringLiteral("recentExes"));

    for (int i = 0, c = m_files.count(); i < c; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(QStringLiteral("file"), m_files[i].filename);
        settings.setValue(QStringLiteral("backendIndex"), (int)m_files[i].type);
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::readSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("RecentFiles"));

    int size = settings.beginReadArray(QStringLiteral("recentExes"));

    m_files.clear();

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString filename = settings.value(QStringLiteral("file")).toString();
        int backend = settings.value(QStringLiteral("backendIndex")).toInt();

        Executable exe = { filename, (BackendType)backend };

        m_files.append(exe);
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
