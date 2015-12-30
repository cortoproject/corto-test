/* $CORTO_GENERATED
 *
 * Suite.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "corto/test/test.h"

corto_int16 _test_Suite_construct(test_Suite this) {
/* $begin(corto/test/Suite/construct) */
    corto_setref(
      &corto_interface(this)->base, corto_interface(test_SuiteData_o));
    return corto_class_construct(this);
/* $end */
}
