/**
 * \mainpage
 *
 * ## What is it?
 *
 * CLAM is a single-header C library that gives simple tools to construct
 * command line parsing with explicit control flow.
 *
 * ### Example
 *
 * \code{.c}
 * for (int a = 0; a < argc; a++) {
 *   const char *arg = argv[a];
 *   clam_match_result_t i = 0;
 *   bool longopt;
 *
 *   if ((longopt = true, i = clam_match_posix_long_option(arg, "name")) ||
 *       (longopt = true, i = clam_match_posix_long_option(arg, "-name")) ||
 *       (longopt = false, i = clam_match_posix_long_option(arg, "n"))) {
 *       if (longopt && !clam_match_end(arg + i) && !clam_match_char(arg + i, '=')) {
 *         printf("invalid trailer %s at %lu in %s\n", arg + i, i, arg);
 *       }
 *       arg += i;
 *       if (clam_match_end(arg)) {
 *          arg = argv[++a];
 *          if (a == argc) {
 *             printf("name requires an argument\n");
 *             return 1;
 *          }
 *
 *       printf("name is %s\n", arg);
 *       continue;
 *   }
 *
 *   // ...
 *
 * }
 * \endcode
 *
 * ## Why? (Rationale)
 *
 * Existing command line argument handling libraries (such as getopt, argp,
 * dropt) provide a simple interface to define options, flags, commands and
 * then primarily act as "blackboxes" mapping command line arguments to those
 * definitions.
 *
 * CLAM suggests a somewhat different approach. It gives you tools for
 * convenient command line arguments processing but leaves the control flow in
 * your hands, giving you more flexibility.
 *
 * "Blackbox"-style libraries are effectively runtime interpreters of
 * table-based programs (lists of options) which leads to indirect code path.
 * CLAM makes command line arguments processing straighforward, easy to follow
 * and inifinitely customizable.
 *
 * ## License
 *
 * CLAM is distributed under the terms of MIT license (see \ref license).
 *
 * ## FAQ
 *
 * ### Which C standard does it conform to?
 *
 * CLAM's baseline expectation is C99. It *may* use C11 if supported by the compiler.
 *
 * ### Why a header file library?
 *
 * Ease of distribution.
 *
 */

/**
 * \page license License
 *
 * # MIT License
 *
 * Copyright (c) 2020, CLAM Authors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * \page acsl ACSL
 *
 * CLAM uses ACSL (https://frama-c.com/acsl.html) to specify and verify its
 * properties.
 *
 * The goal is to ensure the correctness of the library and provide meaningful,
 * deterministic description of behaviours and pre-/post- conditions.
 *
 * \section acsl-known-issues Known Issues
 *
 * \li Frama-C/WP don't handle string literals properly (they are not `valid_read_string`),
 *     so we define these using \code const char value[LEN] = "...";\endcode to allow validation to occur.
 *     These cases are marked with a \c FRAMA-WP-SL marker.
 *
 *
 *
 */

#ifndef CLAM_H
#define CLAM_H

#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdint.h>

#ifdef FRAMA_C_STRING
#define CLAM_USING_FRAME
#endif

#ifndef CLAM_API
/**
 * CLAM API function declaration specifier
 *
 * Can be redefined externally.
 */
#define CLAM_API static inline
#endif

/*@
  @ axiomatic StrlenAxioms {
  @
  @   // Axiomatize the following fact:
  @   // Length of any valid string is less or equal to UINTPTR_MAX.
  @   //
  @   // What's the largest valid string we can imagine? Let's assume it starts
  @   // at the offset of `0` and takes all addressable memory.
  @   //
  @   // Let's  also assume UINTPTR_MAX to be 8 (because why not?).
  @   //
  @   // c c c c c c c c 0
  @   // _ _ _ _ _ _ _ _ _
  @   // 0 1 2 3 4 5 6 7 8
  @   //
  @   // As we can visually confirm, the maximum length of the string in this case is 8
  @   // (which is our UINTPTR_MAX)..
  @   //
  @   axiom strlen_less_or_equal_uintptr_max:
  @     \forall char *s; strlen(s) <= UINTPTR_MAX;
  @ }
  @*/

/**
 * \defgroup matchers Matchers
 *
 * Matching strings
 *
 * CLAM's foundational block is matchers. They are simple functions of the that
 * take `const char *` input and zero or more arguments and return \ref
 * clam_match_result_t.  This return type indicates how much of the input was
 * successfully matched by the matcher.
 *
 * The idea behind this is to combine these functions (matchers) into a parsing
 * program with an explicit flow control.
 *
 * @{
 */

/**
 * Type returned by all `clam_match_XXX` functions.
 *
 * Indicates the number of characters successfully matched by the matcher.
 *
 * If a matcher needs to be able to indicate an error, it should supply an
 * parameter that will allow the matcher to report the error.
 *
 * For example:
 *
 * \code{.h}
 * clam_match_result_t dummy_matcher(const char *restrict input, int *errno);
 * \endcode
 *
 */
typedef uintptr_t clam_match_result_t;

/**
 * \defgroup character-matchers Basic character matchers
 *
 * Matching against characters or sets of characters
 *
 * @{
 */

/**
 * Matches `input`'s first character if it matches `c`
 *
 */
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result \in (0..1);
  @ ensures \result == 1 ==> input[0] == c;
  @ ensures \result == 0 ==> input[0] != c;
  @*/
CLAM_API clam_match_result_t
         clam_match_char(
           const char * restrict input,
           char                  c
         )
{
         return input[0] == c;
}

/**
 * Matches `input`'s end of the (null-terminated) string
 */
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result \in (0..1);
  @ behavior empty:
  @   assumes strlen(input) == 0;
  @   ensures \result == 1;
  @ behavior nonempty:
  @   assumes strlen(input) > 0;
  @   ensures \result == 0;
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_end(
           const char * restrict input
         )
{
         return clam_match_char(input, 0);
}

/**
 * Matches `input`'s first character if it matches any of the characters in `chars`
 *
 * If `input` is shorter than one character, nothing is matched.
 *
 * If `chars` equals `NULL` then any characters are allowed (except a null character).
 *
 * If `chars` equals an empty string than no character is allowed.
 */
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result \in (0..1);
  @ behavior empty:
  @   assumes strlen(input) == 0;
  @   ensures \result == 0;
  @ behavior no_chars:
  @   assumes chars == \null;
  @   assumes strlen(input) > 0;
  @   ensures \result == 1;
  @ behavior chars:
  @   assumes chars != \null;
  @   assumes strlen(input) > 0;
  @   requires valid_read_string(chars);
  @   ensures \result == 1 ==> \exists integer i;
  @                            0 <= i < strlen(chars) ==>
  @                            chars[i] == input[0];
  @   ensures \result == 0 ==> strlen(input) == 0 ||
  @                            \forall integer i;
  @                              0 <= i < strlen(chars) ==>
  @                              chars[i] != input[0];
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_anychar(
           const char * restrict input,
           const char *          chars
         )
{
         if (clam_match_end(input)) {
                 return 0;
         } else if (chars == NULL) {
                 return 1;
         } else {
                 clam_match_result_t i = 0;
                 clam_match_result_t result = 0;
                 /*@
                   @ loop invariant 0 <= i <= strlen(chars);
                   @ loop invariant \forall integer j; 0 <= j < i ==> input[0] != chars[j];
                   @ loop invariant result \in (0..1);
                   @ loop assigns i, result;
                   @*/
                 while (!result &&
                        !clam_match_end(chars + i)) {
                        result = clam_match_char(input, chars[i]);
                        if (result) {
                                break;
                        }
                        i++;
                 }

                 return result;
         }
}

/**
 * Matches at least `n` characters of `input` if they match the
 * first `n` characters of `chars` and more if the following characters
 * still match their counterparts in `chars`
 */
/*@
  @ requires valid_read_string(input);
  @ requires valid_read_string(chars);
  @ requires n <= UINTPTR_MAX;
  @ assigns \nothing;
  @ ensures \result \in (n..strlen(input)) || \result == 0;
  @ behavior too_small:
  @   assumes 0 <= strlen(input) < n;
  @   ensures \result == 0;
  @ behavior no_match:
  @   assumes strlen(input) >= n;
  @   assumes strncmp(input, chars, n) != 0;
  @   ensures \result == 0;
  @ behavior match:
  @   assumes strlen(input) >= n;
  @   assumes strncmp(input, chars, n) == 0;
  @   ensures \result >= n ==> strlen(chars) >= n;
  @   ensures \result > n ==> strncmp(input, chars, \result) == 0;
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_at_least_n_chars(
           const char   * restrict input,
           size_t                  n,
           const char   *          chars
         )
{
         clam_match_result_t i = 0;

         /*@
           @ loop invariant 0 <= i <= strlen(input);
           @ loop invariant 0 <= i <= strlen(chars);
           @ loop invariant \forall integer j; 0 <= j < i ==> input[j] == chars[j];
           @ loop assigns i;
           @*/
         while (!clam_match_end(input + i) && !clam_match_end(chars + i) &&
                clam_match_char(input + i, chars[i])) {
                 i++;
         }

         return (i >= n) ? i : 0;
}

/**
 * Matches `input` if it matches `chars`
 */
/*@
  @ requires valid_read_string(input);
  @ requires valid_read_string(chars);
  @ ensures \result == 0 || \result == strlen(chars);
  @ assigns \nothing;
  @ behavior no_chars:
  @   assumes strlen(chars) == 0 || strlen(input) == 0;
  @   ensures \result == 0;
  @ behavior right_size:
  @   assumes strlen(chars) > 0;
  @   assumes strlen(input) >= strlen(chars);
  @   ensures \result == 0 ==> strncmp(input, chars, strlen(chars)) != 0;
  @   ensures \result == strlen(chars) ==> strncmp(input, chars, strlen(chars)) == 0;
  @ behavior too_small:
  @   assumes strlen(chars) > 0;
  @   assumes 0 < strlen(input) < strlen(chars);
  @   ensures \result == 0;
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_chars(
           const char * restrict input,
           const char *          chars
         )
{
         clam_match_result_t i = 0;

         /*@
           @ loop invariant 0 <= i <= strlen(input);
           @ loop invariant 0 <= i <= strlen(chars);
           @ loop invariant \forall integer j; 0 <= j < i ==> input[j] == chars[j];
           @ loop assigns i;
           @*/
         while (!clam_match_end(input + i) && !clam_match_end(chars + i) &&
                clam_match_char(input + i, chars[i])) {
                 i++;
         }

         return clam_match_end(chars + i) ? i : 0;
}

/**
 * Matches `input` if it matches and terminates with `chars`
 */
/*@
  @ requires valid_read_string(input);
  @ requires valid_read_string(chars);
  @ assigns \nothing;
  @ ensures \result == 0 || \result == strlen(chars);
  @ behavior no_chars:
  @   assumes strlen(chars) == 0 || strlen(input) == 0;
  @   ensures \result == 0;
  @ behavior too_small:
  @   assumes strlen(chars) > 0;
  @   assumes 0 < strlen(input) < strlen(chars);
  @   ensures \result == 0;
  @ behavior too_big:
  @   assumes strlen(chars) > 0;
  @   assumes strlen(input) > strlen(chars);
  @   ensures \result == 0;
  @ behavior match:
  @   assumes strlen(chars) > 0;
  @   assumes strlen(input) == strlen(chars);
  @   assumes strncmp(input, chars, strlen(chars)) == 0;
  @   ensures \result == strlen(chars);
  @ behavior no_match:
  @   assumes strlen(chars) > 0;
  @   assumes strlen(input) == strlen(chars);
  @   assumes strncmp(input, chars, strlen(chars)) != 0;
  @   ensures \result == 0;
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_chars_to_end(
           const char * restrict input,
           const char *          chars
         )
{
         clam_match_result_t i = 0;

         if (!(i = clam_match_chars(input, chars))) {
                 return 0;
         }

         return clam_match_end(input + i) ? i : 0;
}

/*@
  @ axiomatic CharacterRanges {
  @  predicate is_numeric10_char(char x) = '0' <= x <= '9';
  @  predicate is_numeric16_char(char x) = '0' <= x <= '9' || 'a' <= x <= 'f' || 'A' <= x <= 'F';
  @  predicate is_upper_char(char x) = 'A' <= x <= 'Z';
  @  predicate is_lower_char(char x) = 'a' <= x <= 'z';
  @  predicate is_alpha_char(char x) = is_upper_char(x) || is_lower_char(x);
  @  predicate is_alphanumeric_char(char x) = is_alpha_char(x) || is_numeric10_char(x);
  @ }
  @*/

/**
 * Matches `input` if it matches one base-10 numeric character (0-9)
 */
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result \in (0..1);
  @ ensures \result == 1 ==> is_numeric10_char(input[0]);
  @ ensures \result == 0 ==> !is_numeric10_char(input[0]);
  @*/
CLAM_API clam_match_result_t
         clam_match_numeric10_char(
           const char * restrict input
         )
{
         return input[0] >= '0' && input[0] <= '9';
}

/**
 * Matches `input` if it matches an unsigned base-10 integer
 */
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result \in (0..strlen(input));
  @ behavior empty:
  @   assumes strlen(input) == 0;
  @   ensures \result == 0;
  @ behavior matching:
  @   assumes strlen(input) > 0;
  @   ensures \exists integer k; 0 <= k < strlen(input) && k == \result ==> is_numeric10_char(input[k]);   
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_unsigned_integer10(
           const char * restrict input
         )
{
        clam_match_result_t i = 0;
        /*@
          @ loop assigns i;
          @ loop invariant 0 <= i <= strlen(input);
          @ loop invariant \forall integer j; 0 <= j < i ==> is_numeric10_char(input[j]);
          @*/
        while (clam_match_numeric10_char(input + i)) {
                i++;
        }
        return i;
}

/**
 * Matches `input` if it matches an signed base-10 integer.
 *
 * Accepts optional '-' and '+' signs.
 */
const char clam__signs[3] = "-+"; // FRAMA-WP-SL
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result \in (0..strlen(input));
  @ behavior empty:
  @   assumes strlen(input) == 0;
  @   ensures \result == 0;
  @ behavior sign_only:
  @   assumes strlen(input) == 1;
  @   assumes input[0] == '+' || input[0] == '-';
  @   ensures \result == 0;
  @ behavior unsigned:
  @   assumes strlen(input) > 0;
  @   assumes !(input[0] == '+' || input[0] == '-');
  @   ensures \exists integer k; 0 <= k < strlen(input) && k == \result ==> is_numeric10_char(input[k]);
  @ behavior signed:
  @   assumes strlen(input) > 1;
  @   assumes input[0] == '+' || input[0] == '-';
  @   ensures \exists integer k; 0 < k < strlen(input) && k == \result ==> is_numeric10_char(input[k]);
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_signed_integer10(
           const char * restrict input
         )
{
         clam_match_result_t sign = clam_match_anychar(input, clam__signs);

         clam_match_result_t number = clam_match_unsigned_integer10(input + sign);
         return number ? sign + number : 0;
}

/**
 * Matches `input` if it matches one base-16 numeric character (0-9a-fA-F)
 */
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result \in (0..1);
  @ ensures \result == 1 ==> is_numeric16_char(input[0]);
  @ ensures \result == 0 ==> !is_numeric16_char(input[0]);
  @*/
CLAM_API clam_match_result_t
         clam_match_numeric16_char(
           const char * restrict input
         )
{
         return input[0] >= '0' && input[0] <= '9' ||
                input[0] >= 'A' && input[0] <= 'F' ||
                input[0] >= 'a' && input[0] <= 'f';
}

/**
 * Matches `input` if it matches uppercase alphabetic character
 */
/*@
  @ requires valid_read_string(input);
  @ ensures \result \in (0..1);
  @ assigns \nothing;
  @ behavior matches:
  @   assumes is_upper_char(input[0]);
  @   ensures \result == 1;
  @ behavior doesnt_match:
  @   assumes !is_upper_char(input[0]);
  @   ensures \result == 0;
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_uppercase_char(
           const char * restrict input
         )
{
         return input[0] >= 'A' && input[0] <= 'Z';
}

/**
 * Matches `input` if it matches lowercase alphabetic character
 */
/*@
  @ requires valid_read_string(input);
  @ ensures \result \in (0..1);
  @ assigns \nothing;
  @ behavior matches:
  @   assumes is_lower_char(input[0]);
  @   ensures \result == 1;
  @ behavior doesnt_match:
  @   assumes !is_lower_char(input[0]);
  @   ensures \result == 0;
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_lowercase_char(
           const char * restrict input
         )
{
         return input[0] >= 'a' && input[0] <= 'z';
}

/**
 * Matches `input` if it matches alphabetic character
 */
/*@
  @ requires valid_read_string(input);
  @ ensures \result \in (0..1);
  @ assigns \nothing;
  @ behavior matches:
  @   assumes is_alpha_char(input[0]);
  @   ensures \result == 1;
  @ behavior doesnt_match:
  @   assumes !is_alpha_char(input[0]);
  @   ensures \result == 0;
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
CLAM_API clam_match_result_t
         clam_match_alpha_char(
             const char * restrict input
         )
{
         return clam_match_lowercase_char(input) || clam_match_uppercase_char(input);
}

/**
 * Matches `input` if it matches alphanumeric (base-10) character
 */
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result \in (0..1);
  @ ensures \result == 1 ==> is_alphanumeric_char(input[0]);
  @ ensures \result == 0 ==> !is_alphanumeric_char(input[0]);
  @*/
CLAM_API clam_match_result_t
         clam_match_alphanumeric_char(
             const char * restrict input
         )
{
         return clam_match_alpha_char(input) || clam_match_numeric10_char(input);
}

/**@}*/

/**
 * \defgroup posix-matchers POSIX Matchers
 *
 * POSIX-style arguments
 *
 * @{
 */

/**
 * Matches `input` if it matches one of the alphanumeric, allowed single-character POSIX options
 * in `allowed_options`.
 *
 * If `allowed_options` is `NULL` then any alphanumeric single-character POSIX option
 * is allowed
 *
 */
/*@
  @ requires valid_read_string(input);
  @ requires allowed_options == \null || valid_read_string(allowed_options);
  @ assigns \nothing;
  @ ensures \result == 0 || \result == 2;
  @ ensures \result == 2 ==> input[0] == '-' && is_alphanumeric_char(input[1]);
  @ ensures \result == 2 && valid_read_string(allowed_options) ==> \exists integer k;
  @                         0 < k <= strlen(allowed_options) ==> allowed_options[k] == input[1];
  @*/
CLAM_API clam_match_result_t
         clam_match_posix_option(
           const char * restrict input,
           const char *          allowed_options
         )
{
         return clam_match_char(input, '-') && clam_match_alphanumeric_char(input + 1) &&
                clam_match_anychar(input + 1, allowed_options) ? 2 : 0;
}


/**
 * Matches `input` if it matches a dash (`-`) followed by `option`.
 */
/*@
  @ requires valid_read_string(input);
  @ requires valid_read_string(option);
  @ assigns \nothing;
  @ ensures \result == 0 || \result == strlen(option) + 1;
  @ ensures \result > 0 ==> input[0] == '-';
  @ ensures \result == 1 ==> strlen(option) == 0;
  @ ensures \result > 1 ==> strlen(option) > 0;
  @ ensures \result > 1 ==> \forall integer k;
  @                            0 <= k < strlen(option) ==>
  @                                input[k+1] == option[k];
  @*/
CLAM_API clam_match_result_t
         clam_match_posix_long_option(
           const char * restrict input,
           const char * restrict option
         )
{
         clam_match_result_t dash = clam_match_char(input, '-');
         if (dash == 0) {
                 return 0;
         }
         clam_match_result_t opt = clam_match_chars(input + dash, option);
         return opt ? dash + opt : 0;
}

/**
 * Matches `input` if it contains a dash (`-`) followed by any number
 * of alphanumeric characters present in `allowed_options`.
 *
 * If `allowed_options` is `NULL` then any alphanumeric single-character POSIX option
 * is allowed
 *
 */
/*@
  @ requires valid_read_string(input);
  @ requires allowed_options == \null || valid_read_string(allowed_options);
  @ assigns \nothing;
  @ ensures \result == 0 || \result \in (2..strlen(input));
  @ ensures \result >= 2 ==> input[0] == '-';
  @ ensures \result >= 2 && valid_read_string(allowed_options) ==>
  @       \exists integer k; \forall integer i;
  @           0 <= k < strlen(allowed_options) && 1 <= i < \result ==>
  @              allowed_options[k] == input[i] && is_alphanumeric_char(input[i]);
  @
  @*/
CLAM_API clam_match_result_t
         clam_match_posix_flags(
           const char * restrict input,
           const char *          allowed_options
         )
{
         clam_match_result_t i;
         if (!(i = clam_match_char(input, '-'))) {
                 return 0;
         }
         /*@
           @ loop invariant 0 <= i <= strlen(input);
           @ loop assigns i;
           @*/
         while (!clam_match_end(input + i)) {
                 if (!(clam_match_alphanumeric_char(input + i) &&
                       clam_match_anychar(input + i, allowed_options))) {
                         return 0;
                 }
                 i++;
         }
         return i > 1 ? i : 0;
}

/**
 * Matches `input` if it is terminated by two dashes (`--`).
 */
const char clam__dashes[3] = "--"; // FRAMA-WP-SL
/*@
  @ requires valid_read_string(input);
  @ assigns \nothing;
  @ ensures \result == 0 || \result == 2;
  @ ensures \result == 0 ==> strlen(input) != 2 || input[0] != '-' || input[1] != '-';
  @ ensures \result == 1 ==> strlen(input) == 2 || input[0] == '-' || input[1] == '-';
  @*/
CLAM_API clam_match_result_t
         clam_match_posix_terminate_options(
           const char * restrict input
         )
{
         return clam_match_chars_to_end(input, clam__dashes);
}


/**@}*/

/**
 * \defgroup windows-matchers Windows Matchers
 *
 * Windows-style arguments
 *
 * @{
 */

/**
 * Matches `input` if it matches one of the allowed single-character
 * Windows-style switches in `allowed_switches`.
 *
 * If `allowed_switches` is `NULL` then any alphanumeric single-character POSIX
 * option is allowed
 *
 */
/*@
  @ requires valid_read_string(input);
  @ requires allowed_switches == \null || valid_read_string(allowed_switches);
  @ assigns \nothing;
  @ ensures \result == 0 || \result == 2;
  @ ensures \result == 2 ==> input[0] == '/' && is_alphanumeric_char(input[1]);
  @*/
CLAM_API clam_match_result_t
         clam_match_windows_switch(
           const char * restrict input,
           const char *          allowed_switches
         )
{
         return clam_match_char(input, '/') &&
                clam_match_alphanumeric_char(input + 1) &&
                clam_match_anychar(input + 1, allowed_switches) ? 2 : 0;
}

/**
 * Matches `input` if it contains forward slash followed by `option`
 */
/*@
  @ requires valid_read_string(input);
  @ requires valid_read_string(switch_s);
  @ assigns \nothing;
  @ ensures \result == 0 || \result == strlen(switch_s) + 1;
  @ ensures \result > 0 ==> input[0] == '/';
  @ ensures \result == 1 ==> strlen(switch_s) == 0;
  @ ensures \result > 1 ==> strlen(switch_s) > 0;
  @ ensures \result > 1 ==> \forall integer k;
  @                            0 <= k < strlen(switch_s) ==>
  @                                input[k+1] == switch_s[k];
  @*/
CLAM_API clam_match_result_t
         clam_match_windows_long_switch(
           const char * restrict input,
           const char * restrict switch_s
         )
{
         clam_match_result_t slash = clam_match_char(input, '/');
         if (slash == 0) {
                 return 0;
         }
         clam_match_result_t swtch = clam_match_chars(input + slash, switch_s);
         return swtch ? slash + swtch : 0;
}

/**@}*/

/**@}*/

#endif // CLAM_H
/** @file */

