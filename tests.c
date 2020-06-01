#include <stdio.h>
#include <stdbool.h>

#include "clam.h"

#ifdef QUIET
#define printf(...)
#endif

static int error_code = 0;
static int print_positive_assert = true;

void disable_positive_asserts() {
        print_positive_assert = false;
}
void enable_positive_asserts() {
        print_positive_assert = true;
}

#define ASSERT(x, description) { int x_; \
        if (!(x_ = x)) { \
                printf("* [ ] **Failure:** %s does not hold, got %d (%s:%d)\n          %s\n", description, x_, __FILE__, __LINE__, #x); \
                error_code = 1; \
        } else { \
                if (print_positive_assert) { \
                  printf("* [X] Success: %s\n", description); \
                } \
        } \
} 

int main(int argc, char *argv[])
{

        {
            printf("# Basic character matching\n");

            ASSERT(clam_match_char("A", 'A'),
                "`clam_match_char` should match a matching character");
            ASSERT(!clam_match_char("A", 'a'),
                "`clam_match_char` should not match a non-matching character");

            ASSERT(clam_match_end(""),
                "`clam_match_end` should match an empty string");
            ASSERT(!clam_match_end("A"),
                "`clam_match_end` should not match a string");

            ASSERT(clam_match_anychar("A", "aA123"),
                "`clam_match_anychar` should match an allowed character");
            ASSERT(clam_match_anychar("A", NULL),
                "`clam_match_anychar` should match any character if any are allowed");
            ASSERT(!clam_match_anychar("B", "aA123"),
                "`clam_match_anychar` should not match a character that is not allowed");

            ASSERT(clam_match_chars("AA", "AA"),
                "`clam_match_chars` should match a matching string");
            ASSERT(clam_match_chars("AAZ", "AA"),
                "`clam_match_chars` should match a matching string even if the input is longer");
            ASSERT(!clam_match_chars("AA", "BAA"),
                "`clam_match_chars` should not match a non-matching string");
            ASSERT(!clam_match_chars("AAA", "ABA"),
                "`clam_match_chars` should not match a non-matching string that starts the same");
            ASSERT(!clam_match_chars("BA", "BAA"),
                "`clam_match_chars` should not match a non-matching string even if they start the same");

            ASSERT(clam_match_chars_to_end("AA", "AA"),
                "`clam_match_chars_to_end` should match a matching string");
            ASSERT(!clam_match_chars_to_end("AAZ", "AA"),
                "`clam_match_chars_to_end` should not match a matching string if the input is longer");
            ASSERT(!clam_match_chars_to_end("AA", "BAA"),
                "`clam_match_chars_to_end` should not match a non-matching string");

            ASSERT(clam_match_at_least_n_chars("ABC", 2, "ABC") == 3,
                "`clam_match_at_least_n_chars` should match all of the matching string if there is a full match");
            ASSERT(clam_match_at_least_n_chars("ABQ", 2, "ABC") == 2,
                "`clam_match_at_least_n_chars` should match the matching string if there is a minimum requred match");
            ASSERT(!clam_match_at_least_n_chars("ABQ", 3, "ABC"),
                "`clam_match_at_least_n_chars` should not match the matching string if there is no minimum requred match");

            int i;
            char s[1] = {32};
            disable_positive_asserts();
            for (i = 0; i < 10; i++) {
                    s[0] = 48 + i;
                    ASSERT(clam_match_numeric10_char(s),
                                    "`clam_match_numeric10_char` should match a base-10 numeric character");
            }
            enable_positive_asserts();
            ASSERT(!clam_match_numeric10_char("A"),
                "`clam_match_numeric10_char` should not match a non-base10 character");

            ASSERT(clam_match_unsigned_integer10("1234a") == strlen("1234"),
                "`clam_match_unsigned_integer10` should match an unsigned base-10 numeric string");
            ASSERT(!clam_match_unsigned_integer10("a1234a"),
                "`clam_match_unsigned_integer10` should not match a non unsigned base-10 numeric string");

            ASSERT(clam_match_signed_integer10("1234a") == strlen("1234"),
                "`clam_match_signed_integer10` should match an unsigned base-10 numeric string");
            ASSERT(clam_match_signed_integer10("+1234a") == strlen("+1234"),
                "`clam_match_signed_integer10` should match an unsigned base-10 numeric string with a positive sign");
            ASSERT(!clam_match_signed_integer10("++1234a"),
                "`clam_match_signed_integer10` should match an unsigned base-10 numeric string with more than one positive sign");
            ASSERT(clam_match_signed_integer10("-1234a") == strlen("-1234"),
                "`clam_match_signed_integer10` should match an unsigned base-10 numeric string with a negative sign");
            ASSERT(!clam_match_signed_integer10("--1234a"),
                "`clam_match_signed_integer10` should match an unsigned base-10 numeric string with more than one negative sign");
            ASSERT(!clam_match_signed_integer10("a1234a"),
                "`clam_match_signed_integer10` should not match a non unsigned base-10 numeric string");
            ASSERT(!clam_match_signed_integer10("+"),
                "`clam_match_signed_integer10` should not match a sign alone");
            ASSERT(!clam_match_signed_integer10("-"),
                "`clam_match_signed_integer10` should not match a sign alone");

            disable_positive_asserts();
            for (i = 0; i < 10; i++) {
                    s[0] = 48 + i;
                    ASSERT(clam_match_numeric16_char(s),
                                    "`clam_match_numeric16_char` should match a base-16 numeric character");
            }
            for (i = 0; i < 6; i++) {
                    s[0] = 65 + i;
                    ASSERT(clam_match_numeric16_char(s),
                                    "`clam_match_numeric16_char` should match a base-16 numeric character");
                    s[0] = 97 + i;
                    ASSERT(clam_match_numeric16_char(s),
                                    "`clam_match_numeric16_char` should match a base-16 numeric character");

            }
            enable_positive_asserts();

            ASSERT(!clam_match_numeric16_char("G"),
                "`clam_match_numeric16_char` should not match a non-base10 character");

            disable_positive_asserts();
            for (i = 65; i < 91; i++) {
                    s[0] = i;
                    ASSERT(clam_match_uppercase_char(s),
                                    "`clam_match_uppercase_char` should match an uppercase character");

            }
            enable_positive_asserts();
            ASSERT(!clam_match_uppercase_char("a"),
                                    "`clam_match_uppercase_char` should not match  a non-uppercase character");

            disable_positive_asserts();
            for (i = 97; i < 123; i++) {
                    s[0] = i;
                    ASSERT(clam_match_lowercase_char(s),
                                    "`clam_match_lowercase_char` should match an lowercase character");

            }
            enable_positive_asserts();
            ASSERT(!clam_match_lowercase_char("A"),
                                    "`clam_match_lowercase_char` should not match a non-lowercase character");

            disable_positive_asserts();
            for (i = 65; i < 91; i++) {
                    s[0] = i;
                    ASSERT(clam_match_alpha_char(s),
                                    "`clam_match_alpha_char` should match an alphabetic character");

            }
            for (i = 97; i < 123; i++) {
                    s[0] = i;
                    ASSERT(clam_match_alpha_char(s),
                                    "`clam_match_alpha_char` should match an alphabetic character");

            }
            enable_positive_asserts();
            ASSERT(!clam_match_alpha_char("1"),
                                    "`clam_match_alpha_char` should not match a non-alphabetic character");

            disable_positive_asserts();
            for (i = 0; i < 10; i++) {
                    s[0] = 48 + i;
                    ASSERT(clam_match_alphanumeric_char(s),
                                    "`clam_match_alphanumeric_char` should match a numeric character");
            }
            for (i = 65; i < 91; i++) {
                    s[0] = i;
                    ASSERT(clam_match_alphanumeric_char(s),
                                    "`clam_match_alphanumeric_char` should match an uppercase character");

            }
            for (i = 97; i < 123; i++) {
                    s[0] = i;
                    ASSERT(clam_match_alphanumeric_char(s),
                                    "`clam_match_alphanumeric_char` should match a lowercase character");

            }
            enable_positive_asserts();
            ASSERT(!clam_match_alphanumeric_char("-"),
                                    "`clam_match_alphanumeric_char` should not match a non-alphabetic character");

        }

        {
            printf("# POSIX-style matching\n");

            ASSERT(clam_match_posix_option("-a", "dacb1") == 2,
                "`clam_match_posix_option` should match with an option allowed");
            ASSERT(clam_match_posix_option("-azrf", "dacb1") == 2,
                "`clam_match_posix_option` should match with an option allowed and an arbitrary trailer");
            ASSERT(!clam_match_posix_option("-", "dacb1"),
                "`clam_match_posix_option` should not match if no option given");
            ASSERT(!clam_match_posix_option("-A", "dacb1"),
                "`clam_match_posix_option` should not match if no valid option given");

            ASSERT(clam_match_posix_flags("-abcd1", "dacb1"),
                "`clam_match_posix_flags` should match with all flags allowed");
            ASSERT(clam_match_posix_flags("-abcd", NULL),
                "`clam_match_posix_flags` should match with any flags allowed");
            ASSERT(!clam_match_posix_flags("-abcd", "dac"),
                "`clam_match_posix_flags` should not match if not all flags are allowed");
            ASSERT(!clam_match_posix_flags("-abcd_", "dacb"),
                "`clam_match_posix_flags` should not match for non-alphanumeric characters");

            ASSERT(clam_match_posix_long_option("--hello", "-hello") == strlen("--hello"),
                "`clam_match_posix_long_option` should match against an exact match");
            ASSERT(clam_match_posix_long_option("--hellop", "-hello") == strlen("--hello"),
                "`clam_match_posix_long_option` should match against a match");
            ASSERT(!clam_match_posix_long_option("--hellop", "-help"),
                "`clam_match_posix_long_option` should not match against a non-matching string");
            ASSERT(!clam_match_posix_long_option("--hellop", "hellop"),
                "`clam_match_posix_long_option` should not match against a non-matching string");
            ASSERT(!clam_match_posix_long_option("-", ""),
                "`clam_match_posix_long_option` should not match against an empty string");

            ASSERT(clam_match_posix_terminate_options("--"),
                "`clam_match_posix_terminate_options` should match if the string starts & ends with --");
            ASSERT(!clam_match_posix_terminate_options("--a"),
                "`clam_match_posix_terminate_options` should not match if the string starts but not ends with --");
            ASSERT(!clam_match_posix_terminate_options("b--a"),
                "`clam_match_posix_terminate_options` should not match if the string neither starts nor ends with --");
        }

        {
            printf("# Windows-style matching\n");

            ASSERT(clam_match_windows_switch("/a", "dacb1") == 2,
                "`clam_match_windows_switch` should match with an option allowed");
            ASSERT(clam_match_windows_switch("/azrf", "dacb1") == 2,
                "`clam_match_windows_switch` should match with an option allowed and an arbitrary trailer");
            ASSERT(!clam_match_windows_switch("/", "dacb1"),
                "`clam_match_windows_switch` should not match if no option given");
            ASSERT(!clam_match_windows_switch("/A", "dacb1"),
                "`clam_match_windows_switch` should not match if no valid option given");

            ASSERT(clam_match_windows_long_switch("/hello", "hello") == strlen("/hello"),
                "`clam_match_windows_long_switch` should match against an exact match");
            ASSERT(clam_match_windows_long_switch("/hellop", "hello") == strlen("/hello"),
                "`clam_match_windows_long_switch` should match against a match");
            ASSERT(!clam_match_windows_long_switch("/hellop", "help"),
                "`clam_match_windows_long_switch` should not match against a non-matching string");
            ASSERT(!clam_match_windows_long_switch("/hellop", ""),
                "`clam_match_windows_long_switch` should not match against an empty string");
        }

        return error_code;
}
