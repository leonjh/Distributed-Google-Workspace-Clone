// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: storageserver.proto

#include "storageserver.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG

namespace _pb = ::PROTOBUF_NAMESPACE_ID;
namespace _pbi = _pb::internal;

namespace backend {
PROTOBUF_CONSTEXPR StorageServerRequest::StorageServerRequest(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_._has_bits_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}
  , /*decltype(_impl_.row_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.col_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.value1_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.value2_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.from_primary_)*/false
  , /*decltype(_impl_.sequence_number_)*/0} {}
struct StorageServerRequestDefaultTypeInternal {
  PROTOBUF_CONSTEXPR StorageServerRequestDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~StorageServerRequestDefaultTypeInternal() {}
  union {
    StorageServerRequest _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 StorageServerRequestDefaultTypeInternal _StorageServerRequest_default_instance_;
PROTOBUF_CONSTEXPR SuicideRequest::SuicideRequest(
    ::_pbi::ConstantInitialized) {}
struct SuicideRequestDefaultTypeInternal {
  PROTOBUF_CONSTEXPR SuicideRequestDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~SuicideRequestDefaultTypeInternal() {}
  union {
    SuicideRequest _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 SuicideRequestDefaultTypeInternal _SuicideRequest_default_instance_;
PROTOBUF_CONSTEXPR StorageServerResponse::StorageServerResponse(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_._has_bits_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}
  , /*decltype(_impl_.value_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}} {}
struct StorageServerResponseDefaultTypeInternal {
  PROTOBUF_CONSTEXPR StorageServerResponseDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~StorageServerResponseDefaultTypeInternal() {}
  union {
    StorageServerResponse _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 StorageServerResponseDefaultTypeInternal _StorageServerResponse_default_instance_;
}  // namespace backend
static ::_pb::Metadata file_level_metadata_storageserver_2eproto[3];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_storageserver_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_storageserver_2eproto = nullptr;

const uint32_t TableStruct_storageserver_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerRequest, _impl_._has_bits_),
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerRequest, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerRequest, _impl_.row_),
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerRequest, _impl_.col_),
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerRequest, _impl_.value1_),
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerRequest, _impl_.value2_),
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerRequest, _impl_.from_primary_),
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerRequest, _impl_.sequence_number_),
  ~0u,
  ~0u,
  0,
  1,
  ~0u,
  2,
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::backend::SuicideRequest, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerResponse, _impl_._has_bits_),
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerResponse, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::backend::StorageServerResponse, _impl_.value_),
  0,
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 12, -1, sizeof(::backend::StorageServerRequest)},
  { 18, -1, -1, sizeof(::backend::SuicideRequest)},
  { 24, 31, -1, sizeof(::backend::StorageServerResponse)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::backend::_StorageServerRequest_default_instance_._instance,
  &::backend::_SuicideRequest_default_instance_._instance,
  &::backend::_StorageServerResponse_default_instance_._instance,
};

const char descriptor_table_protodef_storageserver_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\023storageserver.proto\022\007backend\"\270\001\n\024Stora"
  "geServerRequest\022\013\n\003row\030\001 \001(\t\022\013\n\003col\030\002 \001("
  "\t\022\023\n\006value1\030\003 \001(\tH\000\210\001\001\022\023\n\006value2\030\004 \001(\tH\001"
  "\210\001\001\022\024\n\014from_primary\030\005 \001(\010\022\034\n\017sequence_nu"
  "mber\030\006 \001(\005H\002\210\001\001B\t\n\007_value1B\t\n\007_value2B\022\n"
  "\020_sequence_number\"\020\n\016SuicideRequest\"5\n\025S"
  "torageServerResponse\022\022\n\005value\030\001 \001(\tH\000\210\001\001"
  "B\010\n\006_value2\233\003\n\rStorageServer\022M\n\nStorageG"
  "et\022\035.backend.StorageServerRequest\032\036.back"
  "end.StorageServerResponse\"\000\022M\n\nStoragePu"
  "t\022\035.backend.StorageServerRequest\032\036.backe"
  "nd.StorageServerResponse\"\000\022N\n\013StorageCPu"
  "t\022\035.backend.StorageServerRequest\032\036.backe"
  "nd.StorageServerResponse\"\000\022P\n\rStorageDel"
  "ete\022\035.backend.StorageServerRequest\032\036.bac"
  "kend.StorageServerResponse\"\000\022J\n\rSuicideS"
  "witch\022\027.backend.SuicideRequest\032\036.backend"
  ".StorageServerResponse\"\000b\006proto3"
  ;
static ::_pbi::once_flag descriptor_table_storageserver_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_storageserver_2eproto = {
    false, false, 712, descriptor_table_protodef_storageserver_2eproto,
    "storageserver.proto",
    &descriptor_table_storageserver_2eproto_once, nullptr, 0, 3,
    schemas, file_default_instances, TableStruct_storageserver_2eproto::offsets,
    file_level_metadata_storageserver_2eproto, file_level_enum_descriptors_storageserver_2eproto,
    file_level_service_descriptors_storageserver_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_storageserver_2eproto_getter() {
  return &descriptor_table_storageserver_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_storageserver_2eproto(&descriptor_table_storageserver_2eproto);
namespace backend {

// ===================================================================

class StorageServerRequest::_Internal {
 public:
  using HasBits = decltype(std::declval<StorageServerRequest>()._impl_._has_bits_);
  static void set_has_value1(HasBits* has_bits) {
    (*has_bits)[0] |= 1u;
  }
  static void set_has_value2(HasBits* has_bits) {
    (*has_bits)[0] |= 2u;
  }
  static void set_has_sequence_number(HasBits* has_bits) {
    (*has_bits)[0] |= 4u;
  }
};

StorageServerRequest::StorageServerRequest(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:backend.StorageServerRequest)
}
StorageServerRequest::StorageServerRequest(const StorageServerRequest& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  StorageServerRequest* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){from._impl_._has_bits_}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.row_){}
    , decltype(_impl_.col_){}
    , decltype(_impl_.value1_){}
    , decltype(_impl_.value2_){}
    , decltype(_impl_.from_primary_){}
    , decltype(_impl_.sequence_number_){}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _impl_.row_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.row_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_row().empty()) {
    _this->_impl_.row_.Set(from._internal_row(), 
      _this->GetArenaForAllocation());
  }
  _impl_.col_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.col_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_col().empty()) {
    _this->_impl_.col_.Set(from._internal_col(), 
      _this->GetArenaForAllocation());
  }
  _impl_.value1_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.value1_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (from._internal_has_value1()) {
    _this->_impl_.value1_.Set(from._internal_value1(), 
      _this->GetArenaForAllocation());
  }
  _impl_.value2_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.value2_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (from._internal_has_value2()) {
    _this->_impl_.value2_.Set(from._internal_value2(), 
      _this->GetArenaForAllocation());
  }
  ::memcpy(&_impl_.from_primary_, &from._impl_.from_primary_,
    static_cast<size_t>(reinterpret_cast<char*>(&_impl_.sequence_number_) -
    reinterpret_cast<char*>(&_impl_.from_primary_)) + sizeof(_impl_.sequence_number_));
  // @@protoc_insertion_point(copy_constructor:backend.StorageServerRequest)
}

inline void StorageServerRequest::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.row_){}
    , decltype(_impl_.col_){}
    , decltype(_impl_.value1_){}
    , decltype(_impl_.value2_){}
    , decltype(_impl_.from_primary_){false}
    , decltype(_impl_.sequence_number_){0}
  };
  _impl_.row_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.row_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  _impl_.col_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.col_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  _impl_.value1_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.value1_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  _impl_.value2_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.value2_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
}

StorageServerRequest::~StorageServerRequest() {
  // @@protoc_insertion_point(destructor:backend.StorageServerRequest)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void StorageServerRequest::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.row_.Destroy();
  _impl_.col_.Destroy();
  _impl_.value1_.Destroy();
  _impl_.value2_.Destroy();
}

void StorageServerRequest::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void StorageServerRequest::Clear() {
// @@protoc_insertion_point(message_clear_start:backend.StorageServerRequest)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.row_.ClearToEmpty();
  _impl_.col_.ClearToEmpty();
  cached_has_bits = _impl_._has_bits_[0];
  if (cached_has_bits & 0x00000003u) {
    if (cached_has_bits & 0x00000001u) {
      _impl_.value1_.ClearNonDefaultToEmpty();
    }
    if (cached_has_bits & 0x00000002u) {
      _impl_.value2_.ClearNonDefaultToEmpty();
    }
  }
  _impl_.from_primary_ = false;
  _impl_.sequence_number_ = 0;
  _impl_._has_bits_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* StorageServerRequest::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  _Internal::HasBits has_bits{};
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string row = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          auto str = _internal_mutable_row();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "backend.StorageServerRequest.row"));
        } else
          goto handle_unusual;
        continue;
      // string col = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 18)) {
          auto str = _internal_mutable_col();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "backend.StorageServerRequest.col"));
        } else
          goto handle_unusual;
        continue;
      // optional string value1 = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 26)) {
          auto str = _internal_mutable_value1();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "backend.StorageServerRequest.value1"));
        } else
          goto handle_unusual;
        continue;
      // optional string value2 = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 34)) {
          auto str = _internal_mutable_value2();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "backend.StorageServerRequest.value2"));
        } else
          goto handle_unusual;
        continue;
      // bool from_primary = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 40)) {
          _impl_.from_primary_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // optional int32 sequence_number = 6;
      case 6:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 48)) {
          _Internal::set_has_sequence_number(&has_bits);
          _impl_.sequence_number_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  _impl_._has_bits_.Or(has_bits);
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* StorageServerRequest::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:backend.StorageServerRequest)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // string row = 1;
  if (!this->_internal_row().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_row().data(), static_cast<int>(this->_internal_row().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "backend.StorageServerRequest.row");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_row(), target);
  }

  // string col = 2;
  if (!this->_internal_col().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_col().data(), static_cast<int>(this->_internal_col().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "backend.StorageServerRequest.col");
    target = stream->WriteStringMaybeAliased(
        2, this->_internal_col(), target);
  }

  // optional string value1 = 3;
  if (_internal_has_value1()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_value1().data(), static_cast<int>(this->_internal_value1().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "backend.StorageServerRequest.value1");
    target = stream->WriteStringMaybeAliased(
        3, this->_internal_value1(), target);
  }

  // optional string value2 = 4;
  if (_internal_has_value2()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_value2().data(), static_cast<int>(this->_internal_value2().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "backend.StorageServerRequest.value2");
    target = stream->WriteStringMaybeAliased(
        4, this->_internal_value2(), target);
  }

  // bool from_primary = 5;
  if (this->_internal_from_primary() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteBoolToArray(5, this->_internal_from_primary(), target);
  }

  // optional int32 sequence_number = 6;
  if (_internal_has_sequence_number()) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteInt32ToArray(6, this->_internal_sequence_number(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:backend.StorageServerRequest)
  return target;
}

size_t StorageServerRequest::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:backend.StorageServerRequest)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string row = 1;
  if (!this->_internal_row().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_row());
  }

  // string col = 2;
  if (!this->_internal_col().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_col());
  }

  cached_has_bits = _impl_._has_bits_[0];
  if (cached_has_bits & 0x00000003u) {
    // optional string value1 = 3;
    if (cached_has_bits & 0x00000001u) {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
          this->_internal_value1());
    }

    // optional string value2 = 4;
    if (cached_has_bits & 0x00000002u) {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
          this->_internal_value2());
    }

  }
  // bool from_primary = 5;
  if (this->_internal_from_primary() != 0) {
    total_size += 1 + 1;
  }

  // optional int32 sequence_number = 6;
  if (cached_has_bits & 0x00000004u) {
    total_size += ::_pbi::WireFormatLite::Int32SizePlusOne(this->_internal_sequence_number());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData StorageServerRequest::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    StorageServerRequest::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*StorageServerRequest::GetClassData() const { return &_class_data_; }


void StorageServerRequest::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<StorageServerRequest*>(&to_msg);
  auto& from = static_cast<const StorageServerRequest&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:backend.StorageServerRequest)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_row().empty()) {
    _this->_internal_set_row(from._internal_row());
  }
  if (!from._internal_col().empty()) {
    _this->_internal_set_col(from._internal_col());
  }
  cached_has_bits = from._impl_._has_bits_[0];
  if (cached_has_bits & 0x00000003u) {
    if (cached_has_bits & 0x00000001u) {
      _this->_internal_set_value1(from._internal_value1());
    }
    if (cached_has_bits & 0x00000002u) {
      _this->_internal_set_value2(from._internal_value2());
    }
  }
  if (from._internal_from_primary() != 0) {
    _this->_internal_set_from_primary(from._internal_from_primary());
  }
  if (cached_has_bits & 0x00000004u) {
    _this->_internal_set_sequence_number(from._internal_sequence_number());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void StorageServerRequest::CopyFrom(const StorageServerRequest& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:backend.StorageServerRequest)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool StorageServerRequest::IsInitialized() const {
  return true;
}

void StorageServerRequest::InternalSwap(StorageServerRequest* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_._has_bits_[0], other->_impl_._has_bits_[0]);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.row_, lhs_arena,
      &other->_impl_.row_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.col_, lhs_arena,
      &other->_impl_.col_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.value1_, lhs_arena,
      &other->_impl_.value1_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.value2_, lhs_arena,
      &other->_impl_.value2_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(StorageServerRequest, _impl_.sequence_number_)
      + sizeof(StorageServerRequest::_impl_.sequence_number_)
      - PROTOBUF_FIELD_OFFSET(StorageServerRequest, _impl_.from_primary_)>(
          reinterpret_cast<char*>(&_impl_.from_primary_),
          reinterpret_cast<char*>(&other->_impl_.from_primary_));
}

::PROTOBUF_NAMESPACE_ID::Metadata StorageServerRequest::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_storageserver_2eproto_getter, &descriptor_table_storageserver_2eproto_once,
      file_level_metadata_storageserver_2eproto[0]);
}

// ===================================================================

class SuicideRequest::_Internal {
 public:
};

SuicideRequest::SuicideRequest(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase(arena, is_message_owned) {
  // @@protoc_insertion_point(arena_constructor:backend.SuicideRequest)
}
SuicideRequest::SuicideRequest(const SuicideRequest& from)
  : ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase() {
  SuicideRequest* const _this = this; (void)_this;
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:backend.SuicideRequest)
}





const ::PROTOBUF_NAMESPACE_ID::Message::ClassData SuicideRequest::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::CopyImpl,
    ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::MergeImpl,
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*SuicideRequest::GetClassData() const { return &_class_data_; }







::PROTOBUF_NAMESPACE_ID::Metadata SuicideRequest::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_storageserver_2eproto_getter, &descriptor_table_storageserver_2eproto_once,
      file_level_metadata_storageserver_2eproto[1]);
}

// ===================================================================

class StorageServerResponse::_Internal {
 public:
  using HasBits = decltype(std::declval<StorageServerResponse>()._impl_._has_bits_);
  static void set_has_value(HasBits* has_bits) {
    (*has_bits)[0] |= 1u;
  }
};

StorageServerResponse::StorageServerResponse(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:backend.StorageServerResponse)
}
StorageServerResponse::StorageServerResponse(const StorageServerResponse& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  StorageServerResponse* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){from._impl_._has_bits_}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.value_){}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _impl_.value_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.value_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (from._internal_has_value()) {
    _this->_impl_.value_.Set(from._internal_value(), 
      _this->GetArenaForAllocation());
  }
  // @@protoc_insertion_point(copy_constructor:backend.StorageServerResponse)
}

inline void StorageServerResponse::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.value_){}
  };
  _impl_.value_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.value_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
}

StorageServerResponse::~StorageServerResponse() {
  // @@protoc_insertion_point(destructor:backend.StorageServerResponse)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void StorageServerResponse::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.value_.Destroy();
}

void StorageServerResponse::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void StorageServerResponse::Clear() {
// @@protoc_insertion_point(message_clear_start:backend.StorageServerResponse)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  cached_has_bits = _impl_._has_bits_[0];
  if (cached_has_bits & 0x00000001u) {
    _impl_.value_.ClearNonDefaultToEmpty();
  }
  _impl_._has_bits_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* StorageServerResponse::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  _Internal::HasBits has_bits{};
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // optional string value = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          auto str = _internal_mutable_value();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "backend.StorageServerResponse.value"));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  _impl_._has_bits_.Or(has_bits);
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* StorageServerResponse::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:backend.StorageServerResponse)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // optional string value = 1;
  if (_internal_has_value()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_value().data(), static_cast<int>(this->_internal_value().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "backend.StorageServerResponse.value");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_value(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:backend.StorageServerResponse)
  return target;
}

size_t StorageServerResponse::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:backend.StorageServerResponse)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // optional string value = 1;
  cached_has_bits = _impl_._has_bits_[0];
  if (cached_has_bits & 0x00000001u) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_value());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData StorageServerResponse::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    StorageServerResponse::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*StorageServerResponse::GetClassData() const { return &_class_data_; }


void StorageServerResponse::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<StorageServerResponse*>(&to_msg);
  auto& from = static_cast<const StorageServerResponse&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:backend.StorageServerResponse)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_has_value()) {
    _this->_internal_set_value(from._internal_value());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void StorageServerResponse::CopyFrom(const StorageServerResponse& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:backend.StorageServerResponse)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool StorageServerResponse::IsInitialized() const {
  return true;
}

void StorageServerResponse::InternalSwap(StorageServerResponse* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_._has_bits_[0], other->_impl_._has_bits_[0]);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.value_, lhs_arena,
      &other->_impl_.value_, rhs_arena
  );
}

::PROTOBUF_NAMESPACE_ID::Metadata StorageServerResponse::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_storageserver_2eproto_getter, &descriptor_table_storageserver_2eproto_once,
      file_level_metadata_storageserver_2eproto[2]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace backend
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::backend::StorageServerRequest*
Arena::CreateMaybeMessage< ::backend::StorageServerRequest >(Arena* arena) {
  return Arena::CreateMessageInternal< ::backend::StorageServerRequest >(arena);
}
template<> PROTOBUF_NOINLINE ::backend::SuicideRequest*
Arena::CreateMaybeMessage< ::backend::SuicideRequest >(Arena* arena) {
  return Arena::CreateMessageInternal< ::backend::SuicideRequest >(arena);
}
template<> PROTOBUF_NOINLINE ::backend::StorageServerResponse*
Arena::CreateMaybeMessage< ::backend::StorageServerResponse >(Arena* arena) {
  return Arena::CreateMessageInternal< ::backend::StorageServerResponse >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
