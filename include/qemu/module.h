/*
 * QEMU Module Infrastructure
 *
 * Copyright IBM, Corp. 2009
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */

#ifndef QEMU_MODULE_H
#define QEMU_MODULE_H

#include "trace/recorder.h"

#define DSO_STAMP_FUN         glue(qemu_stamp, CONFIG_STAMP)
#define DSO_STAMP_FUN_STR     stringify(DSO_STAMP_FUN)

RECORDER_DECLARE(modules);

#ifdef BUILD_DSO
void DSO_STAMP_FUN(void);
/* This is a dummy symbol to identify a loaded DSO as a QEMU module, so we can
 * distinguish "version mismatch" from "not a QEMU module", when the stamp
 * check fails during module loading */
void qemu_module_dummy(void);

#define module_init(function, type)                                         \
static void __attribute__((constructor)) do_qemu_init_ ## function(void)    \
{                                                                           \
    register_dso_module_init(function, type);                               \
}
#else
/* This should not be used directly.  Use block_init etc. instead.  */
#define module_init(function, type)                                         \
static void __attribute__((constructor)) do_qemu_init_ ## function(void)    \
{                                                                           \
    register_module_init(function, type);                                   \
}
#endif

#ifdef CONFIG_MODULES
/* Identify which functions are replaced by a callback stub */
#ifdef MODULE_STUBS
#define MODIFACE(Ret,Name,Args)                                         \
    Ret (*Name)Args;                                                    \
    extern Ret Name##_implementation Args
#else /* !MODULE_STUBS */
#define MODIFACE(Ret,Name,Args)                                         \
    extern Ret (*Name)Args;                                             \
    extern Ret Name##_implementation Args
#endif /* MODULE_STUBS */

#define MODIMPL(Ret,Name,Args)                                          \
    static void __attribute__((constructor)) Name##_register(void)      \
    {                                                                   \
        Name = Name##_implementation;                                   \
        record(modules, "Setting " #Name " to %p", Name);               \
    }                                                                   \
    Ret Name##_implementation Args
#else /* !CONFIG_MODULES */
/* When not using a module, such functions are called directly */
#define MODIFACE(Ret,Name,Args)         Ret Name Args
#define MODIMPL(Ret,Name,Args)          Ret Name Args
#endif /* CONFIG_MODULES */

typedef enum {
    MODULE_INIT_MIGRATION,
    MODULE_INIT_BLOCK,
    MODULE_INIT_OPTS,
    MODULE_INIT_QOM,
    MODULE_INIT_TRACE,
    MODULE_INIT_XEN_BACKEND,
    MODULE_INIT_LIBQOS,
    MODULE_INIT_FUZZ_TARGET,
    MODULE_INIT_MAX
} module_init_type;

#define block_init(function) module_init(function, MODULE_INIT_BLOCK)
#define opts_init(function) module_init(function, MODULE_INIT_OPTS)
#define type_init(function) module_init(function, MODULE_INIT_QOM)
#define trace_init(function) module_init(function, MODULE_INIT_TRACE)
#define xen_backend_init(function) module_init(function, \
                                               MODULE_INIT_XEN_BACKEND)
#define libqos_init(function) module_init(function, MODULE_INIT_LIBQOS)
#define fuzz_target_init(function) module_init(function, \
                                               MODULE_INIT_FUZZ_TARGET)
#define migration_init(function) module_init(function, MODULE_INIT_MIGRATION)
#define block_module_load_one(lib) module_load_one("block-", lib)
#define ui_module_load_one(lib) module_load_one("ui-", lib)
#define audio_module_load_one(lib) module_load_one("audio-", lib)

void register_module_init(void (*fn)(void), module_init_type type);
void register_dso_module_init(void (*fn)(void), module_init_type type);

void module_call_init(module_init_type type);
bool module_load_one(const char *prefix, const char *lib_name);

#endif
