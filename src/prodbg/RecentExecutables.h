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

    void updateActionList(QVector<QAction*>& list);
    void setFile(QVector<QAction*>& actionList,
                 const QString& filename,
                 BackendType type);
    void putFileOnTop(QVector<QAction*>& actionList, const QString& filename);

    void writeSettings();
    void readSettings();

    struct Executable {
        QString filename;
        BackendType type;
    };

   private:
    void setCurrentFile(const QString& filename, BackendType type);

    QVector<Executable> m_files;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
