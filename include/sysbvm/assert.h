#ifndef SYSBVM_ASSERT_H
#define SYSBVM_ASSERT_H

#pragma once

#include "errors.h"

#define SYSBVM_ASSERT_LINE_TO_STRING__(x) #x
#define SYSBVM_ASSERT_LINE_TO_STRING_(x) SYSBVM_ASSERT_LINE_TO_STRING__(x)
#define SYSBVM_ASSERT_LINE_TO_STRING(x) SYSBVM_ASSERT_LINE_TO_STRING_(x)
#define SYSBVM_ASSERT(x) if(!(x)) sysbvm_error_assertionFailure(__FILE__ ":" SYSBVM_ASSERT_LINE_TO_STRING_(__LINE__)": assertion failure: " #x)

#ifdef NDEBUG
#   define SYSBVM_DASSERT(x) while(false)
#else
#   define SYSBVM_DASSERT(x) if(!(x)) sysbvm_error_fatalAssertionFailure(__FILE__ ":" SYSBVM_ASSERT_LINE_TO_STRING_(__LINE__)": assertion failure: " #x)
#endif

#endif //SYSBVM_ASSERT_H