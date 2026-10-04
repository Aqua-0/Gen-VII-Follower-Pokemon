#pragma once

#include <3ds/types.h>

#define ALIGN(m) __attribute__((aligned(m)))
#define PACKED __attribute__((packed))
#define USED __attribute__((used))
#define UNUSED __attribute__((unused))
#define DEPRECATED __attribute__((deprecated))
#define NAKED __attribute__((naked))
#define NORETURN __attribute__((noreturn))
