#pragma once

#include <seccomp.h>


void InitSeccomp() {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(socket), 0);

    seccomp_load(ctx);
}