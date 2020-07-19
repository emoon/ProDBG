/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>
#include <QIcon>

#include "edbee/models/textbuffer.h"

class QAction;

namespace edbee {

class ChangeGroup;
class DynamicVariables;
class TextBufferChange;
class TextCaretCache;
class TextCommand;
class TextDocument;
class TextEditorCommand;
class TextEditorKeyMap;
class TextEditorCommandMap;
class TextEditorComponent;
class TextEditorWidget;
class TextRenderer;
class TextRangeSet;
class TextSearcher;
class TextSelection;
class UndoableTextCommand;


/// The texteditor works via the controller. The controller is the central point/mediater
/// which maps/controls all messages between the different editor componenents
class EDBEE_EXPORT TextEditorController : public QObject
{
    Q_OBJECT
public:

    enum AutoScrollToCaret {
        AutoScrollAlways,
        AutoScrollWhenFocus,
        AutoScrollNever
    };


    explicit TextEditorController( TextEditorWidget* widget=0, QObject *parent = 0);
    virtual ~TextEditorController();

// public method
    void notifyStateChange();

    void giveTextDocument( TextDocument* doc );
    void setTextDocument( TextDocument* doc );

    void setAutoScrollToCaret( AutoScrollToCaret autoScroll );
    virtual AutoScrollToCaret autoScrollToCaret() const;

    bool hasFocus();
    QAction* createUnconnectedAction(const QString& command, const QString& text, const QIcon& icon=QIcon(), QObject* owner=0 );
    QAction* createAction(const QString& command, const QString& text , const QIcon& icon=QIcon(), QObject* owner=0 );


// getters
//    TextBuffer* textBuffer() const;
    TextDocument* textDocument() const;
    TextSelection* textSelection() const;
    TextRenderer* textRenderer() const;
    TextRangeSet* borderedTextRanges() const;

    void setKeyMap( TextEditorKeyMap* keyMap );
    void giveKeyMap( TextEditorKeyMap* keyMap );
    TextEditorKeyMap* keyMap() const;
    void setCommandMap( TextEditorCommandMap* commandMap );
    void giveCommandMap( TextEditorCommandMap* commandMap );
    TextEditorCommandMap* commandMap() const;
    TextEditorWidget* widget() const;
    TextCaretCache* textCaretCache() const;
    void giveTextSearcher( TextSearcher* searcher );
    TextSearcher* textSearcher();
    DynamicVariables* dynamicVariables() const;

    /// This signal is fired if the statusbar needs updating
    Q_SIGNAL void updateStatusTextSignal( const QString& text );

    /// This signal is fired if the textdocument changes.
    Q_SIGNAL void textDocumentChanged( edbee::TextDocument* oldDocument, edbee::TextDocument* newDocument );

    /// this method is executed when a command is going to be executed
    Q_SIGNAL void commandToBeExecuted( edbee::TextEditorCommand* command );
    Q_SIGNAL void commandExecuted( edbee::TextEditorCommand* command );

    Q_SIGNAL void backspacePressed();

public:

    Q_SLOT void onTextChanged( edbee::TextBufferChange change );
    Q_SLOT void onSelectionChanged( edbee::TextRangeSet *oldRangeSet );
    Q_SLOT void onLineDataChanged( int line, int length, int newLength );

    Q_SLOT void updateAfterConfigChange();

public:

    // updates the status text
    Q_SLOT virtual void updateStatusText( const QString& extraText=QString() );

    Q_SLOT virtual void update();

    // scrolling
    Q_SLOT virtual void scrollPositionVisible( int xPos, int yPos );
    Q_SLOT virtual void scrollOffsetVisible( int offset );
    Q_SLOT virtual void scrollCaretVisible();

    Q_SLOT virtual void storeSelection( int coalesceId=0 );

    // replace the given selection
    Q_SLOT virtual void replace( int offset, int length, const QString& text, int coalesceId );
    Q_SLOT virtual void replaceSelection( const QString& text, int coalesceId=0 );
    Q_SLOT virtual void replaceSelection( const QStringList& texts, int coalesceId=0 );
    Q_SLOT virtual void replaceRangeSet(TextRangeSet& rangeSet, const QString& text, int coalesceId=0 );
    Q_SLOT virtual void replaceRangeSet(TextRangeSet& rangeSet, const QStringList& texts, int coalesceId=0 );

    // caret movements
    Q_SLOT virtual void moveCaretTo( int line, int col, bool keepAnchors, int rangeIndex=-1 );
    Q_SLOT virtual void moveCaretToOffset( int offset, bool keepAnchors, int rangeIndex=-1 );
    Q_SLOT virtual void addCaretAt( int line, int col);
    Q_SLOT virtual void addCaretAtOffset( int offset );
    Q_SLOT virtual void changeAndGiveTextSelection(TextRangeSet* rangeSet , int coalesceId = 0);

    // perform an undo
    Q_SLOT virtual void undo(bool soft=false);
    Q_SLOT virtual void redo(bool soft=false);

    // command execution
    Q_SLOT virtual void beginUndoGroup( ChangeGroup* group=0 );
    Q_SLOT virtual void endUndoGroup(int coalesceId=0, bool flatten=false);

    // low level command execution
    Q_SLOT virtual void executeCommand( TextEditorCommand* textCommand );
    Q_SLOT virtual bool executeCommand( const QString& name=QString() );

private:

    TextEditorWidget* widgetRef_;             ///< A reference to the text editor widget
    TextDocument* textDocument_;              ///< The text document (only filled when owned)
    TextDocument* textDocumentRef_;           ///< The reference to the text-document

    TextSelection* textSelection_;            ///< The text selection

    TextEditorKeyMap* keyMap_;                ///< The ownership of the keymap
    TextEditorKeyMap* keyMapRef_;             ///< A reference to the keymap
    TextEditorCommandMap* commandMap_;        ///< the ownership
    TextEditorCommandMap* commandMapRef_;     ///< A reference to the command
    TextRenderer* textRenderer_;              ///< The text renderer
    TextCaretCache* textCaretCache_;          ///< The text-caret cache. (For remembering the x-position of the current carrets)

    TextSearcher* textSearcher_;              ///< The text-searcher

    AutoScrollToCaret autoScrollToCaret_;     ///< This flags tells the editor to automaticly scrol to the caret


    // extra highlight text
    TextRangeSet* borderedTextRanges_;       ///< Extra marked text ranges
};

} // edbee
