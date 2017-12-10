/* This is a managed file. Do not delete this comment. */

#include <corto/test/test.h>

#define FIND(parent, id) corto(parent, id, NULL, NULL, NULL, NULL, -1, 0)

void test_updateProgress(test_Runner this) {
    if (corto_ll_count(this->failures)) {
        if (corto_ll_count(this->empty)) {
            corto_log("#[]%s: PASS:%d, #[red]FAIL#[normal]:%d, #[yellow]EMPTY#[normal]:%d",
                this->name,
                corto_ll_count(this->successes),
                corto_ll_count(this->failures),
                corto_ll_count(this->empty));
            } else {
                corto_log("#[]%s: PASS:%d, #[red]FAIL#[normal]:%d, EMPTY:%d",
                    this->name,
                    corto_ll_count(this->successes),
                    corto_ll_count(this->failures),
                    corto_ll_count(this->empty));
            }
    } else {
        if (corto_ll_count(this->empty)) {
            corto_log("#[]%s: #[green]PASS#[normal]:%d, FAIL:%d, #[yellow]EMPTY#[normal]:%d",
                this->name,
                corto_ll_count(this->successes),
                corto_ll_count(this->failures),
                corto_ll_count(this->empty));
        } else {
            corto_log("#[]%s: #[green]PASS#[normal]:%d, FAIL:%d, EMPTY:%d",
                this->name,
                corto_ll_count(this->successes),
                corto_ll_count(this->failures),
                corto_ll_count(this->empty));
        }
    }
}

int16_t test_Runner_construct(
    test_Runner this)
{
    /* Use corto_find, to prevent loading a package /test, if it exists */
    corto_object testroot = FIND(root_o, "test");
    if (!testroot) {
        corto_throw("test: /test not found");
        goto error;
    }

    /* If a testcase is provided, run it. Otherwise, discover testcases and
     * forward to separate process. */
    if (this->testcase) {
        corto_object testcase = FIND(testroot, this->testcase);
        if (testcase) {
            /* Set TARGET in case this testcase is called directly */
            char *oldenv = corto_getenv("BAKE_TARGET");
            corto_setenv("BAKE_TARGET", "$HOME/.corto_tmp");

            corto_log_push(strarg("test:%s", this->testcase));
            corto_type testClass = corto_parentof(testcase);
            test_SuiteData suite = test_SuiteData(corto_create(testClass));

            if (corto_getenv("CORTO_TEST_BY_ID") && !stricmp(corto_getenv("CORTO_TEST_BY_ID"), "true")) {
                corto_id cmd;
                corto_log("RUN %s\n",
                    test_command(cmd, this->lib, testcase));
            }

            if (test_SuiteData_run(suite, testcase)) {
                corto_error("test: failed to define test suite");
                corto_claim(testcase);
                corto_ll_append(this->failures, testcase);
            } else {
                corto_object prev = corto_setOwner(this);
                corto_define(suite);
                corto_setOwner(prev);

                corto_ok("DONE  %s", this->testcase);
            }
            corto_log_pop();

            corto_setenv("BAKE_TARGET", oldenv);

            corto_delete(suite);
        } else {
            /* Testcase not found, don't report error */
            corto_trace("test: %s NOT FOUND", this->testcase);
        }
    } else {
        if (corto_observer_observe(test_Runner_runTest_o, this, root_o)) {
            goto error;
        }
    }

    corto_release(testroot);

    return 0;
error:
    return -1;
}

void test_Runner_destruct(
    test_Runner this)
{
    if (!this->testcase) {
        test_updateProgress(this);
        printf("\n");
        corto_observer_unobserve(test_Runner_runTest_o, this, root_o);
    }
}

void test_Runner_runTest(
    corto_observerEvent *e)
{
    test_Runner this = e->instance;
    corto_object object = e->data;

    if (corto_instanceof(corto_type(test_Case_o), object)) {
        corto_id testcaseId;
        corto_int8 err = 0, ret = 0;
        corto_string ciEnv = corto_getenv("CI");

        /* Set TARGET to temporary environment so tests can't contaminate the
         * current environment */
        char *oldenv = corto_getenv("BAKE_TARGET");

        corto_setenv("BAKE_TARGET", "$HOME/.corto_tmp");

        test_id(testcaseId, object);

        {
            corto_id cmd; (void)cmd;
            corto_trace("\ntest: TESTCASE %s", testcaseId);
            corto_trace("test:   EXEC  %s",
                test_command(cmd, this->lib, object));
        }

        char *tool = getenv("CORTO_TEST_TOOL");
        corto_proc pid = 0;

        if (!tool || !strcmp(tool, "")) {
            pid = corto_proc_run(
                "corto",
                (char*[]){
                    "corto",
                    "--mute",
                    "-l",
                    this->lib,
                    testcaseId,
                    NULL
                }
            );
        } else if (!strcmp(tool, "check-memory")) {
            setenv("CORTO_TEST_RUNSLOW", "TRUE", 1);
            if (!strcmp(CORTO_OS_STRING, "linux")) {
                pid = corto_proc_run(
                    "valgrind",
                    (char*[]){
                        "valgrind",
                        "-q",
                        "corto",
                        "-l",
                        this->lib,
                        testcaseId,
                        NULL
                    }
                );
            } else {
                corto_throw("check-memory is only supported on Linux (uses valgrind)");
            }
        } else if (!strcmp(tool, "check-thread")) {
            setenv("CORTO_TEST_RUNSLOW", "TRUE", 1);
            if (!strcmp(CORTO_OS_STRING, "linux")) {
                pid = corto_proc_run(
                    "valgrind",
                    (char*[]){
                        "valgrind",
                        "-q",
                        "--tool=helgrind",
                        "corto",
                        "-l",
                        this->lib,
                        testcaseId,
                        NULL
                    }
                );
            } else {
                corto_throw("check-thread is only supported on Linux (uses valgrind)");
            }
        }

        if (!pid) {
            corto_raise();
            corto_log("#[red]FAIL#[normal]: %s\n",
                testcaseId);
        } else if ((err = corto_proc_wait(pid, &ret)) || ret) {
            if (err > 0) {
                corto_catch();
                test_erase();
                corto_log("#[red]FAIL#[normal]: %s: test crashed with signal %d\n",
                    testcaseId, err);
            } else if (err < 0) {
                corto_log("#[red]FAIL#[normal]: %s\n",
                    testcaseId);
            } else {
                /* Process exited with a returncode != 0 so
                 * must've printed an error msg itself */
                corto_catch();
            }
            if (!ciEnv || stricmp(ciEnv, "true")) {
                corto_id cmd;
                corto_log(" Use this command to debug the testcase:\n  %s\n\n",
                    test_command(cmd, this->lib, object));
            }

            if (ret == 1) {
                corto_claim(object);
                corto_ll_append(this->empty, object);
            } else {
                corto_claim(object);
                corto_ll_append(this->failures, object);
            }
        } else {
            corto_claim(object);
            corto_ll_append(this->successes, object);
        }
        this->testsRun++;

        /* Don't print statistics when in CI mode */
        if ((!ciEnv || stricmp(ciEnv, "true")) && (corto_log_verbosityGet() > CORTO_TRACE)) {
            corto_time start;
            corto_time_get(&start);
            corto_float64 diff =
              corto_time_toDouble(start) - corto_time_toDouble(this->timer);

            if (diff > 0.05) {
                test_updateProgress(this);
                corto_time_get(&this->timer);
            }
        } else {
            /* When in CI mode, show each individual testcase. Failures are
             * already reported. */
            if (!err && !ret) {
                corto_log("#[green]%s#[normal]: %s\n",
                    "PASS",
                    testcaseId);
            }
        }

        if (corto_rm(corto_getenv("BAKE_TARGET"))) {
            corto_lasterr(); /* Catch error */
            corto_throw(NULL);
        }
        corto_setenv("BAKE_TARGET", oldenv);
    }
}
