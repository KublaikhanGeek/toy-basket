// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: rpc_meta.proto

#include "rpc_meta.pb.h"

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
namespace toyBasket
{
class RpcMetaDefaultTypeInternal
{
public:
    ::PROTOBUF_NAMESPACE_ID::internal::ExplicitlyConstructed<RpcMeta> _instance;
} _RpcMeta_default_instance_;
} // namespace toyBasket
static void InitDefaultsscc_info_RpcMeta_rpc_5fmeta_2eproto()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    {
        void* ptr = &::toyBasket::_RpcMeta_default_instance_;
        new (ptr)::toyBasket::RpcMeta();
        ::PROTOBUF_NAMESPACE_ID::internal::OnShutdownDestroyMessage(ptr);
    }
    ::toyBasket::RpcMeta::InitAsDefaultInstance();
}

::PROTOBUF_NAMESPACE_ID::internal::SCCInfo<0> scc_info_RpcMeta_rpc_5fmeta_2eproto
    = { { ATOMIC_VAR_INIT(::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase::kUninitialized), 0, 0,
          InitDefaultsscc_info_RpcMeta_rpc_5fmeta_2eproto },
        {} };

static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_rpc_5fmeta_2eproto[1];
static constexpr ::PROTOBUF_NAMESPACE_ID::EnumDescriptor const** file_level_enum_descriptors_rpc_5fmeta_2eproto
    = nullptr;
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_rpc_5fmeta_2eproto
    = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32
    TableStruct_rpc_5fmeta_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold)
    = {
          ~0u, // no _has_bits_
          PROTOBUF_FIELD_OFFSET(::toyBasket::RpcMeta, _internal_metadata_),
          ~0u, // no _extensions_
          ~0u, // no _oneof_case_
          ~0u, // no _weak_field_map_
          PROTOBUF_FIELD_OFFSET(::toyBasket::RpcMeta, id_),
          PROTOBUF_FIELD_OFFSET(::toyBasket::RpcMeta, server_),
          PROTOBUF_FIELD_OFFSET(::toyBasket::RpcMeta, method_),
          PROTOBUF_FIELD_OFFSET(::toyBasket::RpcMeta, data_size_),
      };
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
    { 0, -1, sizeof(::toyBasket::RpcMeta) },
};

static ::PROTOBUF_NAMESPACE_ID::Message const* const file_default_instances[] = {
    reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::toyBasket::_RpcMeta_default_instance_),
};

const char descriptor_table_protodef_rpc_5fmeta_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold)
    = "\n\016rpc_meta.proto\022\ttoyBasket\"H\n\007RpcMeta\022\n"
      "\n\002id\030\001 \001(\006\022\016\n\006server\030\002 \001(\t\022\016\n\006method\030\003 \001"
      "(\t\022\021\n\tdata_size\030\004 \001(\005b\006proto3";
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* const descriptor_table_rpc_5fmeta_2eproto_deps[1] = {};
static ::PROTOBUF_NAMESPACE_ID::internal::SCCInfoBase* const descriptor_table_rpc_5fmeta_2eproto_sccs[1]           = {
    &scc_info_RpcMeta_rpc_5fmeta_2eproto.base,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_rpc_5fmeta_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_rpc_5fmeta_2eproto = {
    false,
    false,
    descriptor_table_protodef_rpc_5fmeta_2eproto,
    "rpc_meta.proto",
    109,
    &descriptor_table_rpc_5fmeta_2eproto_once,
    descriptor_table_rpc_5fmeta_2eproto_sccs,
    descriptor_table_rpc_5fmeta_2eproto_deps,
    1,
    0,
    schemas,
    file_default_instances,
    TableStruct_rpc_5fmeta_2eproto::offsets,
    file_level_metadata_rpc_5fmeta_2eproto,
    1,
    file_level_enum_descriptors_rpc_5fmeta_2eproto,
    file_level_service_descriptors_rpc_5fmeta_2eproto,
};

// Force running AddDescriptors() at dynamic initialization time.
static bool dynamic_init_dummy_rpc_5fmeta_2eproto
    = (static_cast<void>(::PROTOBUF_NAMESPACE_ID::internal::AddDescriptors(&descriptor_table_rpc_5fmeta_2eproto)),
       true);
namespace toyBasket
{

// ===================================================================

void RpcMeta::InitAsDefaultInstance()
{
}
class RpcMeta::_Internal
{
public:
};

RpcMeta::RpcMeta(::PROTOBUF_NAMESPACE_ID::Arena* arena)
    : ::PROTOBUF_NAMESPACE_ID::Message(arena)
{
    SharedCtor();
    RegisterArenaDtor(arena);
    // @@protoc_insertion_point(arena_constructor:toyBasket.RpcMeta)
}
RpcMeta::RpcMeta(const RpcMeta& from)
    : ::PROTOBUF_NAMESPACE_ID::Message()
{
    _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
    server_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
    if (!from._internal_server().empty())
    {
        server_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from._internal_server(),
                    GetArena());
    }
    method_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
    if (!from._internal_method().empty())
    {
        method_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), from._internal_method(),
                    GetArena());
    }
    ::memcpy(&id_, &from.id_,
             static_cast<size_t>(reinterpret_cast<char*>(&data_size_) - reinterpret_cast<char*>(&id_))
                 + sizeof(data_size_));
    // @@protoc_insertion_point(copy_constructor:toyBasket.RpcMeta)
}

void RpcMeta::SharedCtor()
{
    ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&scc_info_RpcMeta_rpc_5fmeta_2eproto.base);
    server_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
    method_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
    ::memset(&id_, 0,
             static_cast<size_t>(reinterpret_cast<char*>(&data_size_) - reinterpret_cast<char*>(&id_))
                 + sizeof(data_size_));
}

RpcMeta::~RpcMeta()
{
    // @@protoc_insertion_point(destructor:toyBasket.RpcMeta)
    SharedDtor();
    _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

void RpcMeta::SharedDtor()
{
    GOOGLE_DCHECK(GetArena() == nullptr);
    server_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
    method_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void RpcMeta::ArenaDtor(void* object)
{
    RpcMeta* _this = reinterpret_cast<RpcMeta*>(object);
    (void)_this;
}
void RpcMeta::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*)
{
}
void RpcMeta::SetCachedSize(int size) const
{
    _cached_size_.Set(size);
}
const RpcMeta& RpcMeta::default_instance()
{
    ::PROTOBUF_NAMESPACE_ID::internal::InitSCC(&::scc_info_RpcMeta_rpc_5fmeta_2eproto.base);
    return *internal_default_instance();
}

void RpcMeta::Clear()
{
    // @@protoc_insertion_point(message_clear_start:toyBasket.RpcMeta)
    ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
    // Prevent compiler warnings about cached_has_bits being unused
    (void)cached_has_bits;

    server_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
    method_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
    ::memset(&id_, 0,
             static_cast<size_t>(reinterpret_cast<char*>(&data_size_) - reinterpret_cast<char*>(&id_))
                 + sizeof(data_size_));
    _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* RpcMeta::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx)
{
#define CHK_(x)                       \
    if (PROTOBUF_PREDICT_FALSE(!(x))) \
    goto failure
    ::PROTOBUF_NAMESPACE_ID::Arena* arena = GetArena();
    (void)arena;
    while (!ctx->Done(&ptr))
    {
        ::PROTOBUF_NAMESPACE_ID::uint32 tag;
        ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
        CHK_(ptr);
        switch (tag >> 3)
        {
        // fixed64 id = 1;
        case 1:
            if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 9))
            {
                id_ = ::PROTOBUF_NAMESPACE_ID::internal::UnalignedLoad<::PROTOBUF_NAMESPACE_ID::uint64>(ptr);
                ptr += sizeof(::PROTOBUF_NAMESPACE_ID::uint64);
            }
            else
                goto handle_unusual;
            continue;
        // string server = 2;
        case 2:
            if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18))
            {
                auto str = _internal_mutable_server();
                ptr      = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
                CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "toyBasket.RpcMeta.server"));
                CHK_(ptr);
            }
            else
                goto handle_unusual;
            continue;
        // string method = 3;
        case 3:
            if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 26))
            {
                auto str = _internal_mutable_method();
                ptr      = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
                CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "toyBasket.RpcMeta.method"));
                CHK_(ptr);
            }
            else
                goto handle_unusual;
            continue;
        // int32 data_size = 4;
        case 4:
            if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 32))
            {
                data_size_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
                CHK_(ptr);
            }
            else
                goto handle_unusual;
            continue;
        default:
        {
        handle_unusual:
            if ((tag & 7) == 4 || tag == 0)
            {
                ctx->SetLastTag(tag);
                goto success;
            }
            ptr = UnknownFieldParse(
                tag, _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(), ptr, ctx);
            CHK_(ptr != nullptr);
            continue;
        }
        } // switch
    }     // while
success:
    return ptr;
failure:
    ptr = nullptr;
    goto success;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8*
RpcMeta::_InternalSerialize(::PROTOBUF_NAMESPACE_ID::uint8* target,
                            ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const
{
    // @@protoc_insertion_point(serialize_to_array_start:toyBasket.RpcMeta)
    ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
    (void)cached_has_bits;

    // fixed64 id = 1;
    if (this->id() != 0)
    {
        target = stream->EnsureSpace(target);
        target
            = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteFixed64ToArray(1, this->_internal_id(), target);
    }

    // string server = 2;
    if (this->server().size() > 0)
    {
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
            this->_internal_server().data(), static_cast<int>(this->_internal_server().length()),
            ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE, "toyBasket.RpcMeta.server");
        target = stream->WriteStringMaybeAliased(2, this->_internal_server(), target);
    }

    // string method = 3;
    if (this->method().size() > 0)
    {
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
            this->_internal_method().data(), static_cast<int>(this->_internal_method().length()),
            ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE, "toyBasket.RpcMeta.method");
        target = stream->WriteStringMaybeAliased(3, this->_internal_method(), target);
    }

    // int32 data_size = 4;
    if (this->data_size() != 0)
    {
        target = stream->EnsureSpace(target);
        target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteInt32ToArray(4, this->_internal_data_size(),
                                                                                      target);
    }

    if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields()))
    {
        target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(
                ::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance),
            target, stream);
    }
    // @@protoc_insertion_point(serialize_to_array_end:toyBasket.RpcMeta)
    return target;
}

size_t RpcMeta::ByteSizeLong() const
{
    // @@protoc_insertion_point(message_byte_size_start:toyBasket.RpcMeta)
    size_t total_size = 0;

    ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
    // Prevent compiler warnings about cached_has_bits being unused
    (void)cached_has_bits;

    // string server = 2;
    if (this->server().size() > 0)
    {
        total_size += 1 + ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(this->_internal_server());
    }

    // string method = 3;
    if (this->method().size() > 0)
    {
        total_size += 1 + ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(this->_internal_method());
    }

    // fixed64 id = 1;
    if (this->id() != 0)
    {
        total_size += 1 + 8;
    }

    // int32 data_size = 4;
    if (this->data_size() != 0)
    {
        total_size += 1 + ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::Int32Size(this->_internal_data_size());
    }

    if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields()))
    {
        return ::PROTOBUF_NAMESPACE_ID::internal::ComputeUnknownFieldsSize(_internal_metadata_, total_size,
                                                                           &_cached_size_);
    }
    int cached_size = ::PROTOBUF_NAMESPACE_ID::internal::ToCachedSize(total_size);
    SetCachedSize(cached_size);
    return total_size;
}

void RpcMeta::MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from)
{
    // @@protoc_insertion_point(generalized_merge_from_start:toyBasket.RpcMeta)
    GOOGLE_DCHECK_NE(&from, this);
    const RpcMeta* source = ::PROTOBUF_NAMESPACE_ID::DynamicCastToGenerated<RpcMeta>(&from);
    if (source == nullptr)
    {
        // @@protoc_insertion_point(generalized_merge_from_cast_fail:toyBasket.RpcMeta)
        ::PROTOBUF_NAMESPACE_ID::internal::ReflectionOps::Merge(from, this);
    }
    else
    {
        // @@protoc_insertion_point(generalized_merge_from_cast_success:toyBasket.RpcMeta)
        MergeFrom(*source);
    }
}

void RpcMeta::MergeFrom(const RpcMeta& from)
{
    // @@protoc_insertion_point(class_specific_merge_from_start:toyBasket.RpcMeta)
    GOOGLE_DCHECK_NE(&from, this);
    _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
    ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
    (void)cached_has_bits;

    if (from.server().size() > 0)
    {
        _internal_set_server(from._internal_server());
    }
    if (from.method().size() > 0)
    {
        _internal_set_method(from._internal_method());
    }
    if (from.id() != 0)
    {
        _internal_set_id(from._internal_id());
    }
    if (from.data_size() != 0)
    {
        _internal_set_data_size(from._internal_data_size());
    }
}

void RpcMeta::CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from)
{
    // @@protoc_insertion_point(generalized_copy_from_start:toyBasket.RpcMeta)
    if (&from == this)
        return;
    Clear();
    MergeFrom(from);
}

void RpcMeta::CopyFrom(const RpcMeta& from)
{
    // @@protoc_insertion_point(class_specific_copy_from_start:toyBasket.RpcMeta)
    if (&from == this)
        return;
    Clear();
    MergeFrom(from);
}

bool RpcMeta::IsInitialized() const
{
    return true;
}

void RpcMeta::InternalSwap(RpcMeta* other)
{
    using std::swap;
    _internal_metadata_.Swap<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(&other->_internal_metadata_);
    server_.Swap(&other->server_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
    method_.Swap(&other->method_, &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
    ::PROTOBUF_NAMESPACE_ID::internal::memswap<PROTOBUF_FIELD_OFFSET(RpcMeta, data_size_) + sizeof(RpcMeta::data_size_)
                                               - PROTOBUF_FIELD_OFFSET(RpcMeta, id_)>(
        reinterpret_cast<char*>(&id_), reinterpret_cast<char*>(&other->id_));
}

::PROTOBUF_NAMESPACE_ID::Metadata RpcMeta::GetMetadata() const
{
    return GetMetadataStatic();
}

// @@protoc_insertion_point(namespace_scope)
} // namespace toyBasket
PROTOBUF_NAMESPACE_OPEN
template <>
PROTOBUF_NOINLINE ::toyBasket::RpcMeta* Arena::CreateMaybeMessage<::toyBasket::RpcMeta>(Arena* arena)
{
    return Arena::CreateMessageInternal<::toyBasket::RpcMeta>(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
