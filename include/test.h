/* test.h
 * This is the main package file. Include this file in other projects.
 * Only modify inside the header-end and body-end sections.
 */

#ifndef CORTO_TEST_H
#define CORTO_TEST_H

#include "bake_config.h"

#define CORTO_TEST_ETC ut_locate("corto.test", NULL, UT_LOCATE_ETC)

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

#include "_type.h"
#include "_interface.h"
#include "_load.h"
#include "_binding.h"

#include <corto.test.c>

/* $body() */
/* Enter code that requires types here */
/* $end */

#endif

