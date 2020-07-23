#pragma once

#include <QtCore/QVector>
#include "BackendTypes.h"

class QAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RecentExecutables {
public:
    RecentExecutables();
    ~RecentExecutables();

    enum MaxFiles {
        MaxFiles_Count = 8,
    };

    void update_action_list(QVector<QAction*>& list);
    void set_file(QVector<QAction*>& actionList, const QString& filename, BackendType type);
    void put_file_on_top(QVector<QAction*>& actionList, const QString& filename);

    void write_settings();
    void read_settings();

    struct Executable {
        QString filename;
        BackendType type;
    };

private:
    void set_current_file(const QString& filename, BackendType type);

    QVector<Executable> m_files;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
