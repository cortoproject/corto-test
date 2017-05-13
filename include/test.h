/* test.h
 *
 * This file contains generated code. Do not modify!
 */

#ifndef CORTO_TEST_H
#define CORTO_TEST_H

#include <corto/corto.h>
#include <corto/corto.h>
#include <corto/test/_project.h>
#include <corto/c/c.h>

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
#include <corto/test/_interface.h>
#include <corto/test/_load.h>
#include <corto/test/c/_api.h>

/* $body() */
/* Enter code that requires types here */
/* $end */

#endif

