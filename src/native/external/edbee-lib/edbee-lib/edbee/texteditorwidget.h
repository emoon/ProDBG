/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

//#include <QAbstractScrollArea>
#include <QStringList>
#include <QWidget>

class QResizeEvent;
class QScrollBar;

namespace edbee {

class TextDocument;
class TextEditorAutoCompleteComponent;
class TextEditorCommandMap;
class TextEditorComponent;
class TextEditorConfig;
class TextEditorController;
class TextEditorKeyMap;
class TextEditorScrollArea;
class TextMarginComponent;
class TextRenderer;
class TextSelection;


/// This is the general edbee widget
/// This core functionality of this widget is divided in several seperate
/// compnents. (TextEditorComponent: the main editor, TextMarginComponent: the sidebar with line numbers)
class EDBEE_EXPORT TextEditorWidget : public QWidget
{
    Q_OBJECT
public:

    explicit TextEditorWidget(QWidget *parent = 0);
    virtual ~TextEditorWidget();

    void scrollPositionVisible( int xPosIn, int yPosIn );

    // a whole bunch of getters
    TextEditorController* controller() const;
    TextEditorConfig* config() const;
    TextEditorCommandMap* commandMap() const;
    TextEditorKeyMap* keyMap() const;
    TextDocument* textDocument() const;
    TextRenderer* textRenderer() const;
    TextSelection* textSelection() const;
    TextEditorComponent* textEditorComponent() const;
    TextMarginComponent* textMarginComponent() const;
    TextEditorScrollArea* textScrollArea() const;

    void resetCaretTime();
    void fullUpdate();

    QScrollBar* horizontalScrollBar() const;
    QScrollBar* verticalScrollBar() const;
    void setVerticalScrollBar( QScrollBar* scrollBar );
    void setHorizontalScrollBar( QScrollBar* scrollBar );
	int autoScrollMargin() const;
    void setAutoScrollMargin(int amount=50);
    void setPlaceholderText(const QString& text);

protected:

    virtual void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject *obj, QEvent *event );

    Q_SIGNAL void focusIn(QWidget* event);
    Q_SIGNAL void focusOut(QWidget* event);

    Q_SIGNAL void verticalScrollBarChanged( QScrollBar* newScrollBar );
    Q_SIGNAL void horizontalScrollBarChanged(  QScrollBar* newScrollBar );

protected:

    Q_SLOT void connectVerticalScrollBar();
    Q_SLOT void connectHorizontalScrollBar();

public:

    Q_SLOT void scrollTopToLine( int line );
    Q_SLOT virtual void updateLineAtOffset(int offset);
    Q_SLOT virtual void updateAreaAroundOffset(int offset, int width=8);
    Q_SLOT virtual void updateLine( int line, int length=1 );
    Q_SLOT virtual void updateComponents();

    Q_SLOT virtual void updateGeometryComponents();

    Q_SLOT virtual void updateRendererViewport();

private:

    TextEditorController* controller_;                    ///< This method returns the controller

    TextEditorScrollArea* scrollAreaRef_;                 ///< The scrollarea of the widget
    TextEditorComponent* editCompRef_;                    ///< The editor ref
    TextMarginComponent* marginCompRef_;                  ///< The margin components
    TextEditorAutoCompleteComponent* autoCompleteCompRef_; ///< The autocomplete list widget

	int autoScrollMargin_; //< Customize the autoscroll margin
};

} // edbee
