/* This is a managed file. Do not delete this comment. */

#include <corto.test>

#define FIND(p, i) corto(CORTO_LOOKUP, {.parent=p, .id=i})

static void test_updateProgress(test_Runner this) {
    if (ut_ll_count(this->failures)) {
        if (ut_ll_count(this->empty)) {
            ut_log("#[]%s: PASS:%d, #[red]FAIL#[normal]:%d, #[yellow]EMPTY#[normal]:%d",
                this->name,
                ut_ll_count(this->successes),
                ut_ll_count(this->failures),
                ut_ll_count(this->empty));
            } else {
                ut_log("#[]%s: PASS:%d, #[red]FAIL#[normal]:%d, EMPTY:%d",
                    this->name,
                    ut_ll_count(this->successes),
                    ut_ll_count(this->failures),
                    ut_ll_count(this->empty));
            }
    } else {
        if (ut_ll_count(this->empty)) {
            ut_log("#[]%s: #[green]PASS#[normal]:%d, FAIL:%d, #[yellow]EMPTY#[normal]:%d",
                this->name,
                ut_ll_count(this->successes),
                ut_ll_count(this->failures),
                ut_ll_count(this->empty));
        } else {
            ut_log("#[]%s: #[green]PASS#[normal]:%d, FAIL:%d, EMPTY:%d",
                this->name,
                ut_ll_count(this->successes),
                ut_ll_count(this->failures),
                ut_ll_count(this->empty));
        }
    }
}

static ut_proc test_Runner_forkTestCase(test_Runner this, corto_id testcaseId) {
    char *tool = getenv("CORTO_TEST_TOOL");
    ut_proc pid = 0;

    if (!tool || !strcmp(tool, "")) {
        pid = ut_proc_run(
            "corto",
            (const char*[]){"corto", "--mute", "-l", this->lib, testcaseId, NULL}
        );
        goto end;
    }
    if (!strcmp(tool, "check-memory")) {
        setenv("CORTO_TEST_RUNSLOW", "TRUE", 1);
        if (strcmp(UT_OS_STRING, "linux")) {
            ut_throw("check-memory is only supported on Linux (uses valgrind)");
            goto end;
        }
        pid = ut_proc_run(
            "valgrind",
            (const char*[]){"valgrind", "-q", "corto", "-l", this->lib, testcaseId, NULL}
        );
        goto end;
    }
    if (!strcmp(tool, "check-thread")) {
        setenv("CORTO_TEST_RUNSLOW", "TRUE", 1);
        if (strcmp(UT_OS_STRING, "linux")) {
            ut_throw("check-thread is only supported on Linux (uses valgrind)");
            goto end;
        }
        pid = ut_proc_run(
            "valgrind",
            (const char*[]){"valgrind", "-q", "--tool=helgrind", "corto", "-l", this->lib, testcaseId, NULL}
        );
        goto end;
    }
end:
    return pid;
}

int16_t test_Runner_construct(
    test_Runner this)
{
    /* Use corto_find, to prevent loading a package /test, if it exists */
    corto_object testroot = FIND(root_o, "test");
    if (!testroot) {
        ut_throw("test: /test not found");
        goto error;
    }

    /* If a testcase is provided, run it. Otherwise, discover testcases and
     * forward to separate process. */
    if (this->testcase) {
        corto_object testcase = FIND(testroot, this->testcase);
        if (testcase) {
            ut_log_push(strarg("test:%s", this->testcase));
            corto_type testClass = corto_parentof(testcase);

            test_SuiteData suite = test_SuiteData(corto_create(NULL, NULL, testClass));

            if (ut_getenv("CORTO_TEST_BY_ID") && !stricmp(ut_getenv("CORTO_TEST_BY_ID"), "true")) {
                corto_id cmd;
                ut_log("RUN %s\n",
                    test_command(cmd, this->lib, testcase));
            }

            if (test_SuiteData_run(suite, testcase)) {
                ut_error("test: failed to define test suite");
                corto_claim(testcase);
                ut_ll_append(this->failures, testcase);
            } else {
                corto_object prev = corto_set_source(this);
                corto_define(suite);
                corto_set_source(prev);

                ut_ok("DONE  %s", this->testcase);
            }
            ut_log_pop();

            corto_delete(suite);
        } else {
            /* Testcase not found, don't report error */
            ut_trace("test: %s NOT FOUND", this->testcase);
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
    corto_observer_event *e)
{
    test_Runner this = e->instance;
    corto_object object = e->data;

    if (!corto_instanceof(corto_type(test_Case_o), object)) {
        goto end;
    }

    corto_id testcaseId;
    corto_int8 err = 0, ret = 0;
    corto_string ciEnv = ut_getenv("CI");

    test_id(testcaseId, object);

    {
        corto_id cmd; (void)cmd;
        ut_trace("\ntest: TESTCASE %s", testcaseId);
        ut_trace("test:   EXEC  %s",
            test_command(cmd, this->lib, object));
    }

    ut_proc pid = test_Runner_forkTestCase(this, testcaseId);

    if (!pid) {
        ut_raise();
        ut_log("#[red]FAIL#[normal]: %s\n",
            testcaseId);
    } else if ((err = ut_proc_wait(pid, &ret)) || ret) {
        if (err > 0) {
            ut_catch();
            test_erase();
            ut_log("#[red]FAIL#[normal]: %s: test crashed with signal %d\n",
                testcaseId, err);
        } else if (err < 0) {
            ut_log("#[red]FAIL#[normal]: %s\n",
                testcaseId);
        } else {
            /* Process exited with a returncode != 0 so
             * must've printed an error msg itself */
            ut_catch();
        }
        if (!ciEnv || stricmp(ciEnv, "true")) {
            corto_id cmd;
            ut_log(" Use this command to debug the testcase:\n  %s\n\n",
                test_command(cmd, this->lib, object));
        }

        if (ret == 1) {
            corto_claim(object);
            ut_ll_append(this->empty, object);
        } else {
            corto_claim(object);
            ut_ll_append(this->failures, object);
        }
    } else {
        corto_claim(object);
        ut_ll_append(this->successes, object);
    }
    this->testsRun++;

    /* Don't print statistics when in CI mode */
    if ((!ciEnv || stricmp(ciEnv, "true")) && (ut_log_verbosityGet() > UT_TRACE)) {
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
            ut_log("#[green]%s#[normal]: %s\n",
                "PASS",
                testcaseId);
        }
    }

end:;
}
