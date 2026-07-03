#define INCLUDE_CONF_IMPLEMENTATION
#include "conf.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int nfailed = 0;
static int npassed = 0;

#define TEST(name, expr)                                                 \
        do {                                                             \
                if (!(expr)) {                                           \
                        fprintf(stderr, "FAIL: %s (%s)\n", name, #expr); \
                        nfailed++;                                       \
                } else {                                                 \
                        npassed++;                                       \
                }                                                        \
        } while (0)

static void
test_open(void)
{
        Conf conf;

        /* success */
        TEST("Conf_open valid file",
             Conf_open(&conf, "test.lua") == CONF_OK);
        Conf_close(conf);

        /* file not found */
        TEST("Conf_open missing file",
             Conf_open(&conf, "nonexistent.lua") == CONF_NOTFOUND);

        /* syntax error in file */
        {
                FILE *f = fopen("/tmp/bad.lua", "w");
                assert(f);
                fprintf(f, "syntax error {{{{{");
                fclose(f);
                TEST("Conf_open syntax error",
                     Conf_open(&conf, "/tmp/bad.lua") == CONF_NOTFOUND);
                remove("/tmp/bad.lua");
        }
}

static void
test_close(void)
{
        Conf conf;

        /* success */
        Conf_open(&conf, "test.lua");
        TEST("Conf_close valid", Conf_close(conf) == CONF_OK);

        /* NULL handle */
        TEST("Conf_close NULL", Conf_close(NULL) == CONF_INVALID);
}

static void
test_get_num(void)
{
        Conf conf;
        double val;

        Conf_open(&conf, "test.lua");

        /* success */
        TEST("Conf_get_num valid", Conf_get_num(conf, &val, "NumVal") == CONF_OK);
        TEST("Conf_get_num value", val == 42.5);

        /* nested success */
        TEST("Conf_get_num nested",
             Conf_get_num(conf, &val, "Deep.a.b.c") == CONF_OK);
        TEST("Conf_get_num nested value", val == 42);

        /* missing key */
        TEST("Conf_get_num missing",
             Conf_get_num(conf, &val, "Nonexistent") == CONF_UNDEF);

        /* deep missing intermediate */
        TEST("Conf_get_num missing deep",
             Conf_get_num(conf, &val, "Deep.a.z") == CONF_UNDEF);

        /* type mismatch: string */
        TEST("Conf_get_num string",
             Conf_get_num(conf, &val, "StrVal") == CONF_INVALID);

        /* type mismatch: bool */
        TEST("Conf_get_num bool",
             Conf_get_num(conf, &val, "TrueVal") == CONF_INVALID);

        /* type mismatch: table (final value) */
        TEST("Conf_get_num table",
             Conf_get_num(conf, &val, "TableVal") == CONF_INVALID);

        /* intermediate not a table */
        TEST("Conf_get_num intermediate not table",
             Conf_get_num(conf, &val, "Deep.a.b.c.foo") == CONF_INVALID);

        /* deep missing last key (nil) */
        TEST("Conf_get_num deep nil",
             Conf_get_num(conf, &val, "Deep.a.b.z") == CONF_UNDEF);

        Conf_close(conf);
}

static void
test_get_int(void)
{
        Conf conf;
        long long val;

        Conf_open(&conf, "test.lua");

        /* success */
        TEST("Conf_get_int valid", Conf_get_int(conf, &val, "IntVal") == CONF_OK);
        TEST("Conf_get_int value", val == 100);

        /* negative */
        TEST("Conf_get_int negative",
             Conf_get_int(conf, &val, "NegInt") == CONF_OK);
        TEST("Conf_get_int negative value", val == -7);

        /* zero */
        TEST("Conf_get_int zero",
             Conf_get_int(conf, &val, "ZeroInt") == CONF_OK);
        TEST("Conf_get_int zero value", val == 0);

        /* float rejected */
        TEST("Conf_get_int float rejected",
             Conf_get_int(conf, &val, "FloatVal") == CONF_INVALID);

        /* missing key */
        TEST("Conf_get_int missing",
             Conf_get_int(conf, &val, "Nonexistent") == CONF_UNDEF);

        /* deep missing */
        TEST("Conf_get_int deep nil",
             Conf_get_int(conf, &val, "Deep.a.z") == CONF_UNDEF);

        /* type mismatch: string */
        TEST("Conf_get_int string",
             Conf_get_int(conf, &val, "StrVal") == CONF_INVALID);

        /* type mismatch: bool */
        TEST("Conf_get_int bool",
             Conf_get_int(conf, &val, "TrueVal") == CONF_INVALID);

        /* intermediate not table */
        TEST("Conf_get_int intermediate not table",
             Conf_get_int(conf, &val, "Deep.a.b.c.foo") == CONF_INVALID);

        Conf_close(conf);
}

static void
test_get_str(void)
{
        Conf conf;
        const char *str;

        Conf_open(&conf, "test.lua");

        /* success */
        TEST("Conf_get_str valid",
             Conf_get_str(conf, &str, "StrVal") == CONF_OK);
        TEST("Conf_get_str value", strcmp(str, "hello") == 0);

        /* empty string */
        TEST("Conf_get_str empty",
             Conf_get_str(conf, &str, "EmptyStr") == CONF_OK);
        TEST("Conf_get_str empty value", strcmp(str, "") == 0);

        /* nested string */
        TEST("Conf_get_str nested",
             Conf_get_str(conf, &str, "Deep.a.b.d") == CONF_OK);
        TEST("Conf_get_str nested value", strcmp(str, "nested_str") == 0);

        /* missing key */
        TEST("Conf_get_str missing",
             Conf_get_str(conf, &str, "Nonexistent") == CONF_UNDEF);

        /* type mismatch: number */
        TEST("Conf_get_str number",
             Conf_get_str(conf, &str, "NumVal") == CONF_INVALID);

        /* type mismatch: bool */
        TEST("Conf_get_str bool",
             Conf_get_str(conf, &str, "TrueVal") == CONF_INVALID);

        /* type mismatch: table */
        TEST("Conf_get_str table",
             Conf_get_str(conf, &str, "TableVal") == CONF_INVALID);

        /* intermediate not table */
        TEST("Conf_get_str intermediate not table",
             Conf_get_str(conf, &str, "Deep.a.b.c.foo") == CONF_INVALID);

        Conf_close(conf);
}

static void
test_get_bool(void)
{
        Conf conf;
        int val;

        Conf_open(&conf, "test.lua");

        /* true */
        TEST("Conf_get_bool true",
             Conf_get_bool(conf, &val, "TrueVal") == CONF_OK);
        TEST("Conf_get_bool true value", val != 0);

        /* false */
        TEST("Conf_get_bool false",
             Conf_get_bool(conf, &val, "FalseVal") == CONF_OK);
        TEST("Conf_get_bool false value", val == 0);

        /* missing key */
        TEST("Conf_get_bool missing",
             Conf_get_bool(conf, &val, "Nonexistent") == CONF_UNDEF);

        /* type mismatch: number */
        TEST("Conf_get_bool number",
             Conf_get_bool(conf, &val, "NumVal") == CONF_INVALID);

        /* type mismatch: string */
        TEST("Conf_get_bool string",
             Conf_get_bool(conf, &val, "StrVal") == CONF_INVALID);

        /* type mismatch: table */
        TEST("Conf_get_bool table",
             Conf_get_bool(conf, &val, "TableVal") == CONF_INVALID);

        /* intermediate not table */
        TEST("Conf_get_bool intermediate not table",
             Conf_get_bool(conf, &val, "Deep.a.b.c.foo") == CONF_INVALID);

        Conf_close(conf);
}

static void
test_get_list(void)
{
        Conf conf;
        int len;
        long long ival;
        double dval;
        const char *sval;
        int bval;

        Conf_open(&conf, "test.lua");

        /* Conf_get_len */
        TEST("get_len valid", Conf_get_len(conf, &len, "List") == CONF_OK);
        TEST("get_len val", len == 3);
        TEST("get_len missing", Conf_get_len(conf, &len, "Nonexistent") == CONF_UNDEF);
        TEST("get_len not table", Conf_get_len(conf, &len, "NumVal") == CONF_INVALID);
        TEST("get_len empty", Conf_get_len(conf, &len, "EmptyList") == CONF_OK);
        TEST("get_len empty val", len == 0);
        TEST("get_len pure dict", Conf_get_len(conf, &len, "PureDict") == CONF_OK);
        TEST("get_len pure dict val", len == 0);
        TEST("get_len nested", Conf_get_len(conf, &len, "Deep.a.b") == CONF_OK);
        TEST("get_len scalar", Conf_get_len(conf, &len, "Deep.a.b.c") == CONF_INVALID);
        TEST("get_len string", Conf_get_len(conf, &len, "StrVal") == CONF_INVALID);
        TEST("get_len bool", Conf_get_len(conf, &len, "TrueVal") == CONF_INVALID);
        TEST("get_len deep invalid",
             Conf_get_len(conf, &len, "Deep.a.b.c.x") == CONF_INVALID);

        /* iterate List with %d */
        TEST("get_int List.1.n", Conf_get_int(conf, &ival, "List.%d.n", 1) == CONF_OK);
        TEST("get_int List.1.n val", ival == 1);
        TEST("get_int List.2.n", Conf_get_int(conf, &ival, "List.%d.n", 2) == CONF_OK);
        TEST("get_int List.2.n val", ival == 2);
        TEST("get_int List.3.n", Conf_get_int(conf, &ival, "List.%d.n", 3) == CONF_OK);
        TEST("get_int List.3.n val", ival == 3);

        /* iterate Items with %d */
        TEST("get_int Items.2.value", Conf_get_int(conf, &ival, "Items.%d.value", 2) == CONF_OK);
        TEST("get_int Items.2.value val", ival == 20);

        /* iterate Matrix with %d */
        TEST("get_int Matrix.1.2", Conf_get_int(conf, &ival, "Matrix.%d.%d", 1, 2) == CONF_OK);
        TEST("get_int Matrix.1.2 val", ival == 2);
        TEST("get_int Matrix.2.3", Conf_get_int(conf, &ival, "Matrix.%d.%d", 2, 3) == CONF_OK);
        TEST("get_int Matrix.2.3 val", ival == 6);

        /* flat lists with %d */
        TEST("get_num FlatNum.1", Conf_get_num(conf, &dval, "FlatNum.%d", 1) == CONF_OK);
        TEST("get_num FlatNum.1 val", dval == 1.5);
        TEST("get_num FlatNum.3", Conf_get_num(conf, &dval, "FlatNum.%d", 3) == CONF_OK);
        TEST("get_num FlatNum.3 val", dval == 3.5);
        TEST("get_int FlatInt.2", Conf_get_int(conf, &ival, "FlatInt.%d", 2) == CONF_OK);
        TEST("get_int FlatInt.2 val", ival == 20);
        TEST("get_str FlatStr.1", Conf_get_str(conf, &sval, "FlatStr.%d", 1) == CONF_OK);
        TEST("get_str FlatStr.1 val", strcmp(sval, "a") == 0);
        TEST("get_str FlatStr.3", Conf_get_str(conf, &sval, "FlatStr.%d", 3) == CONF_OK);
        TEST("get_str FlatStr.3 val", strcmp(sval, "c") == 0);
        TEST("get_bool FlatBool.1", Conf_get_bool(conf, &bval, "FlatBool.%d", 1) == CONF_OK);
        TEST("get_bool FlatBool.1 val", bval != 0);
        TEST("get_bool FlatBool.2", Conf_get_bool(conf, &bval, "FlatBool.%d", 2) == CONF_OK);
        TEST("get_bool FlatBool.2 val", bval == 0);

        /* type mismatch on flat list */
        TEST("get_str FlatInt.1 mismatch",
             Conf_get_str(conf, &sval, "FlatInt.%d", 1) == CONF_INVALID);
        TEST("get_int FlatNum.2 mismatch",
             Conf_get_int(conf, &ival, "FlatNum.%d", 2) == CONF_INVALID);
        TEST("get_num FlatStr.1 mismatch",
             Conf_get_num(conf, &dval, "FlatStr.%d", 1) == CONF_INVALID);
        TEST("get_bool FlatStr.1 mismatch",
             Conf_get_bool(conf, &bval, "FlatStr.%d", 1) == CONF_INVALID);
        TEST("get_bool FlatInt.1 mismatch",
             Conf_get_bool(conf, &bval, "FlatInt.%d", 1) == CONF_INVALID);

        /* list-of-tables as scalar mismatch */
        TEST("get_int List.1 mismatch",
             Conf_get_int(conf, &ival, "List.%d", 1) == CONF_INVALID);
        TEST("get_str List.1.n mismatch",
             Conf_get_str(conf, &sval, "List.%d.n", 1) == CONF_INVALID);

        /* missing field */
        TEST("get_int missing field",
             Conf_get_int(conf, &ival, "List.%d.z", 1) == CONF_UNDEF);
        TEST("get_int missing nested field",
             Conf_get_int(conf, &ival, "Items.%d.missing", 1) == CONF_UNDEF);

        /* out-of-range */
        TEST("get_int OOR high",
             Conf_get_int(conf, &ival, "List.%d.n", 99) == CONF_UNDEF);
        TEST("get_int OOR zero",
             Conf_get_int(conf, &ival, "List.%d.n", 0) == CONF_UNDEF);
        TEST("get_int OOR neg",
             Conf_get_int(conf, &ival, "List.%d.n", -1) == CONF_UNDEF);
        TEST("get_int flat OOR high",
             Conf_get_int(conf, &ival, "FlatInt.%d", 99) == CONF_UNDEF);
        TEST("get_int flat OOR zero",
             Conf_get_int(conf, &ival, "FlatInt.%d", 0) == CONF_UNDEF);

        /* name not a table */
        TEST("get_int name not table",
             Conf_get_int(conf, &ival, "NumVal.%d.n", 1) == CONF_INVALID);
        TEST("get_str name string",
             Conf_get_str(conf, &sval, "StrVal.%d", 1) == CONF_INVALID);

        /* intermediate not table */
        TEST("get_int intermediate not table",
             Conf_get_int(conf, &ival, "FlatInt.%d.x", 1) == CONF_INVALID);
        TEST("get_int intermediate deep",
             Conf_get_int(conf, &ival, "List.%d.n.x", 1) == CONF_INVALID);

        /* Conf_get_len on dict table */
        {
                int l2;
                TEST("get_len Deep", Conf_get_len(conf, &l2, "Deep") == CONF_OK);
                TEST("get_len Deep val", l2 == 0);
                TEST("get_len scalar final",
                     Conf_get_len(conf, &l2, "List.%d.n", 1) == CONF_INVALID);
        }

        Conf_close(conf);
}

static void
test_open_return_table(void)
{
        Conf conf;
        double dval;
        long long ival;
        const char *sval;
        int bval;
        int len;

        /* create a Lua file with return { ... } style */
        {
                FILE *f = fopen("/tmp/return_cfg.lua", "w");
                assert(f);
                fprintf(f,
                        "return {\n"
                        "  NumVal = 42.5,\n"
                        "  IntVal = 100,\n"
                        "  StrVal = \"hello\",\n"
                        "  TrueVal = true,\n"
                        "  FalseVal = false,\n"
                        "  List = {\n"
                        "    { n = 1 },\n"
                        "    { n = 2 },\n"
                        "  },\n"
                        "  Deep = { a = { b = 7 } },\n"
                        "}\n");
                fclose(f);
        }

        TEST("return table open", Conf_open(&conf, "/tmp/return_cfg.lua") == CONF_OK);

        /* globals lifted from the returned table */
        TEST("return table num", Conf_get_num(conf, &dval, "NumVal") == CONF_OK);
        TEST("return table num val", dval == 42.5);
        TEST("return table int", Conf_get_int(conf, &ival, "IntVal") == CONF_OK);
        TEST("return table int val", ival == 100);
        TEST("return table str", Conf_get_str(conf, &sval, "StrVal") == CONF_OK);
        TEST("return table str val", strcmp(sval, "hello") == 0);
        TEST("return table true", Conf_get_bool(conf, &bval, "TrueVal") == CONF_OK);
        TEST("return table true val", bval != 0);
        TEST("return table false", Conf_get_bool(conf, &bval, "FalseVal") == CONF_OK);
        TEST("return table false val", bval == 0);

        /* nested paths work */
        TEST("return table nested", Conf_get_int(conf, &ival, "Deep.a.b") == CONF_OK);
        TEST("return table nested val", ival == 7);

        /* list operations work */
        TEST("return table list len", Conf_get_len(conf, &len, "List") == CONF_OK);
        TEST("return table list len val", len == 2);
        TEST("return table list elem",
             Conf_get_int(conf, &ival, "List.%d.n", 2) == CONF_OK);
        TEST("return table list elem val", ival == 2);

        Conf_close(conf);

        /* file that returns a non-table (should still work) */
        {
                FILE *f = fopen("/tmp/return_scalar.lua", "w");
                assert(f);
                fprintf(f, "return 42\n");
                fclose(f);
        }
        TEST("return scalar open", Conf_open(&conf, "/tmp/return_scalar.lua") == CONF_OK);
        /* no globals set; any get should return UNDEF */
        TEST("return scalar read", Conf_get_int(conf, &ival, "NumVal") == CONF_UNDEF);
        Conf_close(conf);

        remove("/tmp/return_cfg.lua");
        remove("/tmp/return_scalar.lua");
}

static void
test_trunc(void)
{
        Conf conf;
        long long ival;
        double dval;
        const char *sval;
        int bval;
        int len;

        Conf_open(&conf, "test.lua");

        /* build a path that exceeds 1024 bytes */
        char buf[1100];
        int pos = snprintf(buf, sizeof(buf), "List.%d", 1);
        while (pos < 1024)
                pos += snprintf(buf + pos, sizeof(buf) - pos, ".n");

        /* now add more to guarantee overflow */
        snprintf(buf + pos, sizeof(buf) - pos, ".x");

        TEST("trunc int", Conf_get_int(conf, &ival, "%s", buf) == CONF_TRUNC);
        TEST("trunc num", Conf_get_num(conf, &dval, "%s", buf) == CONF_TRUNC);
        TEST("trunc str", Conf_get_str(conf, &sval, "%s", buf) == CONF_TRUNC);
        TEST("trunc bool", Conf_get_bool(conf, &bval, "%s", buf) == CONF_TRUNC);
        TEST("trunc len", Conf_get_len(conf, &len, "%s", buf) == CONF_TRUNC);

        Conf_close(conf);
}


int
main(void)
{
        printf("=== Conf Library Tests ===\n\n");

        test_open();
        printf("  Conf_open tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        test_close();
        printf("  Conf_close tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        test_get_num();
        printf("  Conf_get_num tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        test_get_int();
        printf("  Conf_get_int tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        test_get_str();
        printf("  Conf_get_str tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        test_get_bool();
        printf("  Conf_get_bool tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        test_get_list();
        printf("  Conf_get_list tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        test_open_return_table();
        printf("  Conf_open return table tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        test_trunc();
        printf("  Conf_trunc tests: %d passed, %d failed\n", npassed, nfailed);
        npassed = nfailed = 0;

        printf("\n");
        if (nfailed == 0)
                printf("ALL TESTS PASSED\n");
        else
                printf("SOME TESTS FAILED\n");

        return nfailed > 0 ? 1 : 0;
}
