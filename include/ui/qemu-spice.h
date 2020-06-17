/*
 * Copyright (C) 2010 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 or
 * (at your option) version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QEMU_SPICE_H
#define QEMU_SPICE_H

#include "qapi/error.h"

#ifdef CONFIG_SPICE

#include <spice.h>
#include "qemu/config-file.h"

#define using_spice     (qemu_is_using_spice())

#if !defined(CONFIG_MODULES) || defined(BUILD_DSO)
/* Use actual functions */
bool qemu_is_using_spice(void);
void qemu_start_using_spice(void);
void qemu_spice_init(void);
void qemu_spice_display_init(void);
int qemu_spice_display_add_client(int csock, int skipauth, int tls);
int qemu_spice_set_passwd(const char *passwd,
                          bool fail_if_connected, bool disconnect_if_connected);
int qemu_spice_set_pw_expire(time_t expires);
int qemu_spice_migrate_info(const char *hostname, int port, int tls_port,
                            const char *subject);
struct SpiceInfo *qemu_spice_query(Error **errp);
#endif /* !CONFIG_MODULES || BUILD_DSO */

#ifdef CONFIG_MODULES
struct QemuSpiceOps
{
    bool (*qemu_is_using_spice)(void);
    void (*qemu_start_using_spice)(void);
    void (*qemu_spice_init)(void);
    void (*qemu_spice_display_init)(void);
    int (*qemu_spice_display_add_client)(int csock, int skipauth, int tls);

    int (*qemu_spice_set_passwd)(const char *passwd,
                                 bool fail_if_connected,
                                 bool disconnect_if_connected);
    int (*qemu_spice_set_pw_expire)(time_t expires);
    int (*qemu_spice_migrate_info)(const char *hostname,
                                   int port, int tls_port,
                                   const char *subject);
    struct SpiceInfo * (*qemu_spice_query)(Error **errp);
};
typedef struct QemuSpiceOps QemuSpiceOps;
void qemu_spice_ops_register(QemuSpiceOps *ops);

#ifndef BUILD_DSO
/* Inline wrappers for functions implemented in the module */
extern struct QemuSpiceOps qemu_spice;

static inline bool qemu_is_using_spice(void)
{
    return qemu_spice.qemu_is_using_spice();
}

static inline void qemu_start_using_spice(void)
{
    qemu_spice.qemu_start_using_spice();
}

static inline void qemu_spice_init(void)
{
    qemu_spice.qemu_spice_init();
}

static inline void qemu_spice_display_init(void)
{
    qemu_spice.qemu_spice_display_init();
}

static inline int qemu_spice_display_add_client(int csock, int skipauth, int tls)
{
    return qemu_spice.qemu_spice_display_add_client(csock, skipauth, tls);
}

static inline int qemu_spice_set_passwd(const char *passwd,
                                        bool fail_if_connected,
                                        bool disconnect_if_connected)
{
    return qemu_spice.qemu_spice_set_passwd(passwd,
                                            fail_if_connected,
                                            disconnect_if_connected);
}

static inline int qemu_spice_set_pw_expire(time_t expires)
{
    return qemu_spice.qemu_spice_set_pw_expire(expires);
}

static inline int qemu_spice_migrate_info(const char *hostname,
                                          int port, int tls_port,
                                          const char *subject)
{
    return qemu_spice.qemu_spice_migrate_info(hostname, port, tls_port, subject);
}

static inline struct SpiceInfo *qemu_spice_query(Error **errp)
{
    return qemu_spice.qemu_spice_query(errp);
}

#endif /* BUILD_DSO */
#endif /* CONFIG_MODULES */

void qemu_spice_input_init(void);
void qemu_spice_audio_init(void);
int qemu_spice_add_interface(SpiceBaseInstance *sin);
bool qemu_spice_have_display_interface(QemuConsole *con);
int qemu_spice_add_display_interface(QXLInstance *qxlin, QemuConsole *con);

#if !defined(SPICE_SERVER_VERSION) || (SPICE_SERVER_VERSION < 0xc06)
#define SPICE_NEEDS_SET_MM_TIME 1
#else
#define SPICE_NEEDS_SET_MM_TIME 0
#endif
void qemu_spice_register_ports(void);

#else  /* CONFIG_SPICE */

#include "qemu/error-report.h"

static inline bool qemu_is_using_spice(void)
{
    return 0;
}

#define spice_displays 0
static inline int qemu_spice_set_passwd(const char *passwd,
                                        bool fail_if_connected,
                                        bool disconnect_if_connected)
{
    return -1;
}
static inline int qemu_spice_set_pw_expire(time_t expires)
{
    return -1;
}
static inline int qemu_spice_migrate_info(const char *h, int p, int t,
                                          const char *s)
{
    return -1;
}

static inline int qemu_spice_display_add_client(int csock, int skipauth,
                                                int tls)
{
    return -1;
}

static inline void qemu_spice_display_init(void)
{
    /* This must never be called if CONFIG_SPICE is disabled */
    error_report("spice support is disabled");
    abort();
}

static inline void qemu_spice_init(void)
{
}

#endif /* CONFIG_SPICE */

static inline bool qemu_using_spice(Error **errp)
{
    if (!using_spice) {
        error_set(errp, ERROR_CLASS_DEVICE_NOT_ACTIVE,
                  "SPICE is not in use");
        return false;
    }
    return true;
}

#endif /* QEMU_SPICE_H */
