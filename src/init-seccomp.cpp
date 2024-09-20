#include "../include/runner/init-seccomp.h"

scmp_filter_ctx GetFilter() {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(socket), 0);
    return ctx;
}

void InitSeccomp() {
    static scmp_filter_ctx ctx = GetFilter();
    seccomp_load(ctx);
}
