/* $CORTO_GENERATED
 *
 * Suite.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <corto/test/test.h>

int16_t _test_Suite_construct(
    test_Suite this)
{
/* $begin(corto/test/Suite/construct) */
    corto_ptr_setref(
      &corto_interface(this)->base, corto_interface(test_SuiteData_o));
    return corto_class_construct(this);
/* $end */
}
