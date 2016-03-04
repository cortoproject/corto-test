/* $CORTO_GENERATED
 *
 * Runner.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include "corto/test/test.h"

/* $header() */
void test_updateProgress(test_Runner this) {
    int i;
    char *str;
    if (test_CaseListSize(this->failures)) {
        corto_asprintf(&str, "%s: PASS:%d, %sFAIL%s:%d",
            this->name,
            test_CaseListSize(this->successes),
            CORTO_RED,
            CORTO_NORMAL,
            test_CaseListSize(this->failures));
    } else {
        corto_asprintf(&str, "%s: %sPASS%s:%d, FAIL:%d",
            this->name,
            CORTO_GREEN,
            CORTO_NORMAL,
            test_CaseListSize(this->successes),
            test_CaseListSize(this->failures));
    }

    for (i = 0; i < strlen(str) + 2 * 2; i++) {
        printf("\b");
    }
    printf("%s", str);
    fflush(stdout);
}
/* $end */

corto_int16 _test_Runner_construct(
    test_Runner this)
{
/* $begin(corto/test/Runner/construct) */
    /* If a testcase is provided, run it. Otherwise, discover testcases and
     * forward to separate process. */
    if (this->testcase) {
        corto_object testcase = corto_resolve(NULL, this->testcase);
        if (testcase) {
            corto_type testClass = corto_parentof(testcase);
            test_SuiteData suite = test_SuiteData(corto_create(testClass));

            if (test_SuiteData_run(suite, testcase)) {
                corto_error("test: failed to define test suite");
                test_CaseListAppend(this->failures, testcase);
            } else {
                corto_object prev = corto_setOwner(this);
                corto_define(suite);
                corto_setOwner(prev);
            }

            corto_delete(suite);
        } else {
            corto_error("test: testcase '%s' not found", this->testcase);
            goto error;
        }
    } else {
        corto_listen(this, test_Runner_runTest_o, CORTO_ON_DEFINE | CORTO_ON_TREE, root_o, NULL);
    }
    return 0;
error:
    return -1;
/* $end */
}

corto_void _test_Runner_destruct(
    test_Runner this)
{
/* $begin(corto/test/Runner/destruct) */
    if (!this->testcase) {
        test_updateProgress(this);
        printf("\n");
        corto_silence(this, test_Runner_runTest_o, CORTO_ON_DEFINE | CORTO_ON_TREE, root_o);
    }
/* $end */
}

corto_void _test_Runner_runTest(
    test_Runner this,
    corto_object observable)
{
/* $begin(corto/test/Runner/runTest) */

    if (corto_instanceof(corto_type(test_Case_o), observable)) {
        corto_id testcaseId;
        corto_int8 err, ret;

        corto_pid pid = corto_procrun(
            "corto",
            (char*[]){
                "corto",
                "--mute",
                this->lib,
                corto_fullpath(testcaseId, observable),
                NULL
            }
        );

        if ((err = corto_procwait(pid, &ret)) || ret) {
            if (err > 0) {
                int i;
                for (i = 0; i < 255; i++) {
                    fprintf(stderr, "\b");
                }
                fprintf(stderr, "%sFAIL%s: %s: test crashed with signal %d\n",
                    CORTO_RED,
                    CORTO_NORMAL,
                    testcaseId, err);
            } else {
                /* Process exited with a returncode != 0 so
                 * must've printed an error msg itself */
            }
            test_CaseListAppend(this->failures, observable);
        } else {
            test_CaseListAppend(this->successes, observable);
        }
        this->testsRun++;

        /* Don't print statistics when in CI mode */
        corto_string ciEnv = corto_getenv("CI");
        if (!ciEnv || strcmp(ciEnv, "true")) {
            if (!(this->testsRun % 4)) {
                test_updateProgress(this);
            }
        } else {
            /* When in CI mode, show each individual testcase */
            printf("%s%s%s: %s\n",
                err ? CORTO_RED : CORTO_GREEN,
                err ? "FAIL" : "PASS",
                CORTO_NORMAL,
                testcaseId);
        }
    }
/* $end */
}
