// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_PDMESSAGES_H_
#define FLATBUFFERS_GENERATED_PDMESSAGES_H_

#include "flatbuffers/flatbuffers.h"

struct FileTargetRequest;
struct FileTargetRequestBuilder;

struct TargetReply;
struct TargetReplyBuilder;

struct ExceptionLocationRequest;
struct ExceptionLocationRequestBuilder;

struct ExceptionLocationReply;
struct ExceptionLocationReplyBuilder;

struct Message;
struct MessageBuilder;

enum MessageType {
  MessageType_NONE = 0,
  MessageType_file_target_request = 1,
  MessageType_target_reply = 2,
  MessageType_exception_location_request = 3,
  MessageType_exception_location_reply = 4,
  MessageType_MIN = MessageType_NONE,
  MessageType_MAX = MessageType_exception_location_reply
};

inline const MessageType (&EnumValuesMessageType())[5] {
  static const MessageType values[] = {
    MessageType_NONE,
    MessageType_file_target_request,
    MessageType_target_reply,
    MessageType_exception_location_request,
    MessageType_exception_location_reply
  };
  return values;
}

inline const char * const *EnumNamesMessageType() {
  static const char * const names[6] = {
    "NONE",
    "file_target_request",
    "target_reply",
    "exception_location_request",
    "exception_location_reply",
    nullptr
  };
  return names;
}

inline const char *EnumNameMessageType(MessageType e) {
  if (flatbuffers::IsOutRange(e, MessageType_NONE, MessageType_exception_location_reply)) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMessageType()[index];
}

template<typename T> struct MessageTypeTraits {
  static const MessageType enum_value = MessageType_NONE;
};

template<> struct MessageTypeTraits<FileTargetRequest> {
  static const MessageType enum_value = MessageType_file_target_request;
};

template<> struct MessageTypeTraits<TargetReply> {
  static const MessageType enum_value = MessageType_target_reply;
};

template<> struct MessageTypeTraits<ExceptionLocationRequest> {
  static const MessageType enum_value = MessageType_exception_location_request;
};

template<> struct MessageTypeTraits<ExceptionLocationReply> {
  static const MessageType enum_value = MessageType_exception_location_reply;
};

bool VerifyMessageType(flatbuffers::Verifier &verifier, const void *obj, MessageType type);
bool VerifyMessageTypeVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

struct FileTargetRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef FileTargetRequestBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_PATH = 4
  };
  const flatbuffers::String *path() const {
    return GetPointer<const flatbuffers::String *>(VT_PATH);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_PATH) &&
           verifier.VerifyString(path()) &&
           verifier.EndTable();
  }
};

struct FileTargetRequestBuilder {
  typedef FileTargetRequest Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_path(flatbuffers::Offset<flatbuffers::String> path) {
    fbb_.AddOffset(FileTargetRequest::VT_PATH, path);
  }
  explicit FileTargetRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<FileTargetRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<FileTargetRequest>(end);
    return o;
  }
};

inline flatbuffers::Offset<FileTargetRequest> CreateFileTargetRequest(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> path = 0) {
  FileTargetRequestBuilder builder_(_fbb);
  builder_.add_path(path);
  return builder_.Finish();
}

inline flatbuffers::Offset<FileTargetRequest> CreateFileTargetRequestDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *path = nullptr) {
  auto path__ = path ? _fbb.CreateString(path) : 0;
  return CreateFileTargetRequest(
      _fbb,
      path__);
}

struct TargetReply FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef TargetReplyBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_STATUS = 4,
    VT_ERROR_MESSAGE = 6
  };
  bool status() const {
    return GetField<uint8_t>(VT_STATUS, 0) != 0;
  }
  const flatbuffers::String *error_message() const {
    return GetPointer<const flatbuffers::String *>(VT_ERROR_MESSAGE);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_STATUS) &&
           VerifyOffset(verifier, VT_ERROR_MESSAGE) &&
           verifier.VerifyString(error_message()) &&
           verifier.EndTable();
  }
};

struct TargetReplyBuilder {
  typedef TargetReply Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_status(bool status) {
    fbb_.AddElement<uint8_t>(TargetReply::VT_STATUS, static_cast<uint8_t>(status), 0);
  }
  void add_error_message(flatbuffers::Offset<flatbuffers::String> error_message) {
    fbb_.AddOffset(TargetReply::VT_ERROR_MESSAGE, error_message);
  }
  explicit TargetReplyBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<TargetReply> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<TargetReply>(end);
    return o;
  }
};

inline flatbuffers::Offset<TargetReply> CreateTargetReply(
    flatbuffers::FlatBufferBuilder &_fbb,
    bool status = false,
    flatbuffers::Offset<flatbuffers::String> error_message = 0) {
  TargetReplyBuilder builder_(_fbb);
  builder_.add_error_message(error_message);
  builder_.add_status(status);
  return builder_.Finish();
}

inline flatbuffers::Offset<TargetReply> CreateTargetReplyDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    bool status = false,
    const char *error_message = nullptr) {
  auto error_message__ = error_message ? _fbb.CreateString(error_message) : 0;
  return CreateTargetReply(
      _fbb,
      status,
      error_message__);
}

struct ExceptionLocationRequest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ExceptionLocationRequestBuilder Builder;
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           verifier.EndTable();
  }
};

struct ExceptionLocationRequestBuilder {
  typedef ExceptionLocationRequest Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  explicit ExceptionLocationRequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ExceptionLocationRequest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ExceptionLocationRequest>(end);
    return o;
  }
};

inline flatbuffers::Offset<ExceptionLocationRequest> CreateExceptionLocationRequest(
    flatbuffers::FlatBufferBuilder &_fbb) {
  ExceptionLocationRequestBuilder builder_(_fbb);
  return builder_.Finish();
}

struct ExceptionLocationReply FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef ExceptionLocationReplyBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_FILENAME = 4,
    VT_LINE = 6,
    VT_ADDRESS = 8
  };
  const flatbuffers::String *filename() const {
    return GetPointer<const flatbuffers::String *>(VT_FILENAME);
  }
  int32_t line() const {
    return GetField<int32_t>(VT_LINE, 0);
  }
  uint64_t address() const {
    return GetField<uint64_t>(VT_ADDRESS, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_FILENAME) &&
           verifier.VerifyString(filename()) &&
           VerifyField<int32_t>(verifier, VT_LINE) &&
           VerifyField<uint64_t>(verifier, VT_ADDRESS) &&
           verifier.EndTable();
  }
};

struct ExceptionLocationReplyBuilder {
  typedef ExceptionLocationReply Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_filename(flatbuffers::Offset<flatbuffers::String> filename) {
    fbb_.AddOffset(ExceptionLocationReply::VT_FILENAME, filename);
  }
  void add_line(int32_t line) {
    fbb_.AddElement<int32_t>(ExceptionLocationReply::VT_LINE, line, 0);
  }
  void add_address(uint64_t address) {
    fbb_.AddElement<uint64_t>(ExceptionLocationReply::VT_ADDRESS, address, 0);
  }
  explicit ExceptionLocationReplyBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<ExceptionLocationReply> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ExceptionLocationReply>(end);
    return o;
  }
};

inline flatbuffers::Offset<ExceptionLocationReply> CreateExceptionLocationReply(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> filename = 0,
    int32_t line = 0,
    uint64_t address = 0) {
  ExceptionLocationReplyBuilder builder_(_fbb);
  builder_.add_address(address);
  builder_.add_line(line);
  builder_.add_filename(filename);
  return builder_.Finish();
}

inline flatbuffers::Offset<ExceptionLocationReply> CreateExceptionLocationReplyDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *filename = nullptr,
    int32_t line = 0,
    uint64_t address = 0) {
  auto filename__ = filename ? _fbb.CreateString(filename) : 0;
  return CreateExceptionLocationReply(
      _fbb,
      filename__,
      line,
      address);
}

struct Message FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  typedef MessageBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_MESSAGE_TYPE = 4,
    VT_MESSAGE = 6,
    VT_USER_DATA = 8
  };
  MessageType message_type() const {
    return static_cast<MessageType>(GetField<uint8_t>(VT_MESSAGE_TYPE, 0));
  }
  const void *message() const {
    return GetPointer<const void *>(VT_MESSAGE);
  }
  template<typename T> const T *message_as() const;
  const FileTargetRequest *message_as_file_target_request() const {
    return message_type() == MessageType_file_target_request ? static_cast<const FileTargetRequest *>(message()) : nullptr;
  }
  const TargetReply *message_as_target_reply() const {
    return message_type() == MessageType_target_reply ? static_cast<const TargetReply *>(message()) : nullptr;
  }
  const ExceptionLocationRequest *message_as_exception_location_request() const {
    return message_type() == MessageType_exception_location_request ? static_cast<const ExceptionLocationRequest *>(message()) : nullptr;
  }
  const ExceptionLocationReply *message_as_exception_location_reply() const {
    return message_type() == MessageType_exception_location_reply ? static_cast<const ExceptionLocationReply *>(message()) : nullptr;
  }
  const flatbuffers::Vector<uint8_t> *user_data() const {
    return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_USER_DATA);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_MESSAGE_TYPE) &&
           VerifyOffset(verifier, VT_MESSAGE) &&
           VerifyMessageType(verifier, message(), message_type()) &&
           VerifyOffset(verifier, VT_USER_DATA) &&
           verifier.VerifyVector(user_data()) &&
           verifier.EndTable();
  }
};

template<> inline const FileTargetRequest *Message::message_as<FileTargetRequest>() const {
  return message_as_file_target_request();
}

template<> inline const TargetReply *Message::message_as<TargetReply>() const {
  return message_as_target_reply();
}

template<> inline const ExceptionLocationRequest *Message::message_as<ExceptionLocationRequest>() const {
  return message_as_exception_location_request();
}

template<> inline const ExceptionLocationReply *Message::message_as<ExceptionLocationReply>() const {
  return message_as_exception_location_reply();
}

struct MessageBuilder {
  typedef Message Table;
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_message_type(MessageType message_type) {
    fbb_.AddElement<uint8_t>(Message::VT_MESSAGE_TYPE, static_cast<uint8_t>(message_type), 0);
  }
  void add_message(flatbuffers::Offset<void> message) {
    fbb_.AddOffset(Message::VT_MESSAGE, message);
  }
  void add_user_data(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> user_data) {
    fbb_.AddOffset(Message::VT_USER_DATA, user_data);
  }
  explicit MessageBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  flatbuffers::Offset<Message> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Message>(end);
    return o;
  }
};

inline flatbuffers::Offset<Message> CreateMessage(
    flatbuffers::FlatBufferBuilder &_fbb,
    MessageType message_type = MessageType_NONE,
    flatbuffers::Offset<void> message = 0,
    flatbuffers::Offset<flatbuffers::Vector<uint8_t>> user_data = 0) {
  MessageBuilder builder_(_fbb);
  builder_.add_user_data(user_data);
  builder_.add_message(message);
  builder_.add_message_type(message_type);
  return builder_.Finish();
}

inline flatbuffers::Offset<Message> CreateMessageDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    MessageType message_type = MessageType_NONE,
    flatbuffers::Offset<void> message = 0,
    const std::vector<uint8_t> *user_data = nullptr) {
  auto user_data__ = user_data ? _fbb.CreateVector<uint8_t>(*user_data) : 0;
  return CreateMessage(
      _fbb,
      message_type,
      message,
      user_data__);
}

inline bool VerifyMessageType(flatbuffers::Verifier &verifier, const void *obj, MessageType type) {
  switch (type) {
    case MessageType_NONE: {
      return true;
    }
    case MessageType_file_target_request: {
      auto ptr = reinterpret_cast<const FileTargetRequest *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case MessageType_target_reply: {
      auto ptr = reinterpret_cast<const TargetReply *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case MessageType_exception_location_request: {
      auto ptr = reinterpret_cast<const ExceptionLocationRequest *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case MessageType_exception_location_reply: {
      auto ptr = reinterpret_cast<const ExceptionLocationReply *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return true;
  }
}

inline bool VerifyMessageTypeVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyMessageType(
        verifier,  values->Get(i), types->GetEnum<MessageType>(i))) {
      return false;
    }
  }
  return true;
}

inline const Message *GetMessage(const void *buf) {
  return flatbuffers::GetRoot<Message>(buf);
}

inline const Message *GetSizePrefixedMessage(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<Message>(buf);
}

inline bool VerifyMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<Message>(nullptr);
}

inline bool VerifySizePrefixedMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<Message>(nullptr);
}

inline void FinishMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<Message> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<Message> root) {
  fbb.FinishSizePrefixed(root);
}

#endif  // FLATBUFFERS_GENERATED_PDMESSAGES_H_
