#ifndef TUUVM_ASSERT_H
#define TUUVM_ASSERT_H

#include "errors.h"

#define TUUVM_ASSERT_LINE_TO_STRING_(x) #x
#define TUUVM_ASSERT_LINE_TO_STRING(x) TUUVM_ASSERT_LINE_TO_STRING_(x)
#define TUUVM_ASSERT(x) if(!(x)) tuuvm_error_assertionFailure(__FILE__ ":" TUUVM_ASSERT_LINE_TO_STRING_(__LINE__)": assertion failure: " #x)

#endif //TUUVM_ASSERT_H