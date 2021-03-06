// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: pipeline.proto

#include "pipeline.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
extern PROTOBUF_INTERNAL_EXPORT_pipeline_2eproto ::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<0> scc_info_Pipeline_Shader_pipeline_2eproto;
namespace Config {
class Pipeline_ShaderDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<Pipeline_Shader> _instance;
} _Pipeline_Shader_default_instance_;
class PipelineDefaultTypeInternal {
 public:
  ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<Pipeline> _instance;
} _Pipeline_default_instance_;
}  // namespace Config
static void InitDefaultsscc_info_Pipeline_pipeline_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::Config::_Pipeline_default_instance_;
    new (ptr) ::Config::Pipeline();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
  ::Config::Pipeline::InitAsDefaultInstance();
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<1> scc_info_Pipeline_pipeline_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 1, InitDefaultsscc_info_Pipeline_pipeline_2eproto}, {
      &scc_info_Pipeline_Shader_pipeline_2eproto.base,}};

static void InitDefaultsscc_info_Pipeline_Shader_pipeline_2eproto() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  {
    void* ptr = &::Config::_Pipeline_Shader_default_instance_;
    new (ptr) ::Config::Pipeline_Shader();
    ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
  }
  ::Config::Pipeline_Shader::InitAsDefaultInstance();
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<0> scc_info_Pipeline_Shader_pipeline_2eproto =
    {{ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 0, InitDefaultsscc_info_Pipeline_Shader_pipeline_2eproto}, {}};

static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_pipeline_2eproto[2];
static const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* file_level_enum_descriptors_pipeline_2eproto[2];
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_pipeline_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_pipeline_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline_Shader, _has_bits_),
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline_Shader, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline_Shader, path_),
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline_Shader, entry_point_),
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline_Shader, type_),
  0,
  1,
  2,
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline, _has_bits_),
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline, type_),
  PROTOBUF_FIELD_OFFSET(::Config::Pipeline, shaders_),
  0,
  ~0u,
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 8, sizeof(::Config::Pipeline_Shader)},
  { 11, 18, sizeof(::Config::Pipeline)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::Config::_Pipeline_Shader_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::Config::_Pipeline_default_instance_),
};

const char descriptor_table_protodef_pipeline_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\016pipeline.proto\022\006Config\"\276\002\n\010Pipeline\022#\n"
  "\004type\030\001 \002(\0162\025.Config.Pipeline.Type\022(\n\007sh"
  "aders\030\002 \003(\0132\027.Config.Pipeline.Shader\032\277\001\n"
  "\006Shader\022\014\n\004path\030\001 \002(\t\022\023\n\013entry_point\030\002 \002"
  "(\t\022*\n\004type\030\003 \002(\0162\034.Config.Pipeline.Shade"
  "r.Type\"f\n\004Type\022\n\n\006VERTEX\020\000\022\t\n\005PIXEL\020\001\022\013\n"
  "\007COMPUTE\020\002\022\014\n\010GEOMETRY\020\003\022\026\n\022TESSELATION_"
  "DOMAIN\020\004\022\024\n\020TESSELATION_HULL\020\005\"!\n\004Type\022\014"
  "\n\010GRAPHICS\020\000\022\013\n\007COMPUTE\020\001"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_pipeline_2eproto_deps[1] = {
};
static ::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase*const descriptor_table_pipeline_2eproto_sccs[2] = {
  &scc_info_Pipeline_pipeline_2eproto.base,
  &scc_info_Pipeline_Shader_pipeline_2eproto.base,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_pipeline_2eproto_once;
static bool descriptor_table_pipeline_2eproto_initialized = false;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_pipeline_2eproto = {
  &descriptor_table_pipeline_2eproto_initialized, descriptor_table_protodef_pipeline_2eproto, "pipeline.proto", 345,
  &descriptor_table_pipeline_2eproto_once, descriptor_table_pipeline_2eproto_sccs, descriptor_table_pipeline_2eproto_deps, 2, 0,
  schemas, file_default_instances, TableStruct_pipeline_2eproto::offsets,
  file_level_metadata_pipeline_2eproto, 2, file_level_enum_descriptors_pipeline_2eproto, file_level_service_descriptors_pipeline_2eproto,
};

// Force running AddDescriptors() at dynamic initialization time.
static bool dynamic_init_dummy_pipeline_2eproto = (  ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptors(&descriptor_table_pipeline_2eproto), true);
namespace Config {
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* Pipeline_Shader_Type_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_pipeline_2eproto);
  return file_level_enum_descriptors_pipeline_2eproto[0];
}
bool Pipeline_Shader_Type_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      return true;
    default:
      return false;
  }
}

#if (__cplusplus < 201703) && (!defined(_MSC_VER) || _MSC_VER >= 1900)
constexpr Pipeline_Shader_Type Pipeline_Shader::VERTEX;
constexpr Pipeline_Shader_Type Pipeline_Shader::PIXEL;
constexpr Pipeline_Shader_Type Pipeline_Shader::COMPUTE;
constexpr Pipeline_Shader_Type Pipeline_Shader::GEOMETRY;
constexpr Pipeline_Shader_Type Pipeline_Shader::TESSELATION_DOMAIN;
constexpr Pipeline_Shader_Type Pipeline_Shader::TESSELATION_HULL;
constexpr Pipeline_Shader_Type Pipeline_Shader::Type_MIN;
constexpr Pipeline_Shader_Type Pipeline_Shader::Type_MAX;
constexpr int Pipeline_Shader::Type_ARRAYSIZE;
#endif  // (__cplusplus < 201703) && (!defined(_MSC_VER) || _MSC_VER >= 1900)
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* Pipeline_Type_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_pipeline_2eproto);
  return file_level_enum_descriptors_pipeline_2eproto[1];
}
bool Pipeline_Type_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
      return true;
    default:
      return false;
  }
}

#if (__cplusplus < 201703) && (!defined(_MSC_VER) || _MSC_VER >= 1900)
constexpr Pipeline_Type Pipeline::GRAPHICS;
constexpr Pipeline_Type Pipeline::COMPUTE;
constexpr Pipeline_Type Pipeline::Type_MIN;
constexpr Pipeline_Type Pipeline::Type_MAX;
constexpr int Pipeline::Type_ARRAYSIZE;
#endif  // (__cplusplus < 201703) && (!defined(_MSC_VER) || _MSC_VER >= 1900)

// ===================================================================

void Pipeline_Shader::InitAsDefaultInstance() {
}
class Pipeline_Shader::_Internal {
 public:
  using HasBits = decltype(std::declval<Pipeline_Shader>()._has_bits_);
  static void set_has_path(HasBits* has_bits) {
    (*has_bits)[0] |= 1u;
  }
  static void set_has_entry_point(HasBits* has_bits) {
    (*has_bits)[0] |= 2u;
  }
  static void set_has_type(HasBits* has_bits) {
    (*has_bits)[0] |= 4u;
  }
};

Pipeline_Shader::Pipeline_Shader()
  : ::PROTOBUF_NAMESPACE_ID::Message(), _internal_metadata_(nullptr) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:Config.Pipeline.Shader)
}
Pipeline_Shader::Pipeline_Shader(const Pipeline_Shader& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      _internal_metadata_(nullptr),
      _has_bits_(from._has_bits_) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  path_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (from.has_path()) {
    path_.AssignWithDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from.path_);
  }
  entry_point_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (from.has_entry_point()) {
    entry_point_.AssignWithDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from.entry_point_);
  }
  type_ = from.type_;
  // @@protoc_insertion_point(copy_constructor:Config.Pipeline.Shader)
}

void Pipeline_Shader::SharedCtor() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&scc_info_Pipeline_Shader_pipeline_2eproto.base);
  path_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  entry_point_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  type_ = 0;
}

Pipeline_Shader::~Pipeline_Shader() {
  // @@protoc_insertion_point(destructor:Config.Pipeline.Shader)
  SharedDtor();
}

void Pipeline_Shader::SharedDtor() {
  path_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  entry_point_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void Pipeline_Shader::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const Pipeline_Shader& Pipeline_Shader::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_Pipeline_Shader_pipeline_2eproto.base);
  return *internal_default_instance();
}


void Pipeline_Shader::Clear() {
// @@protoc_insertion_point(message_clear_start:Config.Pipeline.Shader)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  if (cached_has_bits & 0x00000003u) {
    if (cached_has_bits & 0x00000001u) {
      path_.ClearNonDefaultToEmptyNoArena();
    }
    if (cached_has_bits & 0x00000002u) {
      entry_point_.ClearNonDefaultToEmptyNoArena();
    }
  }
  type_ = 0;
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

#if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
const char* Pipeline_Shader::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  _Internal::HasBits has_bits{};
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // required string path = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParserUTF8Verify(mutable_path(), ptr, ctx, "Config.Pipeline.Shader.path");
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // required string entry_point = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParserUTF8Verify(mutable_entry_point(), ptr, ctx, "Config.Pipeline.Shader.entry_point");
          CHK_(ptr);
        } else goto handle_unusual;
        continue;
      // required .Config.Pipeline.Shader.Type type = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 24)) {
          ::PROTOBUF_NAMESPACE_ID::uint64 val = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint(&ptr);
          CHK_(ptr);
          if (PROTOBUF_PREDICT_TRUE(::Config::Pipeline_Shader_Type_IsValid(val))) {
            set_type(static_cast<::Config::Pipeline_Shader_Type>(val));
          } else {
            ::PROTOBUF_NAMESPACE_ID::internal::WriteVarint(3, val, mutable_unknown_fields());
          }
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag & 7) == 4 || tag == 0) {
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag, &_internal_metadata_, ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  _has_bits_.Or(has_bits);
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}
#else  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
bool Pipeline_Shader::MergePartialFromCodedStream(
    ::PROTOBUF_NAMESPACE_ID::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!PROTOBUF_PREDICT_TRUE(EXPRESSION)) goto failure
  ::PROTOBUF_NAMESPACE_ID::uint32 tag;
  // @@protoc_insertion_point(parse_start:Config.Pipeline.Shader)
  for (;;) {
    ::std::pair<::PROTOBUF_NAMESPACE_ID::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required string path = 1;
      case 1: {
        if (static_cast< ::PROTOBUF_NAMESPACE_ID::uint8>(tag) == (10 & 0xFF)) {
          DO_(::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::ReadString(
                input, this->mutable_path()));
          ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::VerifyUTF8StringNamedField(
            this->path().data(), static_cast<int>(this->path().length()),
            ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::PARSE,
            "Config.Pipeline.Shader.path");
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required string entry_point = 2;
      case 2: {
        if (static_cast< ::PROTOBUF_NAMESPACE_ID::uint8>(tag) == (18 & 0xFF)) {
          DO_(::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::ReadString(
                input, this->mutable_entry_point()));
          ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::VerifyUTF8StringNamedField(
            this->entry_point().data(), static_cast<int>(this->entry_point().length()),
            ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::PARSE,
            "Config.Pipeline.Shader.entry_point");
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required .Config.Pipeline.Shader.Type type = 3;
      case 3: {
        if (static_cast< ::PROTOBUF_NAMESPACE_ID::uint8>(tag) == (24 & 0xFF)) {
          int value = 0;
          DO_((::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::ReadPrimitive<
                   int, ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::Config::Pipeline_Shader_Type_IsValid(value)) {
            set_type(static_cast< ::Config::Pipeline_Shader_Type >(value));
          } else {
            mutable_unknown_fields()->AddVarint(
                3, static_cast<::PROTOBUF_NAMESPACE_ID::uint64>(value));
          }
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:Config.Pipeline.Shader)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:Config.Pipeline.Shader)
  return false;
#undef DO_
}
#endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER

void Pipeline_Shader::SerializeWithCachedSizes(
    ::PROTOBUF_NAMESPACE_ID::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:Config.Pipeline.Shader)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required string path = 1;
  if (cached_has_bits & 0x00000001u) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::VerifyUTF8StringNamedField(
      this->path().data(), static_cast<int>(this->path().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SERIALIZE,
      "Config.Pipeline.Shader.path");
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->path(), output);
  }

  // required string entry_point = 2;
  if (cached_has_bits & 0x00000002u) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::VerifyUTF8StringNamedField(
      this->entry_point().data(), static_cast<int>(this->entry_point().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SERIALIZE,
      "Config.Pipeline.Shader.entry_point");
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->entry_point(), output);
  }

  // required .Config.Pipeline.Shader.Type type = 3;
  if (cached_has_bits & 0x00000004u) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnum(
      3, this->type(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SerializeUnknownFields(
        _internal_metadata_.unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:Config.Pipeline.Shader)
}

::PROTOBUF_NAMESPACE_ID::uint8* Pipeline_Shader::InternalSerializeWithCachedSizesToArray(
    ::PROTOBUF_NAMESPACE_ID::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:Config.Pipeline.Shader)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required string path = 1;
  if (cached_has_bits & 0x00000001u) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::VerifyUTF8StringNamedField(
      this->path().data(), static_cast<int>(this->path().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SERIALIZE,
      "Config.Pipeline.Shader.path");
    target =
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteStringToArray(
        1, this->path(), target);
  }

  // required string entry_point = 2;
  if (cached_has_bits & 0x00000002u) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::VerifyUTF8StringNamedField(
      this->entry_point().data(), static_cast<int>(this->entry_point().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SERIALIZE,
      "Config.Pipeline.Shader.entry_point");
    target =
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteStringToArray(
        2, this->entry_point(), target);
  }

  // required .Config.Pipeline.Shader.Type type = 3;
  if (cached_has_bits & 0x00000004u) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnumToArray(
      3, this->type(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:Config.Pipeline.Shader)
  return target;
}

size_t Pipeline_Shader::RequiredFieldsByteSizeFallback() const {
// @@protoc_insertion_point(required_fields_byte_size_fallback_start:Config.Pipeline.Shader)
  size_t total_size = 0;

  if (has_path()) {
    // required string path = 1;
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->path());
  }

  if (has_entry_point()) {
    // required string entry_point = 2;
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->entry_point());
  }

  if (has_type()) {
    // required .Config.Pipeline.Shader.Type type = 3;
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::EnumSize(this->type());
  }

  return total_size;
}
size_t Pipeline_Shader::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:Config.Pipeline.Shader)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::ComputeUnknownFieldsSize(
        _internal_metadata_.unknown_fields());
  }
  if (((_has_bits_[0] & 0x00000007) ^ 0x00000007) == 0) {  // All required fields are present.
    // required string path = 1;
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->path());

    // required string entry_point = 2;
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->entry_point());

    // required .Config.Pipeline.Shader.Type type = 3;
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::EnumSize(this->type());

  } else {
    total_size += RequiredFieldsByteSizeFallback();
  }
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void Pipeline_Shader::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:Config.Pipeline.Shader)
  GOOGLE_DCHECK_NE(&from, this);
  const Pipeline_Shader* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<Pipeline_Shader>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:Config.Pipeline.Shader)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:Config.Pipeline.Shader)
    MergeFrom(*source);
  }
}

void Pipeline_Shader::MergeFrom(const Pipeline_Shader& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:Config.Pipeline.Shader)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = from._has_bits_[0];
  if (cached_has_bits & 0x00000007u) {
    if (cached_has_bits & 0x00000001u) {
      _has_bits_[0] |= 0x00000001u;
      path_.AssignWithDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from.path_);
    }
    if (cached_has_bits & 0x00000002u) {
      _has_bits_[0] |= 0x00000002u;
      entry_point_.AssignWithDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from.entry_point_);
    }
    if (cached_has_bits & 0x00000004u) {
      type_ = from.type_;
    }
    _has_bits_[0] |= cached_has_bits;
  }
}

void Pipeline_Shader::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:Config.Pipeline.Shader)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Pipeline_Shader::CopyFrom(const Pipeline_Shader& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:Config.Pipeline.Shader)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Pipeline_Shader::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000007) != 0x00000007) return false;
  return true;
}

void Pipeline_Shader::InternalSwap(Pipeline_Shader* other) {
  using std::swap;
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(_has_bits_[0], other->_has_bits_[0]);
  path_.Swap(&other->path_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  entry_point_.Swap(&other->entry_point_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
    GetArenaNoVirtual());
  swap(type_, other->type_);
}

::PROTOBUF_NAMESPACE_ID::Metadata Pipeline_Shader::GetMetadata() const {
  return GetMetadataStatic();
}


// ===================================================================

void Pipeline::InitAsDefaultInstance() {
}
class Pipeline::_Internal {
 public:
  using HasBits = decltype(std::declval<Pipeline>()._has_bits_);
  static void set_has_type(HasBits* has_bits) {
    (*has_bits)[0] |= 1u;
  }
};

Pipeline::Pipeline()
  : ::PROTOBUF_NAMESPACE_ID::Message(), _internal_metadata_(nullptr) {
  SharedCtor();
  // @@protoc_insertion_point(constructor:Config.Pipeline)
}
Pipeline::Pipeline(const Pipeline& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      _internal_metadata_(nullptr),
      _has_bits_(from._has_bits_),
      shaders_(from.shaders_) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  type_ = from.type_;
  // @@protoc_insertion_point(copy_constructor:Config.Pipeline)
}

void Pipeline::SharedCtor() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&scc_info_Pipeline_pipeline_2eproto.base);
  type_ = 0;
}

Pipeline::~Pipeline() {
  // @@protoc_insertion_point(destructor:Config.Pipeline)
  SharedDtor();
}

void Pipeline::SharedDtor() {
}

void Pipeline::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}
const Pipeline& Pipeline::default_instance() {
  ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_Pipeline_pipeline_2eproto.base);
  return *internal_default_instance();
}


void Pipeline::Clear() {
// @@protoc_insertion_point(message_clear_start:Config.Pipeline)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  shaders_.Clear();
  type_ = 0;
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

#if GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
const char* Pipeline::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  _Internal::HasBits has_bits{};
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    CHK_(ptr);
    switch (tag >> 3) {
      // required .Config.Pipeline.Type type = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          ::PROTOBUF_NAMESPACE_ID::uint64 val = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint(&ptr);
          CHK_(ptr);
          if (PROTOBUF_PREDICT_TRUE(::Config::Pipeline_Type_IsValid(val))) {
            set_type(static_cast<::Config::Pipeline_Type>(val));
          } else {
            ::PROTOBUF_NAMESPACE_ID::internal::WriteVarint(1, val, mutable_unknown_fields());
          }
        } else goto handle_unusual;
        continue;
      // repeated .Config.Pipeline.Shader shaders = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(add_shaders(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::UnalignedLoad<::PROTOBUF_NAMESPACE_ID::uint8>(ptr) == 18);
        } else goto handle_unusual;
        continue;
      default: {
      handle_unusual:
        if ((tag & 7) == 4 || tag == 0) {
          ctx->SetLastTag(tag);
          goto success;
        }
        ptr = UnknownFieldParse(tag, &_internal_metadata_, ptr, ctx);
        CHK_(ptr != nullptr);
        continue;
      }
    }  // switch
  }  // while
success:
  _has_bits_.Or(has_bits);
  return ptr;
failure:
  ptr = nullptr;
  goto success;
#undef CHK_
}
#else  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER
bool Pipeline::MergePartialFromCodedStream(
    ::PROTOBUF_NAMESPACE_ID::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!PROTOBUF_PREDICT_TRUE(EXPRESSION)) goto failure
  ::PROTOBUF_NAMESPACE_ID::uint32 tag;
  // @@protoc_insertion_point(parse_start:Config.Pipeline)
  for (;;) {
    ::std::pair<::PROTOBUF_NAMESPACE_ID::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required .Config.Pipeline.Type type = 1;
      case 1: {
        if (static_cast< ::PROTOBUF_NAMESPACE_ID::uint8>(tag) == (8 & 0xFF)) {
          int value = 0;
          DO_((::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::ReadPrimitive<
                   int, ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::Config::Pipeline_Type_IsValid(value)) {
            set_type(static_cast< ::Config::Pipeline_Type >(value));
          } else {
            mutable_unknown_fields()->AddVarint(
                1, static_cast<::PROTOBUF_NAMESPACE_ID::uint64>(value));
          }
        } else {
          goto handle_unusual;
        }
        break;
      }

      // repeated .Config.Pipeline.Shader shaders = 2;
      case 2: {
        if (static_cast< ::PROTOBUF_NAMESPACE_ID::uint8>(tag) == (18 & 0xFF)) {
          DO_(::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::ReadMessage(
                input, add_shaders()));
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0) {
          goto success;
        }
        DO_(::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SkipField(
              input, tag, _internal_metadata_.mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:Config.Pipeline)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:Config.Pipeline)
  return false;
#undef DO_
}
#endif  // GOOGLE_PROTOBUF_ENABLE_EXPERIMENTAL_PARSER

void Pipeline::SerializeWithCachedSizes(
    ::PROTOBUF_NAMESPACE_ID::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:Config.Pipeline)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required .Config.Pipeline.Type type = 1;
  if (cached_has_bits & 0x00000001u) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnum(
      1, this->type(), output);
  }

  // repeated .Config.Pipeline.Shader shaders = 2;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->shaders_size()); i < n; i++) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteMessageMaybeToArray(
      2,
      this->shaders(static_cast<int>(i)),
      output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SerializeUnknownFields(
        _internal_metadata_.unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:Config.Pipeline)
}

::PROTOBUF_NAMESPACE_ID::uint8* Pipeline::InternalSerializeWithCachedSizesToArray(
    ::PROTOBUF_NAMESPACE_ID::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:Config.Pipeline)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required .Config.Pipeline.Type type = 1;
  if (cached_has_bits & 0x00000001u) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnumToArray(
      1, this->type(), target);
  }

  // repeated .Config.Pipeline.Shader shaders = 2;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->shaders_size()); i < n; i++) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessageToArray(
        2, this->shaders(static_cast<int>(i)), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::SerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:Config.Pipeline)
  return target;
}

size_t Pipeline::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:Config.Pipeline)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::ComputeUnknownFieldsSize(
        _internal_metadata_.unknown_fields());
  }
  // required .Config.Pipeline.Type type = 1;
  if (has_type()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::EnumSize(this->type());
  }
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .Config.Pipeline.Shader shaders = 2;
  {
    unsigned int count = static_cast<unsigned int>(this->shaders_size());
    total_size += 1UL * count;
    for (unsigned int i = 0; i < count; i++) {
      total_size +=
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
          this->shaders(static_cast<int>(i)));
    }
  }

  int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
  SetCachedSize(cached_size);
  return total_size;
}

void Pipeline::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:Config.Pipeline)
  GOOGLE_DCHECK_NE(&from, this);
  const Pipeline* source =
      ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<Pipeline>(
          &from);
  if (source == nullptr) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:Config.Pipeline)
    ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:Config.Pipeline)
    MergeFrom(*source);
  }
}

void Pipeline::MergeFrom(const Pipeline& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:Config.Pipeline)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  shaders_.MergeFrom(from.shaders_);
  if (from.has_type()) {
    set_type(from.type());
  }
}

void Pipeline::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:Config.Pipeline)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Pipeline::CopyFrom(const Pipeline& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:Config.Pipeline)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Pipeline::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;
  if (!::PROTOBUF_NAMESPACE_ID::internal::AllAreInitialized(this->shaders())) return false;
  return true;
}

void Pipeline::InternalSwap(Pipeline* other) {
  using std::swap;
  _internal_metadata_.Swap(&other->_internal_metadata_);
  swap(_has_bits_[0], other->_has_bits_[0]);
  CastToBase(&shaders_)->InternalSwap(CastToBase(&other->shaders_));
  swap(type_, other->type_);
}

::PROTOBUF_NAMESPACE_ID::Metadata Pipeline::GetMetadata() const {
  return GetMetadataStatic();
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace Config
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::Config::Pipeline_Shader* Arena::CreateMaybeMessage< ::Config::Pipeline_Shader >(Arena* arena) {
  return Arena::CreateInternal< ::Config::Pipeline_Shader >(arena);
}
template<> PROTOBUF_NOINLINE ::Config::Pipeline* Arena::CreateMaybeMessage< ::Config::Pipeline >(Arena* arena) {
  return Arena::CreateInternal< ::Config::Pipeline >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
