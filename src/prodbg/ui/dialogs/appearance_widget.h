#pragma once

#include <QDialog>

class Ui_AppearanceWidget;
class QListWidgetItem;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AppearanceWidget shows settings for font and interface, etc. Everything that
// has to do with colors and such
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AppearanceWidget : public QDialog {
    Q_OBJECT

   public:
    explicit AppearanceWidget(QWidget* parent = nullptr);
    virtual ~AppearanceWidget();

   private:
    Ui_AppearanceWidget* m_ui = nullptr;
};
