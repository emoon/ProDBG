#pragma once

#include <stdint.h>

namespace prodbg {

  struct MemoryReadResult
  {
    uint64_t startRange;
    uint64_t endRange;
  };

  class MemoryViewInterface : public QObject
  {
    Q_OBJECT;

  public:
    enum MemoryAddressFlags
    {
      kReadable = 1 << 8,
      kWritable = 1 << 9
    };

  public:
    // Return the number of bytes an address uses. E.g. 4 for a 32-bit target.
    virtual int addressWidthBytes() = 0;

    virtual bool resolveAddress(const QString& expression, uint64_t* out) = 0;

  public:
    virtual void beginReadBytes(uint64_t lo, uint64_t hi, QVector<uint16_t>* target) = 0;

  public:
    //Q_SIGNAL void dataRead(QVector<uint16_t>* result, uint64_t 
  };
}
