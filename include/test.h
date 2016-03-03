/* test.h
 *
 * This file contains generated code. Do not modify!
 */

#ifndef CORTO_TEST_H
#define CORTO_TEST_H

#include "corto/corto.h"
#include "corto/test/_type.h"
#include "corto/test/_api.h"
#include "corto/test/_meta.h"
#include "corto/test/_interface.h"

#ifdef __cplusplus
extern "C" {
#endif


CORTO_TEST_EXPORT corto_bool _test_assert(
    corto_bool condition,
    corto_string str_condition,
    corto_uint32 __line);
#define test_assert(condition) _test_assert(condition, #condition, __LINE__)

CORTO_TEST_EXPORT corto_bool _test_assertEqual(
    corto_any a,
    corto_any b,
    corto_string str_a,
    corto_string str_b,
    corto_uint32 __line);
#define test_assertEqual(a, b) _test_assertEqual(a, b, #a, #b, __LINE__)

CORTO_TEST_EXPORT corto_void _test_fail(
    corto_string err);
#define test_fail(err) _test_fail(err)
#include "corto/test/Case.h"
#include "corto/test/Result.h"
#include "corto/test/Runner.h"
#include "corto/test/Suite.h"
#include "corto/test/SuiteData.h"

#ifdef __cplusplus
}
#endif
#endif

