#ifndef SYSBVM_DWARF_H
#define SYSBVM_DWARF_H

#include "common.h"
#include "dynarray.h"
#include <stdbool.h>

enum dwarf_tag_e {
    DW_TAG_array_type = 1,
    DW_TAG_class_type = 2,
    DW_TAG_entry_point = 3,
    DW_TAG_enumeration_type = 4,
    DW_TAG_formal_parameter = 5,
    DW_TAG_imported_declaration = 8,
    DW_TAG_label = 10,
    DW_TAG_lexical_block = 11,
    DW_TAG_member = 13,
    DW_TAG_pointer_type = 15,
    DW_TAG_reference_type = 16,
    DW_TAG_compile_unit = 17,
    DW_TAG_string_type = 18,
    DW_TAG_structure_type = 19,
    DW_TAG_subroutine_type = 21,
    DW_TAG_typedef = 22,
    DW_TAG_union_type = 23,
    DW_TAG_unspecified_parameters = 24,
    DW_TAG_variant = 25,
    DW_TAG_common_block = 26,
    DW_TAG_common_inclusion = 27,
    DW_TAG_inheritance = 28,
    DW_TAG_inlined_subroutine = 29,
    DW_TAG_module = 30,
    DW_TAG_ptr_to_member_type = 31,
    DW_TAG_set_type = 32,
    DW_TAG_subrange_type = 33,
    DW_TAG_with_stmt = 34,
    DW_TAG_access_declaration = 35,
    DW_TAG_base_type = 36,
    DW_TAG_catch_block = 37,
    DW_TAG_const_type = 38,
    DW_TAG_constant = 39,
    DW_TAG_enumerator = 40,
    DW_TAG_file_type = 41,
    DW_TAG_friend = 42,
    DW_TAG_namelist = 43,
    DW_TAG_namelist_item = 44,
    DW_TAG_packed_type = 45,
    DW_TAG_subprogram = 46,
    DW_TAG_template_type_parameter = 47,
    DW_TAG_template_value_parameter = 48,
    DW_TAG_thrown_type = 49,
    DW_TAG_try_block = 50,
    DW_TAG_variant_part = 51,
    DW_TAG_variable = 52,
    DW_TAG_volatile_type = 53,
    DW_TAG_dwarf_procedure = 54,
    DW_TAG_restrict_type = 55,
    DW_TAG_interface_type = 56,
    DW_TAG_namespace = 57,
    DW_TAG_imported_module = 58,
    DW_TAG_unspecified_type = 59,
    DW_TAG_partial_unit = 60,
    DW_TAG_imported_unit = 61,
    DW_TAG_condition = 63,
    DW_TAG_shared_type = 64,
    DW_TAG_type_unit = 65,
    DW_TAG_rvalue_reference_type = 66,
    DW_TAG_template_alias = 67,
    DW_TAG_coarray_type =  68,
    DW_TAG_generic_subrange =  69,
    DW_TAG_dynamic_type =  70,
    DW_TAG_atomic_type =  71,
    DW_TAG_call_site =  72,
    DW_TAG_call_site_parameter =  73,
    DW_TAG_skeleton_unit =  74,
    DW_TAG_immutable_type =  75,
    DW_TAG_lo_user = 1032,
    DW_TAG_hi_user = 15,
};

enum dwarf_children_e {
    DW_CHILDREN_no = 0,
    DW_CHILDREN_yes = 0,
};

enum dwarf_attribute_e {
    DW_AT_sibling = 1,
    DW_AT_location = 2,
    DW_AT_name = 3,
    DW_AT_ordering = 9,
    DW_AT_byte_size = 11,
    DW_AT_bit_size = 13,
    DW_AT_stmt_list = 16,
    DW_AT_low_pc = 17,
    DW_AT_high_pc = 18,
    DW_AT_language = 19,
    DW_AT_discr = 21,
    DW_AT_discr_value = 22,
    DW_AT_visibility = 23,
    DW_AT_import = 24,
    DW_AT_string_length = 25,
    DW_AT_common_reference = 26,
    DW_AT_comp_dir = 27,
    DW_AT_const_value = 28,
    DW_AT_containing_type = 29,
    DW_AT_default_value = 30,
    DW_AT_inline = 32,
    DW_AT_is_optional = 33,
    DW_AT_lower_bound = 34,
    DW_AT_producer = 37,
    DW_AT_prototyped = 39,
    DW_AT_return_addr = 42,
    DW_AT_start_scope = 44,
    DW_AT_bit_stride = 46,
    DW_AT_upper_bound = 47,
    DW_AT_abstract_origin = 49,
    DW_AT_accessibility = 50,
    DW_AT_address_class = 51,
    DW_AT_artificial = 52,
    DW_AT_base_types = 53,
    DW_AT_calling_convention = 54,
    DW_AT_count = 55,
    DW_AT_data_member_location = 56,
    DW_AT_decl_column = 57,
    DW_AT_decl_file = 58,
    DW_AT_decl_line = 59,
    DW_AT_declaration = 60,
    DW_AT_discr_list = 61,
    DW_AT_encoding = 62,
    DW_AT_external = 63,
    DW_AT_frame_base = 64,
    DW_AT_friend = 65,
    DW_AT_identifier_case = 66,
    DW_AT_namelist_item = 68,
    DW_AT_priority = 69,
    DW_AT_segment = 70,
    DW_AT_specification = 71,
    DW_AT_static_link = 72,
    DW_AT_type = 73,
    DW_AT_use_location = 74,
    DW_AT_variable_parameter = 75,
    DW_AT_virtuality = 76,
    DW_AT_vtable_elem_location = 77,
    DW_AT_allocated = 78,
    DW_AT_associated = 79,
    DW_AT_data_location = 80,
    DW_AT_byte_stride = 81,
    DW_AT_entry_pc = 82,
    DW_AT_use_UTF8 = 83,
    DW_AT_extension = 84,
    DW_AT_ranges = 85,
    DW_AT_trampoline = 86,
    DW_AT_call_column = 87,
    DW_AT_call_file = 88,
    DW_AT_call_line = 89,
    DW_AT_description = 90,
    DW_AT_binary_scale = 91,
    DW_AT_decimal_scale = 92,
    DW_AT_small = 93,
    DW_AT_decimal_sign = 94,
    DW_AT_digit_count = 95,
    DW_AT_picture_string = 96,
    DW_AT_mutable = 97,
    DW_AT_threads_scaled = 98,
    DW_AT_explicit = 99,
    DW_AT_object_pointer = 100,
    DW_AT_endianity = 101,
    DW_AT_elemental = 102,
    DW_AT_pure = 103,
    DW_AT_recursive = 104,
    DW_AT_signature = 105,
    DW_AT_main_subprogram = 106,
    DW_AT_data_bit_offset = 107,
    DW_AT_const_expr = 108,
    DW_AT_enum_class = 109,
    DW_AT_linkage_name = 110,
    DW_AT_string_length_bit_size = 111,
    DW_AT_string_length_byte_size = 112,
    DW_AT_rank = 113,
    DW_AT_str_offsets_base = 114,
    DW_AT_addr_base = 115,
    DW_AT_rnglists_base = 116,
    DW_AT_dwo_name = 118,
    DW_AT_reference = 119,
    DW_AT_rvalue_reference = 120,
    DW_AT_macros = 121,
    DW_AT_call_all_calls = 122,
    DW_AT_call_all_source_calls = 123,
    DW_AT_call_all_tail_calls = 124,
    DW_AT_call_return_pc = 125,
    DW_AT_call_value = 126,
    DW_AT_call_origin = 127,
    DW_AT_call_parameter = 128,
    DW_AT_call_pc = 129,
    DW_AT_call_tail_call = 130,
    DW_AT_call_target = 131,
    DW_AT_call_target_clobbered = 132,
    DW_AT_call_data_location = 133,
    DW_AT_call_data_value = 134,
    DW_AT_noreturn = 135,
    DW_AT_alignment = 136,
    DW_AT_export_symbols = 137,
    DW_AT_deleted = 138,
    DW_AT_defaulted = 139,
    DW_AT_loclists_base = 140,
    DW_AT_lo_user = 8192,
    DW_AT_hi_user = 63,
};

enum dwarf_form_e {
    DW_FORM_addr = 1,
    DW_FORM_block2 = 3,
    DW_FORM_block4 = 4,
    DW_FORM_data2 = 5,
    DW_FORM_data4 = 6,
    DW_FORM_data8 = 7,
    DW_FORM_string = 8,
    DW_FORM_block = 9,
    DW_FORM_block1 = 10,
    DW_FORM_data1 = 11,
    DW_FORM_flag = 12,
    DW_FORM_sdata = 13,
    DW_FORM_strp = 14,
    DW_FORM_udata = 15,
    DW_FORM_ref_addr = 16,
    DW_FORM_ref1 = 17,
    DW_FORM_ref2 = 18,
    DW_FORM_ref4 = 19,
    DW_FORM_ref8 = 20,
    DW_FORM_ref_udata = 21,
    DW_FORM_indirect = 22,
    DW_FORM_sec_offset = 23,
    DW_FORM_exprloc = 24,
    DW_FORM_flag_present = 25,
    DW_FORM_strx =  26,
    DW_FORM_addrx =  27,
    DW_FORM_ref_sup4 =  28,
    DW_FORM_strp_sup =  29,
    DW_FORM_data16 =  30,
    DW_FORM_line_strp =  31,
    DW_FORM_ref_sig8 = 32,
    DW_FORM_implicit_const =  33,
    DW_FORM_loclistx =  34,
    DW_FORM_rnglistx =  35,
    DW_FORM_ref_sup8 =  36,
    DW_FORM_strx1 =  37,
    DW_FORM_strx2 =  38,
    DW_FORM_strx3 =  39,
    DW_FORM_strx4 =  40,
    DW_FORM_addrx1 =  41,
    DW_FORM_addrx2 =  42,
    DW_FORM_addrx3 =  43,
    DW_FORM_addrx4 =  44,
};

enum dwarf_operation_e {
    DW_OP_addr = 3,
    DW_OP_deref = 6,
    DW_OP_const1u = 8,
    DW_OP_const1s = 9,
    DW_OP_const2u = 10,
    DW_OP_const2s = 11,
    DW_OP_const4u = 12,
    DW_OP_const4s = 13,
    DW_OP_const8u = 14,
    DW_OP_const8s = 15,
    DW_OP_constu = 16,
    DW_OP_consts = 17,
    DW_OP_dup = 18,
    DW_OP_drop = 19,
    DW_OP_over = 20,
    DW_OP_pick = 21,
    DW_OP_swap = 22,
    DW_OP_plus = 0x22,
    DW_OP_reg0 = 0x50,
    DW_OP_breg0 = 0x70,
    DW_OP_regx = 0x90,
    DW_OP_fbreg = 0x91,
    DW_OP_bregx = 0x92,
    DW_OP_entry_value =  163,
    DW_OP_const_type =  164,
    DW_OP_regval_type =  165,
    DW_OP_deref_type =  166,
    DW_OP_xderef_type =  167,
    DW_OP_convert =  168,
    DW_OP_reinterpret =  169,
    DW_OP_lo_user = 224,
    DW_OP_hi_user = 15,
};

enum dwarf_operation_cfa_e {
    DW_OP_CFA_advance_loc = 0x1,
    DW_OP_CFA_offset = 0x2,
    DW_OP_CFA_restore = 0x3,
    DW_OP_CFA_nop = 0x0,
    DW_OP_CFA_set_loc = 0x01,
    DW_OP_CFA_advance_loc1 = 0x02,
    DW_OP_CFA_advance_loc2 = 0x03,
    DW_OP_CFA_advance_loc4 = 0x04,
    DW_OP_CFA_offset_extended = 0x05,
    DW_OP_CFA_restore_extended = 0x06,
    DW_OP_CFA_undefined = 0x07,
    DW_OP_CFA_same_value = 0x08,
    DW_OP_CFA_register = 0x09,
    DW_OP_CFA_remember_state = 0x0a,
    DW_OP_CFA_restore_state = 0x0b,
    DW_OP_CFA_def_cfa = 0x0c,
    DW_OP_CFA_def_cfa_register = 0x0d,
    DW_OP_CFA_def_cfa_offset = 0x0e,
    DW_OP_CFA_def_cfa_expression = 0x0f,
    DW_OP_CFA_expression = 0x10,
    DW_OP_CFA_offset_extended_sf = 0x11,
    DW_OP_CFA_def_cfa_sf = 0x12,
    DW_OP_CFA_def_cfa_offset_sf = 0x13,
    DW_OP_CFA_val_offset = 0x14,
    DW_OP_CFA_val_offset_sf = 0x15,
    DW_OP_CFA_val_expression = 0x16,
    DW_OP_CFA_lo_user = 0x09,
    DW_OP_CFA_hi_user = 0x09,
};

enum dwarf_attribute_encoding_e {
    DW_ATE_address = 1,
    DW_ATE_boolean = 2,
    DW_ATE_complex_float = 3,
    DW_ATE_float = 4,
    DW_ATE_signed = 5,
    DW_ATE_signed_char = 6,
    DW_ATE_unsigned = 7,
    DW_ATE_unsigned_char = 8,
    DW_ATE_imaginary_float = 9,
    DW_ATE_packed_decimal = 10,
    DW_ATE_numeric_string = 11,
    DW_ATE_edited = 12,
    DW_ATE_signed_fixed = 13,
    DW_ATE_unsigned_fixed = 14,
    DW_ATE_decimal_float = 15,
    DW_ATE_UTF = 16,
    DW_ATE_UCS = 17,
    DW_ATE_ASCII = 18,
    DW_ATE_lo_user = 128,
    DW_ATE_hi_user = 15,
};

enum dwarf_lns_e {
    DW_LNS_copy = 1,
    DW_LNS_advance_pc = 2,
    DW_LNS_advance_line = 3,
    DW_LNS_set_file = 4,
    DW_LNS_set_column = 5,
    DW_LNS_negate_stmt = 6,
    DW_LNS_set_basic_block = 7,
    DW_LNS_const_add_pc = 8,
    DW_LNS_fixed_advance_pc = 9,
    DW_LNS_set_prologue_end = 10,
    DW_LNS_set_epilogue_begin = 11,
    DW_LNS_set_isa = 12,
};

enum dwarf_lne_e {
    DW_LNE_end_sequence = 1,
    DW_LNE_set_address = 2,
    DW_LNE_set_discriminator = 4,
    DW_LNE_lo_user = 128,
    DW_LNE_hi_user = 15,
};

enum dwarf_lnct_e {
    DW_LNCT_path = 1,
    DW_LNCT_directory_index = 2,
    DW_LNCT_timestamp = 3,
    DW_LNCT_size = 4,
    DW_LNCT_MD5 = 5,
    DW_LNCT_lo_user = 8,
    DW_LNCT_hi_user = 6,
};

enum dwarf_eh_pe_e {
    DW_EH_PE_absptr = 0x00,
    DW_EH_PE_uleb128 = 0x01,
    DW_EH_PE_udata2 = 0x02,
    DW_EH_PE_udata4 = 0x03,
    DW_EH_PE_udata8 = 0x04,
    DW_EH_PE_sleb128 = 0x09,
    DW_EH_PE_sdata2 = 0x0A,
    DW_EH_PE_sdata4 = 0x0B,
    DW_EH_PE_sdata8 = 0x0C,

    DW_EH_PE_pcrel = 0x10,
    DW_EH_PE_textrel = 0x20,
    DW_EH_PE_datarel = 0x30,
    DW_EH_PE_funcrel = 0x40,
    DW_EH_PE_aligned = 0x50,
};

enum dwarf_x86_register_e {
    DW_X86_REG_EAX = 0,
    DW_X86_REG_ECX = 1,
    DW_X86_REG_EDX = 2,
    DW_X86_REG_EBX = 3,
    DW_X86_REG_ESI = 4,
    DW_X86_REG_EDI = 5,
    DW_X86_REG_EBP = 6,
    DW_X86_REG_ESP = 7,
    DW_X86_REG_RA = 8,
};

enum dwarf_x64_register_e {
    DW_X64_REG_RAX = 0,
    DW_X64_REG_RDX = 1,
    DW_X64_REG_RCX = 2,
    DW_X64_REG_RBX = 3,
    DW_X64_REG_RSI = 4,
    DW_X64_REG_RDI = 5,
    DW_X64_REG_RBP = 6,
    DW_X64_REG_RSP = 7,
    DW_X64_REG_R8 = 8,
    DW_X64_REG_R9 = 9,
    DW_X64_REG_R10 = 10,
    DW_X64_REG_R11 = 11,
    DW_X64_REG_R12 = 12,
    DW_X64_REG_R13 = 13,
    DW_X64_REG_R14 = 14,
    DW_X64_REG_R15 = 15,
    DW_X64_REG_RA = 16,
};

typedef struct sysbvm_dwarf_cie_s {
    uint8_t pointerSize;
    uintptr_t codeAlignmentFactor;
    intptr_t dataAlignmentFactor;
    uintptr_t returnAddressRegister;
} sysbvm_dwarf_cie_t;

typedef struct sysbvm_dwarf_cfi_builder_s {
    sysbvm_dynarray_t buffer;

    uint8_t version;
    bool isEhFrame;

    size_t cieOffset;
    size_t cieContentOffset;
    size_t initialStackFrameSize;
    sysbvm_dwarf_cie_t cie;

    size_t fdeOffset;
    size_t fdeContentOffset;
    size_t fdeInitialPC;
    size_t fdeInitialLocationOffset;
    size_t fdeAddressingRangeOffset;

    size_t currentPC;
    size_t stackFrameSize;
    size_t stackPointerRegister;
    size_t framePointerRegister;
    size_t stackFrameSizeAtFramePointer;
    bool hasFramePointerRegister;

    bool isInPrologue;
} sysbvm_dwarf_cfi_builder_t;

typedef struct sysbvm_dwarf_debugInfo_builder_s {
    int version;
    int abbreviationCount;

    sysbvm_dynarray_t line;
    sysbvm_dynarray_t str;
    sysbvm_dynarray_t abbrev;
    sysbvm_dynarray_t info;

    sysbvm_dynarray_t lineTextAddresses;
    sysbvm_dynarray_t infoTextAddresses;
} sysbvm_dwarf_debugInfo_builder_t;

SYSBVM_API size_t sysbvm_dwarf_encodeByte(sysbvm_dynarray_t *buffer, uint8_t value);
SYSBVM_API size_t sysbvm_dwarf_encodeWord(sysbvm_dynarray_t *buffer, uint16_t value);
SYSBVM_API size_t sysbvm_dwarf_encodeDWord(sysbvm_dynarray_t *buffer, uint32_t value);
SYSBVM_API size_t sysbvm_dwarf_encodeDwarfPointer(sysbvm_dynarray_t *buffer, uint32_t value);
SYSBVM_API size_t sysbvm_dwarf_encodeDwarfPointerPCRelative(sysbvm_dynarray_t *buffer, uint32_t value);
SYSBVM_API size_t sysbvm_dwarf_encodePointer(sysbvm_dynarray_t *buffer, uintptr_t value);
SYSBVM_API size_t sysbvm_dwarf_encodeCString(sysbvm_dynarray_t *buffer, const char *cstring);
SYSBVM_API size_t sysbvm_dwarf_encodeULEB128(sysbvm_dynarray_t *buffer, uintptr_t value);
SYSBVM_API size_t sysbvm_dwarf_encodeSLEB128(sysbvm_dynarray_t *buffer, intptr_t value);
SYSBVM_API size_t sysbvm_dwarf_encodeAlignment(sysbvm_dynarray_t *buffer, size_t alignment);

SYSBVM_API void sysbvm_dwarf_cfi_create(sysbvm_dwarf_cfi_builder_t *cfi);
SYSBVM_API void sysbvm_dwarf_cfi_destroy(sysbvm_dwarf_cfi_builder_t *cfi);

SYSBVM_API void sysbvm_dwarf_cfi_beginCIE(sysbvm_dwarf_cfi_builder_t *cfi, sysbvm_dwarf_cie_t *cie);
SYSBVM_API void sysbvm_dwarf_cfi_endCIE(sysbvm_dwarf_cfi_builder_t *cfi);

SYSBVM_API void sysbvm_dwarf_cfi_beginFDE(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc);
SYSBVM_API void sysbvm_dwarf_cfi_endFDE(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc);
SYSBVM_API void sysbvm_dwarf_cfi_finish(sysbvm_dwarf_cfi_builder_t *cfi);

SYSBVM_API void sysbvm_dwarf_cfi_setPC(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc);
SYSBVM_API void sysbvm_dwarf_cfi_cfaInRegisterWithOffset(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg, intptr_t offset);
SYSBVM_API void sysbvm_dwarf_cfi_cfaInRegisterWithFactoredOffset(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg, size_t offset);
SYSBVM_API void sysbvm_dwarf_cfi_registerValueAtFactoredOffset(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg, size_t offset);
SYSBVM_API void sysbvm_dwarf_cfi_pushRegister(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg);
SYSBVM_API void sysbvm_dwarf_cfi_saveFramePointerInRegister(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg, intptr_t offset);
SYSBVM_API void sysbvm_dwarf_cfi_stackSizeAdvance(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc, size_t increment);
SYSBVM_API void sysbvm_dwarf_cfi_endPrologue(sysbvm_dwarf_cfi_builder_t *cfi);

SYSBVM_API void sysbvm_dwarf_debugInfo_create(sysbvm_dwarf_debugInfo_builder_t *builder);
SYSBVM_API void sysbvm_dwarf_debugInfo_finish(sysbvm_dwarf_debugInfo_builder_t *builder);
SYSBVM_API void sysbvm_dwarf_debugInfo_destroy(sysbvm_dwarf_debugInfo_builder_t *builder);
SYSBVM_API void sysbvm_dwarf_debugInfo_patchTextAddressesRelativeTo(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t baseAddress);

SYSBVM_API void sysbvm_dwarf_debugInfo_beginDIE(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t tag, bool hasChildren);
SYSBVM_API void sysbvm_dwarf_debugInfo_endDIE(sysbvm_dwarf_debugInfo_builder_t *builder);
SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_string(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute, const char *value);
SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_textAddress(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute, uintptr_t value);

#endif //SYSBVM_DWARF_H