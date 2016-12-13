/* test.h
 *
 * This file contains generated code. Do not modify!
 */

#ifndef CORTO_TEST_H
#define CORTO_TEST_H

#include <corto/corto.h>
#include <corto/test/_project.h>
/* $header() */
#ifdef __cplusplus
extern "C" {
#endif

void test_erase(void);
corto_string test_id(corto_id buffer, corto_object testcase);
corto_string test_command(corto_id buffer, corto_string lib, corto_object testcase);

#ifdef __cplusplus
}
#endif
/* $end */


#include <corto/test/_type.h>
#include <corto/test/_api.h>
#include <corto/test/_load.h>

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

CORTO_TEST_EXPORT corto_bool _test_assertflt(
    corto_float64 f1,
    corto_float64 f2,
    corto_string str_f1,
    corto_string str_f2,
    corto_uint32 __line);
#define test_assertflt(f1, f2) _test_assertflt(f1, f2, #f1, #f2, __LINE__)

CORTO_TEST_EXPORT corto_bool _test_assertint(
    corto_uint64 i1,
    corto_uint64 i2,
    corto_string str_i1,
    corto_string str_i2,
    corto_uint32 __line);
#define test_assertint(i1, i2) _test_assertint(i1, i2, #i1, #i2, __LINE__)

CORTO_TEST_EXPORT corto_bool _test_assertstr(
    corto_string s1,
    corto_string s2,
    corto_string str_s1,
    corto_string str_s2,
    corto_uint32 __line);
#define test_assertstr(s1, s2) _test_assertstr(s1, s2, #s1, #s2, __LINE__)

CORTO_TEST_EXPORT corto_void _test_fail(
    corto_string err);
#define test_fail(err) _test_fail(err)

CORTO_TEST_EXPORT corto_bool _test_runslow(void);
#define test_runslow() _test_runslow()

CORTO_TEST_EXPORT corto_void _test_setTimeout(
    corto_time *t);
#define test_setTimeout(t) _test_setTimeout(t)
#include <corto/test/Case.h>
#include <corto/test/Result.h>
#include <corto/test/Runner.h>
#include <corto/test/Suite.h>
#include <corto/test/SuiteData.h>

#ifdef __cplusplus
}
#endif
#endif

