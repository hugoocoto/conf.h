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
        TEST("Conf_get_num valid", Conf_get_num(conf, "NumVal", &val) == CONF_OK);
        TEST("Conf_get_num value", val == 42.5);

        /* nested success */
        TEST("Conf_get_num nested",
             Conf_get_num(conf, "Deep.a.b.c", &val) == CONF_OK);
        TEST("Conf_get_num nested value", val == 42);

        /* missing key */
        TEST("Conf_get_num missing",
             Conf_get_num(conf, "Nonexistent", &val) == CONF_UNDEF);

        /* deep missing intermediate */
        TEST("Conf_get_num missing deep",
             Conf_get_num(conf, "Deep.a.z", &val) == CONF_UNDEF);

        /* type mismatch: string */
        TEST("Conf_get_num string",
             Conf_get_num(conf, "StrVal", &val) == CONF_INVALID);

        /* type mismatch: bool */
        TEST("Conf_get_num bool",
             Conf_get_num(conf, "TrueVal", &val) == CONF_INVALID);

        /* type mismatch: table (final value) */
        TEST("Conf_get_num table",
             Conf_get_num(conf, "TableVal", &val) == CONF_INVALID);

        /* intermediate not a table */
        TEST("Conf_get_num intermediate not table",
             Conf_get_num(conf, "Deep.a.b.c.foo", &val) == CONF_INVALID);

        /* deep missing last key (nil) */
        TEST("Conf_get_num deep nil",
             Conf_get_num(conf, "Deep.a.b.z", &val) == CONF_UNDEF);

        Conf_close(conf);
}

static void
test_get_int(void)
{
        Conf conf;
        long long val;

        Conf_open(&conf, "test.lua");

        /* success */
        TEST("Conf_get_int valid", Conf_get_int(conf, "IntVal", &val) == CONF_OK);
        TEST("Conf_get_int value", val == 100);

        /* negative */
        TEST("Conf_get_int negative",
             Conf_get_int(conf, "NegInt", &val) == CONF_OK);
        TEST("Conf_get_int negative value", val == -7);

        /* zero */
        TEST("Conf_get_int zero",
             Conf_get_int(conf, "ZeroInt", &val) == CONF_OK);
        TEST("Conf_get_int zero value", val == 0);

        /* float rejected */
        TEST("Conf_get_int float rejected",
             Conf_get_int(conf, "FloatVal", &val) == CONF_INVALID);

        /* missing key */
        TEST("Conf_get_int missing",
             Conf_get_int(conf, "Nonexistent", &val) == CONF_UNDEF);

        /* deep missing */
        TEST("Conf_get_int deep nil",
             Conf_get_int(conf, "Deep.a.z", &val) == CONF_UNDEF);

        /* type mismatch: string */
        TEST("Conf_get_int string",
             Conf_get_int(conf, "StrVal", &val) == CONF_INVALID);

        /* type mismatch: bool */
        TEST("Conf_get_int bool",
             Conf_get_int(conf, "TrueVal", &val) == CONF_INVALID);

        /* intermediate not table */
        TEST("Conf_get_int intermediate not table",
             Conf_get_int(conf, "Deep.a.b.c.foo", &val) == CONF_INVALID);

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
             Conf_get_str(conf, "StrVal", &str) == CONF_OK);
        TEST("Conf_get_str value", strcmp(str, "hello") == 0);

        /* empty string */
        TEST("Conf_get_str empty",
             Conf_get_str(conf, "EmptyStr", &str) == CONF_OK);
        TEST("Conf_get_str empty value", strcmp(str, "") == 0);

        /* nested string */
        TEST("Conf_get_str nested",
             Conf_get_str(conf, "Deep.a.b.d", &str) == CONF_OK);
        TEST("Conf_get_str nested value", strcmp(str, "nested_str") == 0);

        /* missing key */
        TEST("Conf_get_str missing",
             Conf_get_str(conf, "Nonexistent", &str) == CONF_UNDEF);

        /* type mismatch: number */
        TEST("Conf_get_str number",
             Conf_get_str(conf, "NumVal", &str) == CONF_INVALID);

        /* type mismatch: bool */
        TEST("Conf_get_str bool",
             Conf_get_str(conf, "TrueVal", &str) == CONF_INVALID);

        /* type mismatch: table */
        TEST("Conf_get_str table",
             Conf_get_str(conf, "TableVal", &str) == CONF_INVALID);

        /* intermediate not table */
        TEST("Conf_get_str intermediate not table",
             Conf_get_str(conf, "Deep.a.b.c.foo", &str) == CONF_INVALID);

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
             Conf_get_bool(conf, "TrueVal", &val) == CONF_OK);
        TEST("Conf_get_bool true value", val != 0);

        /* false */
        TEST("Conf_get_bool false",
             Conf_get_bool(conf, "FalseVal", &val) == CONF_OK);
        TEST("Conf_get_bool false value", val == 0);

        /* missing key */
        TEST("Conf_get_bool missing",
             Conf_get_bool(conf, "Nonexistent", &val) == CONF_UNDEF);

        /* type mismatch: number */
        TEST("Conf_get_bool number",
             Conf_get_bool(conf, "NumVal", &val) == CONF_INVALID);

        /* type mismatch: string */
        TEST("Conf_get_bool string",
             Conf_get_bool(conf, "StrVal", &val) == CONF_INVALID);

        /* type mismatch: table */
        TEST("Conf_get_bool table",
             Conf_get_bool(conf, "TableVal", &val) == CONF_INVALID);

        /* intermediate not table */
        TEST("Conf_get_bool intermediate not table",
             Conf_get_bool(conf, "Deep.a.b.c.foo", &val) == CONF_INVALID);

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

        /* ── Conf_get_len ────────────────────────────────────── */

        TEST("CGL valid", Conf_get_len(conf, "List", &len) == CONF_OK);
        TEST("CGL valid value", len == 3);
        TEST("CGL missing", Conf_get_len(conf, "Nonexistent", &len) == CONF_UNDEF);
        TEST("CGL not table", Conf_get_len(conf, "NumVal", &len) == CONF_INVALID);
        TEST("CGL empty list", Conf_get_len(conf, "EmptyList", &len) == CONF_OK);
        TEST("CGL empty list value", len == 0);
        TEST("CGL pure dict", Conf_get_len(conf, "PureDict", &len) == CONF_OK);
        TEST("CGL pure dict value", len == 0);
        TEST("CGL nested path", Conf_get_len(conf, "Deep.a.b", &len) == CONF_OK);
        TEST("CGL nested non-list", Conf_get_len(conf, "Deep.a.b.c", &len) == CONF_INVALID);
        TEST("CGL string", Conf_get_len(conf, "StrVal", &len) == CONF_INVALID);
        TEST("CGL bool", Conf_get_len(conf, "TrueVal", &len) == CONF_INVALID);
        TEST("CGL intermediate not table",
             Conf_get_len(conf, "Deep.a.b.c.x", &len) == CONF_INVALID);

        /* ── Conf_get_elem_int with field ────────────────────── */

        TEST("CGEI list 1", Conf_get_elem_int(conf, "List", 1, "n", &ival) == CONF_OK);
        TEST("CGEI list 1 val", ival == 1);
        TEST("CGEI list 2", Conf_get_elem_int(conf, "List", 2, "n", &ival) == CONF_OK);
        TEST("CGEI list 2 val", ival == 2);
        TEST("CGEI list 3", Conf_get_elem_int(conf, "List", 3, "n", &ival) == CONF_OK);
        TEST("CGEI list 3 val", ival == 3);

        /* nested list: Items.2.value */
        TEST("CGEI nested dot-name",
             Conf_get_elem_int(conf, "Items", 2, "value", &ival) == CONF_OK);
        TEST("CGEI nested dot-name val", ival == 20);

        /* field with numeric sub-path: Matrix.1.2 */
        TEST("CGEI field numeric sub",
             Conf_get_elem_int(conf, "Matrix", 1, "2", &ival) == CONF_OK);
        TEST("CGEI field numeric sub val", ival == 2);

        /* ── Conf_get_elem_num with field=NULL on flat list ──── */

        TEST("CGEN flat", Conf_get_elem_num(conf, "FlatNum", 1, NULL, &dval) == CONF_OK);
        TEST("CGEN flat val", dval == 1.5);
        TEST("CGEN flat 2", Conf_get_elem_num(conf, "FlatNum", 3, NULL, &dval) == CONF_OK);
        TEST("CGEN flat 2 val", dval == 3.5);

        /* ── Conf_get_elem_str with field=NULL on flat list ─── */

        TEST("CGES flat", Conf_get_elem_str(conf, "FlatStr", 1, NULL, &sval) == CONF_OK);
        TEST("CGES flat val", strcmp(sval, "a") == 0);
        TEST("CGES flat 2", Conf_get_elem_str(conf, "FlatStr", 3, NULL, &sval) == CONF_OK);
        TEST("CGES flat 2 val", strcmp(sval, "c") == 0);

        /* ── Conf_get_elem_bool with field=NULL on flat list ── */

        TEST("CGEB flat true", Conf_get_elem_bool(conf, "FlatBool", 1, NULL, &bval) == CONF_OK);
        TEST("CGEB flat true val", bval != 0);
        TEST("CGEB flat false", Conf_get_elem_bool(conf, "FlatBool", 2, NULL, &bval) == CONF_OK);
        TEST("CGEB flat false val", bval == 0);
        TEST("CGEB flat 3", Conf_get_elem_bool(conf, "FlatBool", 3, NULL, &bval) == CONF_OK);
        TEST("CGEB flat 3 val", bval != 0);

        /* ── numeric index in raw path ───────────────────────── */

        TEST("raw numeric", Conf_get_int(conf, "List.2.n", &ival) == CONF_OK);
        TEST("raw numeric val", ival == 2);
        TEST("raw numeric nested", Conf_get_int(conf, "Items.2.value", &ival) == CONF_OK);
        TEST("raw numeric nested val", ival == 20);
        TEST("raw numeric matrix", Conf_get_int(conf, "Matrix.2.3", &ival) == CONF_OK);
        TEST("raw numeric matrix val", ival == 6);
        TEST("raw numeric deep", Conf_get_num(conf, "FlatNum.2", &dval) == CONF_OK);
        TEST("raw numeric deep val", dval == 2.5);

        /* ── type mismatch: _elem_* on flat list with wrong getter ── */

        /* FlatInt.1 is an integer, not a string */
        TEST("CGEI flat int -> str mismatch",
             Conf_get_elem_str(conf, "FlatInt", 1, NULL, &sval) == CONF_INVALID);
        /* FlatNum.1 is a float, not an integer */
        TEST("CGEI flat float -> int mismatch",
             Conf_get_elem_int(conf, "FlatNum", 2, NULL, &ival) == CONF_INVALID);
        /* FlatStr.1 is a string, not a number */
        TEST("CGEI flat str -> num mismatch",
             Conf_get_elem_num(conf, "FlatStr", 1, NULL, &dval) == CONF_INVALID);
        /* FlatStr.1 is a string, not a bool */
        TEST("CGEI flat str -> bool mismatch",
             Conf_get_elem_bool(conf, "FlatStr", 1, NULL, &bval) == CONF_INVALID);
        /* FlatInt.1 is a number, not a bool */
        TEST("CGEI flat int -> bool mismatch",
             Conf_get_elem_bool(conf, "FlatInt", 1, NULL, &bval) == CONF_INVALID);

        /* ── type mismatch: _elem_* on list-of-tables with wrong getter ── */

        /* List.1 is a table, not an int */
        TEST("CGEI list table -> int mismatch",
             Conf_get_elem_int(conf, "List", 1, NULL, &ival) == CONF_INVALID);
        /* List.1.n is an int, not a string */
        TEST("CGEI list int -> str mismatch",
             Conf_get_elem_str(conf, "List", 1, "n", &sval) == CONF_INVALID);

        /* ── element exists but field is missing ─────────────── */
        TEST("CGEI missing field",
             Conf_get_elem_int(conf, "List", 1, "z", &ival) == CONF_UNDEF);
        TEST("CGEI missing nested field",
             Conf_get_elem_int(conf, "Items", 1, "missing", &ival) == CONF_UNDEF);

        /* ── out-of-range ────────────────────────────────────── */

        TEST("CGEI out of range high",
             Conf_get_elem_int(conf, "List", 99, "n", &ival) == CONF_UNDEF);
        TEST("CGEI out of range zero",
             Conf_get_elem_int(conf, "List", 0, "n", &ival) == CONF_UNDEF);
        TEST("CGEI out of range neg",
             Conf_get_elem_int(conf, "List", -1, "n", &ival) == CONF_UNDEF);
        TEST("CGL flat out of range high",
             Conf_get_elem_int(conf, "FlatInt", 99, NULL, &ival) == CONF_UNDEF);
        TEST("CGL flat out of range zero",
             Conf_get_elem_int(conf, "FlatInt", 0, NULL, &ival) == CONF_UNDEF);

        /* ── type mismatch on list name ──────────────────────── */
        TEST("CGEI name not table",
             Conf_get_elem_int(conf, "NumVal", 1, "n", &ival) == CONF_INVALID);
        TEST("CGEI name string",
             Conf_get_elem_str(conf, "StrVal", 1, NULL, &sval) == CONF_INVALID);

        /* ── intermediate not table in field path ────────────── */
        TEST("CGEI intermediate not table",
             Conf_get_elem_int(conf, "FlatInt", 1, "x", &ival) == CONF_INVALID);
        TEST("CGEI intermediate not table deep",
             Conf_get_elem_int(conf, "List", 1, "n.x", &ival) == CONF_INVALID);

        /* ── Conf_get_len extra edge cases ───────────────────── */
        {
                int l2;
                /* Deep is a table but has no array part */
                TEST("CGL Deep", Conf_get_len(conf, "Deep", &l2) == CONF_OK);
                TEST("CGL Deep val", l2 == 0);
                /* Scalar final value */
                TEST("CGL scalar final", Conf_get_len(conf, "List.1.n", &l2) == CONF_INVALID);
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
        TEST("return table num", Conf_get_num(conf, "NumVal", &dval) == CONF_OK);
        TEST("return table num val", dval == 42.5);
        TEST("return table int", Conf_get_int(conf, "IntVal", &ival) == CONF_OK);
        TEST("return table int val", ival == 100);
        TEST("return table str", Conf_get_str(conf, "StrVal", &sval) == CONF_OK);
        TEST("return table str val", strcmp(sval, "hello") == 0);
        TEST("return table true", Conf_get_bool(conf, "TrueVal", &bval) == CONF_OK);
        TEST("return table true val", bval != 0);
        TEST("return table false", Conf_get_bool(conf, "FalseVal", &bval) == CONF_OK);
        TEST("return table false val", bval == 0);

        /* nested paths work */
        TEST("return table nested", Conf_get_int(conf, "Deep.a.b", &ival) == CONF_OK);
        TEST("return table nested val", ival == 7);

        /* list operations work */
        TEST("return table list len", Conf_get_len(conf, "List", &len) == CONF_OK);
        TEST("return table list len val", len == 2);
        TEST("return table list elem",
             Conf_get_elem_int(conf, "List", 2, "n", &ival) == CONF_OK);
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
        TEST("return scalar read", Conf_get_int(conf, "NumVal", &ival) == CONF_UNDEF);
        Conf_close(conf);

        remove("/tmp/return_cfg.lua");
        remove("/tmp/return_scalar.lua");
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

        printf("\n");
        if (nfailed == 0)
                printf("ALL TESTS PASSED\n");
        else
                printf("SOME TESTS FAILED\n");

        return nfailed > 0 ? 1 : 0;
}
