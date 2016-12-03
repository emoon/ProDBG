#pragma once

#include <QtWidgets/QWidget>

namespace prodbg
{
  class MemoryViewPrivate;
  class BackendInterface;

  class MemoryViewWidget : public QWidget
  {
    Q_OBJECT;
    using Base = QWidget;

  public:
    enum DataType
    {
      X8,
      U8,
      S8,

      X16,
      U16,
      S16,

      X32,
      U32,
      S32,

      X64,
      U64,
      S64,

      F32,
      F64,
    };
    
    Q_ENUM(DataType);

    enum Endianess
    {
      Big,
      Little,
    };
    
    Q_ENUM(Endianess);

  public:
    explicit MemoryViewWidget(QWidget* parent = nullptr);
    virtual ~MemoryViewWidget();

  public:
    void setBackendInterface(BackendInterface* interface);

  protected:
    void paintEvent(QPaintEvent* ev) override;
    void contextMenuEvent(QContextMenuEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;

  public:
    DataType dataType() const;
    Q_SLOT void setDataType(DataType t);

    Endianess endianess() const;
    Q_SLOT void setEndianess(Endianess e);

    int elementsPerLine() const;
    Q_SLOT void setElementsPerLine(int count);

  public:
    Q_SLOT void displayNextPage();
    Q_SLOT void displayPrevPage();
    Q_SLOT void displayNextLine();
    Q_SLOT void displayPrevLine();

  private:
    MemoryViewPrivate* m_Private;
  };
}
