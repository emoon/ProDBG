#pragma once

#include <QDialog>

class Ui_PrefsDialog;
class QListWidgetItem;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PrefsDialog is the top view when opening up the preferences settings in the UI.
// It handles the various settings that can be selected on the left side and then it's up to
// each implementation of widget to handle the other settings.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PrefsDialog : public QDialog {
    Q_OBJECT

public:
    explicit PrefsDialog(QWidget* parent = nullptr);
    virtual ~PrefsDialog();

private:
    Q_SLOT void change_layout(QListWidgetItem* curr, QListWidgetItem* prev);
    Ui_PrefsDialog* m_ui = nullptr;
};

// TODO: move
#define sizeof_array(t) (sizeof(t) / sizeof(t[0]))

