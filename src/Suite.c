/* This is a managed file. Do not delete this comment. */

#include <corto.test>
int16_t test_Suite_construct(
    test_Suite this)
{
    corto_set_ref(
      &corto_interface(this)->base, corto_interface(test_SuiteData_o));
    return corto_class_construct(this);
}
