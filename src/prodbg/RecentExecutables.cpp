#include "RecentExecutables.h"
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtWidgets/QAction>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RecentExecutables::RecentExecutables() {
    read_settings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RecentExecutables::~RecentExecutables() {
    write_settings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int find_file_in_list(const QVector<RecentExecutables::Executable>& files, const QString& filename) {
    for (int i = 0, c = files.count(); i < c; ++i) {
        if (files[i].filename == filename) {
            return i;
        }
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::set_current_file(const QString& filename, BackendType type) {
    int existingEntry = find_file_in_list(m_files, filename);

    if (existingEntry != -1) {
        m_files.removeAt(existingEntry);
    }

    Executable exe = {filename, type};

    // slow for vector but we have 8 files so we will live :)
    m_files.prepend(exe);

    // If we are above our limit we remove the last entry
    if (m_files.count() > MaxFiles_Count) {
        m_files.removeLast();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::set_file(QVector<QAction*>& actionList, const QString& filename, BackendType type) {
    set_current_file(filename, type);
    update_action_list(actionList);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::put_file_on_top(QVector<QAction*>& actionList, const QString& filename) {
    int index = find_file_in_list(m_files, filename);

    if (index == -1) {
        return;
    }

    BackendType type = m_files[index].type;

    set_file(actionList, filename, type);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentExecutables::update_action_list(QVector<QAction*>& actionList) {
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

void RecentExecutables::write_settings() {
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

void RecentExecutables::read_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("RecentFiles"));

    int size = settings.beginReadArray(QStringLiteral("recentExes"));

    m_files.clear();

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString filename = settings.value(QStringLiteral("file")).toString();
        int backend = settings.value(QStringLiteral("backendIndex")).toInt();

        Executable exe = {filename, (BackendType)backend};

        m_files.append(exe);
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
