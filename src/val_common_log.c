/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <limits.h>
#include <stdint.h>

#include "val_common_log.h"
#include "val_common_framework.h"

static void val_putc(char *c)
{
    pal_uart_putc(*c);
}

enum { LOG_MAX_STRING_LENGTH = 90 };

/* Keep fields aligned */
/* clang-format off */
struct format_flags {
    bool minus  : 1;
    bool plus   : 1;
    bool space  : 1;
    bool alt    : 1;
    bool zero   : 1;
    bool upper  : 1;
    bool neg    : 1;
};
/* clang-format on */

enum format_base {
    base2 = 2,
    base8 = 8,
    base10 = 10,
    base16 = 16,
};

enum format_length {
    length_char,
    length_short,
    length_int,
    length_long,
    length_long_long,
    length_intmax,
    length_size,
    length_ptrdiff,
};

#ifndef STATIC_ASSERT_CHECKS
static_assert(sizeof(char) == sizeof(uint8_t),
          "log expects char to be 8 bits wide");
static_assert(sizeof(short) == sizeof(uint16_t),
          "log expects short to be 16 bits wide");
static_assert(sizeof(int) == sizeof(uint32_t),
          "log expects int to be 32 bits wide");
static_assert(sizeof(long) == sizeof(uint64_t),
          "log expects long to be 64 bits wide");
static_assert(sizeof(long long) == sizeof(uint64_t),
          "log expects long long to be 64 bits wide");
static_assert(sizeof(intmax_t) == sizeof(uint64_t),
          "log expects intmax_t to be 64 bits wide");
static_assert(sizeof(size_t) == sizeof(uint64_t),
          "log expects size_t to be 64 bits wide");
static_assert(sizeof(ptrdiff_t) == sizeof(uint64_t),
          "log expects ptrdiff_t to be 64 bits wide");
#endif

/*
 * These global variables for the log buffer are not static because a test needs
 * to access them directly.
 */
size_t log_buffer_offset;
char log_buffer[LOG_BUFFER_SIZE];
static const char null_log_message[] = "(null)";

/**
 *   @brief    - Stores a character in a log buffer and outputs it via 'val_putc'
 *   @param    - c  : Input Character
 *   @return   - Sends the character using 'val_putc'
 **/

static void log_putchar(char c)
{
    log_buffer[log_buffer_offset] = c;
    log_buffer_offset = (log_buffer_offset + 1) % LOG_BUFFER_SIZE;

    val_putc(&c);
}

/**
 *   @brief    - Determines length of a string up to a maximum limit
 *   @param    - str    : Input String
 *             - strsz  : Maximum characters to check for string's length
 *   @return   - The length of the null-terminated byte string `str`
 **/

static size_t log_strnlen_s(const char *str, size_t strsz)
{
    if (str == NULL) {
        return 0;
    }

    for (size_t i = 0; i < strsz; ++i) {
        if (str[i] == '\0') {
            return i;
        }
    }

    /* NULL character not found. */
    return strsz;
}

/**
 *   @brief    - Prints a literal string (i.e. '%' is not interpreted specially) to the debug log
 *   @param    - str    : Input literal String
 *   @return   - Number of characters written
 **/

static size_t print_raw_string(const char *str)
{
    const char *c = str;

    for (; *c != '\0'; c++) {
        log_putchar(*c);
    }

    return (size_t)(c - str);
}

/**
 *   @brief    - Prints a formatted string to the debug log
 *   @param    - str        : The full String
 *             - suffix     : Pointer within str that indicates where suffix begins
 *             - min_width  : Minimum width
 *             - flags      : Whether to align to left or right
 *             - fill       : The fill character
 *   @return   - Number of characters written
 **/

static size_t print_string(const char *str, const char *suffix,
               int min_width, struct format_flags *flags,
               char fill)
{
    size_t chars_written = 0;
    size_t len = (size_t)(suffix - str);

    /* Print the string up to the beginning of the suffix. */
    while (str != suffix) {
        chars_written++;
        log_putchar(*str++);
    }

    if (flags->minus) {
        /* Left-aligned. Print suffix, then print padding if needed. */
        len += print_raw_string(suffix);
        while (len < (size_t)min_width) {
            chars_written++;
            log_putchar(' ');
            len++;
        }
        return chars_written;
    }

    /* Fill until we reach the desired length. */
    len += log_strnlen_s(suffix, LOG_MAX_STRING_LENGTH);
    while (len < (size_t)min_width) {
        chars_written++;
        log_putchar(fill);
        len++;
    }

    /* Now print the rest of the string. */
    chars_written += print_raw_string(suffix);
    return chars_written;
}

/**
 *   @brief    - Prints an integer to the debug log
 *   @param    - value      : Integer to be formatted and printed
 *             - base       : Base of the integer
 *             - min_width  : Minimum width of the integer
 *             - flags      : Printf-style flags
 *   @return   - Number of characters written
 **/

static size_t print_int(size_t value, enum format_base base, int min_width,
            struct format_flags *flags)
{
    static const char *digits_lower = "0123456789abcdefxb";
    static const char *digits_upper = "0123456789ABCDEFXB";
    const char *digits = flags->upper ? digits_upper : digits_lower;
    char buf[LOG_MAX_STRING_LENGTH];
    char *ptr = &buf[sizeof(buf) - 1];
    char *num;
    *ptr = '\0';
    do {
        --ptr;
        *ptr = digits[value % base];
        value /= base;
    } while (value);

    /* Num stores where the actual number begins. */
    num = ptr;

    /* Add prefix if requested. */
    if (flags->alt) {
        switch (base) {
        case base16:
            ptr -= 2;
            ptr[0] = '0';
            ptr[1] = digits[16];
            break;

        case base2:
            ptr -= 2;
            ptr[0] = '0';
            ptr[1] = digits[17];
            break;

        case base8:
            ptr--;
            *ptr = '0';
            break;

        case base10:
            /* do nothing */
            break;
        }
    }

    /* Add sign if requested. */
    if (flags->neg) {
        *--ptr = '-';
    } else if (flags->plus) {
        *--ptr = '+';
    } else if (flags->space) {
        *--ptr = ' ';
    }
    return print_string(ptr, num, min_width, flags, flags->zero ? '0' : ' ');
}

/**
 *   @brief    - Parses the optional flags field of a printf-style format
 *   @param    - fmt       : Input format String
 *             - flags     : Store status of formatting flags from input string
 *   @return   - A pointer to the first non-flag character in the string
 **/

static const char *parse_flags(const char *fmt, struct format_flags *flags)
{
    for (;; fmt++) {
        switch (*fmt) {
        case '-':
            flags->minus = true;
            break;

        case '+':
            flags->plus = true;
            break;

        case ' ':
            flags->space = true;
            break;

        case '#':
            flags->alt = true;
            break;

        case '0':
            flags->zero = true;
            break;

        default:
            return fmt;
        }
    }
}

/**
 *   @brief    - Parses the optional length modifier field of a printf-style format
 *   @param    - fmt       : Input String
 *             - length    : Indicates the size of data-type being formatted
 *   @return   - A pointer to the first non-length modifier character in the string
 **/

static const char *parse_length_modifier(const char *fmt,
                     enum format_length *length)
{
    switch (*fmt) {
    case 'h':
        fmt++;
        if (*fmt == 'h') {
            fmt++;
            *length = length_char;
        } else {
            *length = length_short;
        }
        break;
    case 'l':
        fmt++;
        if (*fmt == 'l') {
            fmt++;
            *length = length_long_long;
        } else {
            *length = length_long;
        }
        break;
    case 'j':
        fmt++;
        *length = length_intmax;
        break;
    case 'z':
        fmt++;
        *length = length_size;
        break;
    case 't':
        fmt++;
        *length = length_ptrdiff;
        break;
    default:
        *length = length_int;
        break;
    }

    return fmt;
}

/**
 *   @brief    - Parses the optional minimum width field of a printf-style format
 *   @param    - fmt       : Input String
 *             - args      : List of additional arguments
 *             - flags     : Indicates if the value is negative
 *             - min_width : Integer where parsed minimum width is stored
 *   @return   - A pointer to the first non-digit character in the string
 **/

static const char *parse_min_width(const char *fmt, va_list args,
                   struct format_flags *flags, int *min_width)
{
    int width = 0;
    const int max_tens = INT_MAX / 10;
    const int max_units = INT_MAX % 10;

    /* Read minimum width from arguments. */
    if (*fmt == '*') {
        fmt++;
        int arg_width = va_arg(args, int);
        if (arg_width < 0) {
            flags->minus = true;
            if (arg_width == INT_MIN) {
                width = INT_MAX;
            } else {
                width = -arg_width;
            }
        } else {
            width = arg_width;
        }
    } else {
        for (; *fmt >= '0' && *fmt <= '9'; fmt++) {
            int digit = *fmt - '0';
            if (width > max_tens || (width == max_tens && digit > max_units)) {
                width = INT_MAX;
            } else {
                width = (width * 10) + digit;
            }
        }
    }

    *min_width = width;

    return fmt;
}

/**
 *   @brief    - Reinterpret an unsigned 64-bit integer as a potentially shorter unsigned
 *               integer according to the length modifier.
 *   @param    - length    : Specifies target-bit width of integer
 *             - value     : Input unsigned integer
 *   @return   - An unsigned integer suitable for passing to `print_int`
 **/

static uint64_t reinterpret_unsigned_int(enum format_length length, uint64_t value)
{
    switch (length) {
    case length_char:
        return (uint8_t)value;
    case length_short:
        return (uint16_t)value;
    case length_int:
        return (uint32_t)value;
    case length_long:
    case length_long_long:
    case length_intmax:
    case length_size:
    case length_ptrdiff:
        return value;
    }
    return 0;
}

/**
 *   @brief    - Reinterpret an unsigned 64-bit integer as a potentially shorter signed
 *               integer according to the length modifier.
 *   @param    - length    : Specifies width of the integer
 *             - value     : Input unsigned integer value
 *             - flags     : Indicates if the value is negative
 *   @return   - An *unsigned* integer suitable for passing to `print_int`
 **/

static uint64_t reinterpret_signed_int(enum format_length length, uint64_t value,
                struct format_flags *flags)
{
    int64_t signed_value = (int64_t)reinterpret_unsigned_int(length, value);

    switch (length) {
    case length_char:
        if ((int8_t)signed_value < 0) {
            flags->neg = true;
            signed_value = (-signed_value) & 0xFF;
        }
        break;
    case length_short:
        if ((int16_t)signed_value < 0) {
            flags->neg = true;
            signed_value = (-signed_value) & 0xFFFF;
        }
        break;
    case length_int:
        if ((int32_t)signed_value < 0) {
            flags->neg = true;
            signed_value = (-signed_value) & 0xFFFFFFFF;
        }
        break;
    case length_long:
    case length_long_long:
    case length_intmax:
    case length_size:
    case length_ptrdiff:
        if ((int64_t)signed_value < 0) {
            flags->neg = true;
            if ((int64_t)signed_value == INT64_MIN) {
                return 1ULL << 63;
            }
            signed_value = -signed_value;
        }
        break;
    }

    return (uint64_t)signed_value;
}

static uint64_t read_signed_argument(va_list *args, enum format_length length)
{
    switch (length) {
    case length_char:
    case length_short:
    case length_int:
        return (uint64_t)va_arg(*args, int);
    case length_long:
        return (uint64_t)va_arg(*args, long);
    case length_long_long:
        return (uint64_t)va_arg(*args, long long);
    case length_intmax:
        return (uint64_t)va_arg(*args, intmax_t);
    case length_size:
        return (uint64_t)va_arg(*args, size_t);
    case length_ptrdiff:
        return (uint64_t)va_arg(*args, ptrdiff_t);
    }
    return 0;
}

static uint64_t read_unsigned_argument(va_list *args, enum format_length length)
{
    switch (length) {
    case length_char:
    case length_short: {
        int value = va_arg(*args, int);
        return (uint64_t)(unsigned int)value;
    }
    case length_int:
        return (uint64_t)va_arg(*args, unsigned int);
    case length_long:
        return (uint64_t)va_arg(*args, unsigned long);
    case length_long_long:
        return (uint64_t)va_arg(*args, unsigned long long);
    case length_intmax:
        return (uint64_t)va_arg(*args, uintmax_t);
    case length_size:
        return (uint64_t)va_arg(*args, size_t);
    case length_ptrdiff:
        return (uint64_t)va_arg(*args, ptrdiff_t);
    }
    return 0;
}

/**
 *   @brief    - This function parses and formats a string according to specified format specifiers
 *   @param    - fmt      : Input String
 *             - args     : Arguments are passed as a va_list
 *   @return   - Number of characters written, or `-1` if format string is invalid
 **/

static size_t val_log(const char *fmt, va_list args)
{
    size_t chars_written = 0;

    while (*fmt != '\0') {
        switch (*fmt) {
        default:
            chars_written++;
            log_putchar(*fmt);
            fmt++;
            break;

        case '%': {
            struct format_flags flags = {0};
            int min_width = 0;
            enum format_length length = length_int;
            uint64_t value;

            fmt++;
            fmt = parse_flags(fmt, &flags);
            fmt = parse_min_width(fmt, args, &flags, &min_width);
            fmt = parse_length_modifier(fmt, &length);

            /* Handle the format specifier. */
            switch (*fmt) {
            case '%':
                fmt++;
                chars_written++;
                log_putchar('%');
                break;

            case 'c': {
                char str[2] = {(char)va_arg(args, int), 0};

                fmt++;
                chars_written += print_string(
                    str, str, min_width, &flags, ' ');
                break;
            }

            case 's': {
                char *str = va_arg(args, char *);
                const char *safe_str = (str != NULL) ? str : "(null)";

                fmt++;
                chars_written += print_string(
                    safe_str, safe_str, min_width, &flags, ' ');
                break;
            }

            case 'd':
            case 'i': {
                fmt++;
                value = read_signed_argument(&args, length);
                value = reinterpret_signed_int(length, value,
                                   &flags);

                chars_written += print_int(value, base10,
                               min_width, &flags);
                break;
            }

            case 'b':
                fmt++;
                value = read_unsigned_argument(&args, length);
                value = reinterpret_unsigned_int(length, value);

                chars_written += print_int(value, base2,
                               min_width, &flags);
                break;

            case 'B':
                fmt++;
                flags.upper = true;
                value = read_unsigned_argument(&args, length);
                value = reinterpret_unsigned_int(length, value);

                chars_written += print_int(value, base2,
                               min_width, &flags);
                break;

            case 'o':
                fmt++;
                value = read_unsigned_argument(&args, length);
                value = reinterpret_unsigned_int(length, value);

                chars_written += print_int(value, base8,
                               min_width, &flags);
                break;

            case 'x':
                fmt++;
                value = read_unsigned_argument(&args, length);
                value = reinterpret_unsigned_int(length, value);

                chars_written += print_int(value, base16,
                               min_width, &flags);
                break;

            case 'X':
                fmt++;
                flags.upper = true;
                value = read_unsigned_argument(&args, length);
                value = reinterpret_unsigned_int(length, value);

                chars_written += print_int(value, base16,
                               min_width, &flags);
                break;

            case 'u':
                fmt++;
                value = read_unsigned_argument(&args, length);
                value = reinterpret_unsigned_int(length, value);

                chars_written += print_int(value, base10,
                               min_width, &flags);
                break;

            case 'p':
                fmt++;
                value = (uint64_t)(uintptr_t)va_arg(args, void *);
                min_width = sizeof(size_t) * 2 + 2;
                flags.zero = true;
                flags.alt = true;

                chars_written += print_int(value, base16,
                               min_width, &flags);
                break;

            default:
                chars_written = (size_t)-1;
                goto out;
            }
        }
        }
    }

out:
    return chars_written;
}

/**
 * Prints the given format string to the debug log.
 *
 * The format string supported is the same as described in
 * https://en.cppreference.com/w/c/io/fprintf, with the following exceptions:
 * - Floating-point formatters (`%f`, `%F`, `%e`, `%E`, `%a`, `%A`, `%g`, `%G`,
 *   `%L`) are not supported because floats are not used in Hafnium and
 *   formatting them is too complicated.
 * - `%n` is not supported because it is rarely used and potentially dangerous.
 * - Precision modifiers (`%.*` and `%.` followed by an integer) are not
 *   supported.
 *
 * Returns number of characters written, or `-1` if format string is invalid.
 */

/**
 *   @brief    - This function prints the given string and data onto the uart
 *   @param    - verbosity  : Print Verbosity level
 *             - msg        : Input String
 *             - ...        : ellipses for variadic args
 *   @return   - SUCCESS((Any positive number for character written)/FAILURE(0)
 **/
uint32_t val_printf(print_verbosity_t verbosity, const char *fmt, ...)
{
    size_t chars_written = 0;
    const char *format_msg = (fmt != NULL) ? fmt : null_log_message;
    size_t len = log_strnlen_s(format_msg, LOG_MAX_STRING_LENGTH - 2);
    static bool lastWasNewline = true;
    char formatted_msg[LOG_MAX_STRING_LENGTH];
    va_list args;

    va_start(args, fmt);

    if (verbosity >= VERBOSITY)
    {
        if (lastWasNewline)
        {
            switch (verbosity)
            {
                case INFO:
                    print_raw_string("\t\tINFO: ");
                    break;

                case DBG:
                    print_raw_string("\t\tDBG: ");
                    break;

                case TEST:
                    print_raw_string("\t");
                    break;

                case WARN:
                    print_raw_string("\t\tWARN: ");
                    break;

                case ERROR:
                    print_raw_string("\t\tERROR: ");
                    break;

                case ALWAYS:
                    print_raw_string("");
                    break;

                default:
                    break;
            }
        }

        const bool has_newline = (len > 0) && (format_msg[len - 1] == '\n');
        const bool truncated = (len == LOG_MAX_STRING_LENGTH - 2); /* log_strnlen_s hit the scan limit */

        if (has_newline)
        {
            val_mem_copy(formatted_msg, sizeof(formatted_msg), format_msg, len - 1);
            formatted_msg[len - 1] = '\r';
            formatted_msg[len] = '\n';
            formatted_msg[len + 1] = '\0';

            chars_written = val_log(formatted_msg, args);
            lastWasNewline = true;
        }
        else
        {
            if (truncated)
            {
                val_mem_copy(formatted_msg, sizeof(formatted_msg), format_msg, len);
                formatted_msg[len] = '\0';
                chars_written = val_log(formatted_msg, args);
            }
            else
            {
                chars_written = val_log(format_msg, args);
            }
            lastWasNewline = false;
        }
    }
    va_end(args);

    return (uint32_t)chars_written;
}
