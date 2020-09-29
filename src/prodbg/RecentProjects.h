#pragma once

#include <QtCore/QVector>
#include "BackendTypes.h"
#include "Project.h"

class QAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RecentProjects {
public:
    RecentProjects();
    ~RecentProjects();

    enum MaxProjects {
        MaxProjects_Count = 8,
    };

    void update_action_list(QVector<QAction*>& list);
    void set_project(QVector<QAction*>& actionList, Project* project);
    void put_project_on_top(QVector<QAction*>& actionList, const QString& name);
    void write_settings();
    void read_settings();

private:
    void set_current_project(Project* project);

    QVector<Project*> m_projects;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
