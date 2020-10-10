#include "recent_projects.h"
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtWidgets/QAction>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RecentProjects::RecentProjects() {
    read_settings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RecentProjects::~RecentProjects() {
    write_settings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int find_project(const QVector<Project*>& projects, const QString& name) {
    for (int i = 0, c = projects.count(); i < c; ++i) {
        if (projects[i]->m_name == name) {
            return i;
        }
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentProjects::set_current_project(Project* project) {
    int existing_entry = find_project(m_projects, project->m_name);

    if (existing_entry != -1) {
        m_projects.removeAt(existing_entry);
    }

    m_projects.prepend(project);

    // If we are above our limit we remove the last entry
    if (m_projects.count() > MaxProjects_Count) {
        m_projects.removeLast();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentProjects::set_project(QVector<QAction*>& action_list, Project* project) {
    set_current_project(project);
    update_action_list(action_list);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentProjects::put_project_on_top(QVector<QAction*>& action_list, const QString& name) {
    int index = find_project(m_projects, name);

    if (index == -1) {
        return;
    }

    set_project(action_list, m_projects[index]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentProjects::update_action_list(QVector<QAction*>& action_list) {
    int end = 0;
    int project_count = m_projects.count();

    if (project_count <= MaxProjects_Count) {
        end = project_count;
    } else {
        end = MaxProjects_Count;
    }

    for (int i = 0; i < end; ++i) {
        action_list[i]->setShortcut(QKeySequence(QStringLiteral("Ctrl+") + QString::number(i + 1)));
        action_list[i]->setText(m_projects[i]->m_name);
        action_list[i]->setData(m_projects[i]->m_name);
        action_list[i]->setVisible(true);
    }

    for (int i = end; i < MaxProjects_Count; ++i) {
        action_list[i]->setVisible(false);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentProjects::write_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("RecentProjects"));
    settings.beginWriteArray(QStringLiteral("recent_projects"));

    for (int i = 0, c = m_projects.count(); i < c; ++i) {
        settings.setArrayIndex(i);
        m_projects[i]->write_settings(settings);
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecentProjects::read_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));
    settings.beginGroup(QStringLiteral("RecentProjects"));

    int size = settings.beginReadArray(QStringLiteral("recent_projects"));

    m_projects.clear();

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        Project* project = new Project;
        project->read_settings(settings);
        m_projects.append(project);
    }

    settings.endArray();
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
