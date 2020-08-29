// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: pipeline_state.proto
#pragma once
#ifndef GOOGLE_PROTOBUF_INCLUDED_pipeline_5fstate_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_pipeline_5fstate_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3009000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3009000 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_pipeline_5fstate_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_pipeline_5fstate_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[2]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_pipeline_5fstate_2eproto;
namespace Config {
class Pipeline;
class PipelineDefaultTypeInternal;
extern PipelineDefaultTypeInternal _Pipeline_default_instance_;
class Pipeline_Shader;
class Pipeline_ShaderDefaultTypeInternal;
extern Pipeline_ShaderDefaultTypeInternal _Pipeline_Shader_default_instance_;
}  // namespace Config
PROTOBUF_NAMESPACE_OPEN
template<> ::Config::Pipeline* Arena::CreateMaybeMessage<::Config::Pipeline>(Arena*);
template<> ::Config::Pipeline_Shader* Arena::CreateMaybeMessage<::Config::Pipeline_Shader>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace Config {

enum Pipeline_Shader_Type : int {
  Pipeline_Shader_Type_VERTREX = 0,
  Pipeline_Shader_Type_PIXEL = 1,
  Pipeline_Shader_Type_COMPUTE = 2,
  Pipeline_Shader_Type_GEOMETRY = 3,
  Pipeline_Shader_Type_TESSELATION_DOMAIN = 4,
  Pipeline_Shader_Type_TESSELATION_HULL = 5
};
bool Pipeline_Shader_Type_IsValid(int value);
constexpr Pipeline_Shader_Type Pipeline_Shader_Type_Type_MIN = Pipeline_Shader_Type_VERTREX;
constexpr Pipeline_Shader_Type Pipeline_Shader_Type_Type_MAX = Pipeline_Shader_Type_TESSELATION_HULL;
constexpr int Pipeline_Shader_Type_Type_ARRAYSIZE = Pipeline_Shader_Type_Type_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* Pipeline_Shader_Type_descriptor();
template<typename T>
inline const std::string& Pipeline_Shader_Type_Name(T enum_t_value) {
  static_assert(::std::is_same<T, Pipeline_Shader_Type>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function Pipeline_Shader_Type_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    Pipeline_Shader_Type_descriptor(), enum_t_value);
}
inline bool Pipeline_Shader_Type_Parse(
    const std::string& name, Pipeline_Shader_Type* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<Pipeline_Shader_Type>(
    Pipeline_Shader_Type_descriptor(), name, value);
}
enum Pipeline_Type : int {
  Pipeline_Type_GRAPHICS = 0,
  Pipeline_Type_COMPUTE = 1
};
bool Pipeline_Type_IsValid(int value);
constexpr Pipeline_Type Pipeline_Type_Type_MIN = Pipeline_Type_GRAPHICS;
constexpr Pipeline_Type Pipeline_Type_Type_MAX = Pipeline_Type_COMPUTE;
constexpr int Pipeline_Type_Type_ARRAYSIZE = Pipeline_Type_Type_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* Pipeline_Type_descriptor();
template<typename T>
inline const std::string& Pipeline_Type_Name(T enum_t_value) {
  static_assert(::std::is_same<T, Pipeline_Type>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function Pipeline_Type_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    Pipeline_Type_descriptor(), enum_t_value);
}
inline bool Pipeline_Type_Parse(
    const std::string& name, Pipeline_Type* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<Pipeline_Type>(
    Pipeline_Type_descriptor(), name, value);
}
// ===================================================================

class Pipeline_Shader :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:Config.Pipeline.Shader) */ {
 public:
  Pipeline_Shader();
  virtual ~Pipeline_Shader();

  Pipeline_Shader(const Pipeline_Shader& from);
  Pipeline_Shader(Pipeline_Shader&& from) noexcept
    : Pipeline_Shader() {
    *this = ::std::move(from);
  }

  inline Pipeline_Shader& operator=(const Pipeline_Shader& from) {
    CopyFrom(from);
    return *this;
  }
  inline Pipeline_Shader& operator=(Pipeline_Shader&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const Pipeline_Shader& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Pipeline_Shader* internal_default_instance() {
    return reinterpret_cast<const Pipeline_Shader*>(
               &_Pipeline_Shader_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(Pipeline_Shader& a, Pipeline_Shader& b) {
    a.Swap(&b);
  }
  inline void Swap(Pipeline_Shader* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline Pipeline_Shader* New() const final {
    return CreateMaybeMessage<Pipeline_Shader>(nullptr);
  }

  Pipeline_Shader* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<Pipeline_Shader>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const Pipeline_Shader& from);
  void MergeFrom(const Pipeline_Shader& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  #if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  #else
  bool MergePartialFromCodedStream(
      ::PROTOBUF_NAMESPACE_ID::io::CodedInputStream* input) final;
  #endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  void SerializeWithCachedSizes(
      ::PROTOBUF_NAMESPACE_ID::io::CodedOutputStream* output) const final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Pipeline_Shader* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "Config.Pipeline.Shader";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_pipeline_5fstate_2eproto);
    return ::descriptor_table_pipeline_5fstate_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  typedef Pipeline_Shader_Type Type;
  static constexpr Type VERTREX =
    Pipeline_Shader_Type_VERTREX;
  static constexpr Type PIXEL =
    Pipeline_Shader_Type_PIXEL;
  static constexpr Type COMPUTE =
    Pipeline_Shader_Type_COMPUTE;
  static constexpr Type GEOMETRY =
    Pipeline_Shader_Type_GEOMETRY;
  static constexpr Type TESSELATION_DOMAIN =
    Pipeline_Shader_Type_TESSELATION_DOMAIN;
  static constexpr Type TESSELATION_HULL =
    Pipeline_Shader_Type_TESSELATION_HULL;
  static inline bool Type_IsValid(int value) {
    return Pipeline_Shader_Type_IsValid(value);
  }
  static constexpr Type Type_MIN =
    Pipeline_Shader_Type_Type_MIN;
  static constexpr Type Type_MAX =
    Pipeline_Shader_Type_Type_MAX;
  static constexpr int Type_ARRAYSIZE =
    Pipeline_Shader_Type_Type_ARRAYSIZE;
  static inline const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor*
  Type_descriptor() {
    return Pipeline_Shader_Type_descriptor();
  }
  template<typename T>
  static inline const std::string& Type_Name(T enum_t_value) {
    static_assert(::std::is_same<T, Type>::value ||
      ::std::is_integral<T>::value,
      "Incorrect type passed to function Type_Name.");
    return Pipeline_Shader_Type_Name(enum_t_value);
  }
  static inline bool Type_Parse(const std::string& name,
      Type* value) {
    return Pipeline_Shader_Type_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  enum : int {
    kPathFieldNumber = 1,
    kEntryPointFieldNumber = 2,
    kTypeFieldNumber = 3,
  };
  // required string path = 1;
  bool has_path() const;
  void clear_path();
  const std::string& path() const;
  void set_path(const std::string& value);
  void set_path(std::string&& value);
  void set_path(const char* value);
  void set_path(const char* value, size_t size);
  std::string* mutable_path();
  std::string* release_path();
  void set_allocated_path(std::string* path);

  // required string entry_point = 2;
  bool has_entry_point() const;
  void clear_entry_point();
  const std::string& entry_point() const;
  void set_entry_point(const std::string& value);
  void set_entry_point(std::string&& value);
  void set_entry_point(const char* value);
  void set_entry_point(const char* value, size_t size);
  std::string* mutable_entry_point();
  std::string* release_entry_point();
  void set_allocated_entry_point(std::string* entry_point);

  // required .Config.Pipeline.Shader.Type type = 3;
  bool has_type() const;
  void clear_type();
  ::Config::Pipeline_Shader_Type type() const;
  void set_type(::Config::Pipeline_Shader_Type value);

  // @@protoc_insertion_point(class_scope:Config.Pipeline.Shader)
 private:
  class _Internal;

  // helper for ByteSizeLong()
  size_t RequiredFieldsByteSizeFallback() const;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr path_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr entry_point_;
  int type_;
  friend struct ::TableStruct_pipeline_5fstate_2eproto;
};
// -------------------------------------------------------------------

class Pipeline :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:Config.Pipeline) */ {
 public:
  Pipeline();
  virtual ~Pipeline();

  Pipeline(const Pipeline& from);
  Pipeline(Pipeline&& from) noexcept
    : Pipeline() {
    *this = ::std::move(from);
  }

  inline Pipeline& operator=(const Pipeline& from) {
    CopyFrom(from);
    return *this;
  }
  inline Pipeline& operator=(Pipeline&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }
  inline ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const Pipeline& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Pipeline* internal_default_instance() {
    return reinterpret_cast<const Pipeline*>(
               &_Pipeline_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(Pipeline& a, Pipeline& b) {
    a.Swap(&b);
  }
  inline void Swap(Pipeline* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline Pipeline* New() const final {
    return CreateMaybeMessage<Pipeline>(nullptr);
  }

  Pipeline* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<Pipeline>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const Pipeline& from);
  void MergeFrom(const Pipeline& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  #if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  #else
  bool MergePartialFromCodedStream(
      ::PROTOBUF_NAMESPACE_ID::io::CodedInputStream* input) final;
  #endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
  void SerializeWithCachedSizes(
      ::PROTOBUF_NAMESPACE_ID::io::CodedOutputStream* output) const final;
  ::PROTOBUF_NAMESPACE_ID::uint8* InternalSerializeWithCachedSizesToArray(
      ::PROTOBUF_NAMESPACE_ID::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Pipeline* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "Config.Pipeline";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_pipeline_5fstate_2eproto);
    return ::descriptor_table_pipeline_5fstate_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  typedef Pipeline_Shader Shader;

  typedef Pipeline_Type Type;
  static constexpr Type GRAPHICS =
    Pipeline_Type_GRAPHICS;
  static constexpr Type COMPUTE =
    Pipeline_Type_COMPUTE;
  static inline bool Type_IsValid(int value) {
    return Pipeline_Type_IsValid(value);
  }
  static constexpr Type Type_MIN =
    Pipeline_Type_Type_MIN;
  static constexpr Type Type_MAX =
    Pipeline_Type_Type_MAX;
  static constexpr int Type_ARRAYSIZE =
    Pipeline_Type_Type_ARRAYSIZE;
  static inline const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor*
  Type_descriptor() {
    return Pipeline_Type_descriptor();
  }
  template<typename T>
  static inline const std::string& Type_Name(T enum_t_value) {
    static_assert(::std::is_same<T, Type>::value ||
      ::std::is_integral<T>::value,
      "Incorrect type passed to function Type_Name.");
    return Pipeline_Type_Name(enum_t_value);
  }
  static inline bool Type_Parse(const std::string& name,
      Type* value) {
    return Pipeline_Type_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  enum : int {
    kShadersFieldNumber = 2,
    kTypeFieldNumber = 1,
  };
  // repeated .Config.Pipeline.Shader shaders = 2;
  int shaders_size() const;
  void clear_shaders();
  ::Config::Pipeline_Shader* mutable_shaders(int index);
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::Config::Pipeline_Shader >*
      mutable_shaders();
  const ::Config::Pipeline_Shader& shaders(int index) const;
  ::Config::Pipeline_Shader* add_shaders();
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::Config::Pipeline_Shader >&
      shaders() const;

  // required .Config.Pipeline.Type type = 1;
  bool has_type() const;
  void clear_type();
  ::Config::Pipeline_Type type() const;
  void set_type(::Config::Pipeline_Type value);

  // @@protoc_insertion_point(class_scope:Config.Pipeline)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::Config::Pipeline_Shader > shaders_;
  int type_;
  friend struct ::TableStruct_pipeline_5fstate_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Pipeline_Shader

// required string path = 1;
inline bool Pipeline_Shader::has_path() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Pipeline_Shader::clear_path() {
  path_.ClearToEmptyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  _has_bits_[0] &= ~0x00000001u;
}
inline const std::string& Pipeline_Shader::path() const {
  // @@protoc_insertion_point(field_get:Config.Pipeline.Shader.path)
  return path_.GetNoArena();
}
inline void Pipeline_Shader::set_path(const std::string& value) {
  _has_bits_[0] |= 0x00000001u;
  path_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Config.Pipeline.Shader.path)
}
inline void Pipeline_Shader::set_path(std::string&& value) {
  _has_bits_[0] |= 0x00000001u;
  path_.SetNoArena(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:Config.Pipeline.Shader.path)
}
inline void Pipeline_Shader::set_path(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  _has_bits_[0] |= 0x00000001u;
  path_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Config.Pipeline.Shader.path)
}
inline void Pipeline_Shader::set_path(const char* value, size_t size) {
  _has_bits_[0] |= 0x00000001u;
  path_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Config.Pipeline.Shader.path)
}
inline std::string* Pipeline_Shader::mutable_path() {
  _has_bits_[0] |= 0x00000001u;
  // @@protoc_insertion_point(field_mutable:Config.Pipeline.Shader.path)
  return path_.MutableNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline std::string* Pipeline_Shader::release_path() {
  // @@protoc_insertion_point(field_release:Config.Pipeline.Shader.path)
  if (!has_path()) {
    return nullptr;
  }
  _has_bits_[0] &= ~0x00000001u;
  return path_.ReleaseNonDefaultNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline void Pipeline_Shader::set_allocated_path(std::string* path) {
  if (path != nullptr) {
    _has_bits_[0] |= 0x00000001u;
  } else {
    _has_bits_[0] &= ~0x00000001u;
  }
  path_.SetAllocatedNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), path);
  // @@protoc_insertion_point(field_set_allocated:Config.Pipeline.Shader.path)
}

// required string entry_point = 2;
inline bool Pipeline_Shader::has_entry_point() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Pipeline_Shader::clear_entry_point() {
  entry_point_.ClearToEmptyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  _has_bits_[0] &= ~0x00000002u;
}
inline const std::string& Pipeline_Shader::entry_point() const {
  // @@protoc_insertion_point(field_get:Config.Pipeline.Shader.entry_point)
  return entry_point_.GetNoArena();
}
inline void Pipeline_Shader::set_entry_point(const std::string& value) {
  _has_bits_[0] |= 0x00000002u;
  entry_point_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Config.Pipeline.Shader.entry_point)
}
inline void Pipeline_Shader::set_entry_point(std::string&& value) {
  _has_bits_[0] |= 0x00000002u;
  entry_point_.SetNoArena(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:Config.Pipeline.Shader.entry_point)
}
inline void Pipeline_Shader::set_entry_point(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  _has_bits_[0] |= 0x00000002u;
  entry_point_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Config.Pipeline.Shader.entry_point)
}
inline void Pipeline_Shader::set_entry_point(const char* value, size_t size) {
  _has_bits_[0] |= 0x00000002u;
  entry_point_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Config.Pipeline.Shader.entry_point)
}
inline std::string* Pipeline_Shader::mutable_entry_point() {
  _has_bits_[0] |= 0x00000002u;
  // @@protoc_insertion_point(field_mutable:Config.Pipeline.Shader.entry_point)
  return entry_point_.MutableNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline std::string* Pipeline_Shader::release_entry_point() {
  // @@protoc_insertion_point(field_release:Config.Pipeline.Shader.entry_point)
  if (!has_entry_point()) {
    return nullptr;
  }
  _has_bits_[0] &= ~0x00000002u;
  return entry_point_.ReleaseNonDefaultNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline void Pipeline_Shader::set_allocated_entry_point(std::string* entry_point) {
  if (entry_point != nullptr) {
    _has_bits_[0] |= 0x00000002u;
  } else {
    _has_bits_[0] &= ~0x00000002u;
  }
  entry_point_.SetAllocatedNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), entry_point);
  // @@protoc_insertion_point(field_set_allocated:Config.Pipeline.Shader.entry_point)
}

// required .Config.Pipeline.Shader.Type type = 3;
inline bool Pipeline_Shader::has_type() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void Pipeline_Shader::clear_type() {
  type_ = 0;
  _has_bits_[0] &= ~0x00000004u;
}
inline ::Config::Pipeline_Shader_Type Pipeline_Shader::type() const {
  // @@protoc_insertion_point(field_get:Config.Pipeline.Shader.type)
  return static_cast< ::Config::Pipeline_Shader_Type >(type_);
}
inline void Pipeline_Shader::set_type(::Config::Pipeline_Shader_Type value) {
  assert(::Config::Pipeline_Shader_Type_IsValid(value));
  _has_bits_[0] |= 0x00000004u;
  type_ = value;
  // @@protoc_insertion_point(field_set:Config.Pipeline.Shader.type)
}

// -------------------------------------------------------------------

// Pipeline

// required .Config.Pipeline.Type type = 1;
inline bool Pipeline::has_type() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Pipeline::clear_type() {
  type_ = 0;
  _has_bits_[0] &= ~0x00000001u;
}
inline ::Config::Pipeline_Type Pipeline::type() const {
  // @@protoc_insertion_point(field_get:Config.Pipeline.type)
  return static_cast< ::Config::Pipeline_Type >(type_);
}
inline void Pipeline::set_type(::Config::Pipeline_Type value) {
  assert(::Config::Pipeline_Type_IsValid(value));
  _has_bits_[0] |= 0x00000001u;
  type_ = value;
  // @@protoc_insertion_point(field_set:Config.Pipeline.type)
}

// repeated .Config.Pipeline.Shader shaders = 2;
inline int Pipeline::shaders_size() const {
  return shaders_.size();
}
inline void Pipeline::clear_shaders() {
  shaders_.Clear();
}
inline ::Config::Pipeline_Shader* Pipeline::mutable_shaders(int index) {
  // @@protoc_insertion_point(field_mutable:Config.Pipeline.shaders)
  return shaders_.Mutable(index);
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::Config::Pipeline_Shader >*
Pipeline::mutable_shaders() {
  // @@protoc_insertion_point(field_mutable_list:Config.Pipeline.shaders)
  return &shaders_;
}
inline const ::Config::Pipeline_Shader& Pipeline::shaders(int index) const {
  // @@protoc_insertion_point(field_get:Config.Pipeline.shaders)
  return shaders_.Get(index);
}
inline ::Config::Pipeline_Shader* Pipeline::add_shaders() {
  // @@protoc_insertion_point(field_add:Config.Pipeline.shaders)
  return shaders_.Add();
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::Config::Pipeline_Shader >&
Pipeline::shaders() const {
  // @@protoc_insertion_point(field_list:Config.Pipeline.shaders)
  return shaders_;
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace Config

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::Config::Pipeline_Shader_Type> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::Config::Pipeline_Shader_Type>() {
  return ::Config::Pipeline_Shader_Type_descriptor();
}
template <> struct is_proto_enum< ::Config::Pipeline_Type> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::Config::Pipeline_Type>() {
  return ::Config::Pipeline_Type_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_pipeline_5fstate_2eproto