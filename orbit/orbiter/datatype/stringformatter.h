// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_ORBITER_DATATYPE_STRINGFORMATTER_H_
#define ORBIT_ORBITER_DATATYPE_STRINGFORMATTER_H_

#include <cstring>
#include <type_traits>

#include <orbit/orbiter/memory/iallocator.h>

#include <orbit/orbiter/datatype/orstring.h>

namespace orbiter::datatype {
    enum class FormatFlags : U8 {
        NONE = 0x00,
        LJUST = 0x01, ///< Left-justify  (-)
        SIGN = 0x02, ///< Force sign    (+)
        BLANK = 0x04, ///< Space before positive ( )
        ALT = 0x08, ///< Alternate form (#)
        ZERO = 0x10 ///< Zero-pad      (0)
    };

    /**
     * @brief printf-style string formatter that operates on Orbit objects
     *
     * Supports the following conversion specifiers:
     *   %s  – string (calls ToString on the argument)
     *   %c  – single character (ORString with cp_length == 1, or integer codepoint)
     *   %b  – signed binary        %B – signed binary uppercase prefix
     *   %o  – signed octal
     *   %d  – signed decimal       %i – alias for %d
     *   %u  – unsigned decimal
     *   %x  – unsigned hex lower   %X – unsigned hex upper
     *   %e  – decimal exponential  %E – uppercase
     *   %f  – decimal fixed point  %F – uppercase
     *   %g  – decimal shortest     %G – uppercase
     *   %%  – literal percent
     *
     * Flags: - + (space) # 0
     * Width and precision: numeric literal or * (read from args)
     *
     * Usage:
     *   StringFormatter sf(isolate, "hello %s, you are %d years old", args, false);
     *   HORString result = sf.FormatToString();
     */
    class StringFormatter {
        memory::IsolateAllocator allocator_;

        Isolate *isolate_;

        struct {
            const unsigned char *cursor = nullptr;
            const unsigned char *end = nullptr;

            OObject *args = nullptr;
            MSize args_index = 0;
            MSize args_length = 0;
            int nspec = 0;

            FormatFlags flags = FormatFlags::NONE;
            int width = 0;
            int prec = 0;
        } fmt_;

        struct {
            unsigned char *buffer = nullptr;
            unsigned char *cursor = nullptr;
            unsigned char *end = nullptr;
        } output_;

        bool failed_ = false;

        bool string_as_bytes_ = false;

        // -----------------------------------------------------------------------
        // Private helpers
        // -----------------------------------------------------------------------

        /** Fetch the next argument from args (Tuple or single object). */
        OObject *NextArg();

        /** Format %s as raw bytes. */
        MSSize FormatBytes();

        /** Format %s by calling ToString on the argument. */
        MSSize FormatString();

        /** Append @p length bytes from @p buffer to the output.
         *  @p overalloc pre-allocates extra capacity beyond what is needed. */
        MSSize Write(const unsigned char *buffer, MSize length, int overalloc);

        /** Append @p times copies of @p ch to the output. */
        MSSize WriteRepeat(char ch, int times);

        /** Ensure the output buffer can accept @p length more bytes. */
        bool BufferResize(MSize length);

        /** Dispatch a single format specifier (cursor points at the conversion
         *  character, e.g. 'd'). Returns false on error. */
        bool FormatSpecifier();

        int FormatChar();

        int FormatDecimal(unsigned char op);

        int FormatInteger(int base, bool unsign, bool upper);

        /** Advance cursor past literal text, stopping at the next '%'. */
        bool NextSpecifier();

        /** Parse flags, width, and precision after a '%'. */
        bool ParseOption();

        /** Read a width or precision value from args ('*' form). */
        bool ParseStarOption(bool prec);

        // -----------------------------------------------------------------------
        // Static formatting helpers
        // -----------------------------------------------------------------------

        /**
         * @brief Finalise the digit buffer built by WriteNumber.
         *
         * Appends zero-padding, alternate prefix (0b / 0o / 0x), sign, and space
         * padding then reverses the buffer in-place so digits are in the right order.
         *
         * @return Number of bytes written into @p buf.
         */
        static int FormatNumber(unsigned char *buf, int index, int base, int width,
                                bool upper, bool neg, FormatFlags flags);

        /**
         * @brief Convert a numeric value into ASCII digits followed by FormatNumber.
         *
         * Works for both signed (IntegerUnderlying) and unsigned
         * (UIntegerUnderlying) via template instantiation.
         */
        template<typename T>
        static int WriteNumber(unsigned char *buf, T num, int base, const int prec,
                               const int width, const bool upper, const FormatFlags flags) {
            static unsigned char l_case[] = "0123456789abcdef";
            static unsigned char u_case[] = "0123456789ABCDEF";
            const unsigned char *p_case = upper ? u_case : l_case;

            int index = 0;
            bool neg = false;

            if constexpr (std::is_signed_v<T>) {
                if (num < 0) {
                    num = -num;
                    neg = true;
                } else if (num == 0)
                    buf[index++] = '0';
            } else {
                if (num == 0)
                    buf[index++] = '0';
            }

            while (num) {
                buf[index++] = p_case[num % base];
                num /= base;
            }

            if (prec > index) {
                int pad = prec - index;

                while (pad--)
                    buf[index++] = '0';
            }

            return FormatNumber(buf, index, base, width, upper, neg, flags);
        }

    public:
        /**
         * @brief Construct a formatter from a null-terminated format string.
         *
         * @param isolate         Isolate used for error reporting and string creation.
         * @param fmt             Null-terminated format string.
         * @param args            Argument(s): a Tuple for multiple args, or any
         *                        OObject for a single argument.
         * @param string_as_bytes If true, %s formats the argument as raw bytes
         *                        rather than calling ToString.
         */
        StringFormatter(Isolate *isolate, const char *fmt, OObject *args, const bool string_as_bytes)
            : allocator_(isolate), isolate_(isolate), string_as_bytes_(string_as_bytes) {
            this->fmt_.cursor = (const unsigned char *) fmt;
            this->fmt_.end = this->fmt_.cursor + strlen(fmt);
            this->fmt_.args = args;
        }

        /**
         * @brief Construct a formatter from a buffer + explicit length.
         *
         */
        StringFormatter(Isolate *isolate, const char *fmt, const MSize length, OObject *args,
                        const bool string_as_bytes) : allocator_(isolate), isolate_(isolate),
                                                      string_as_bytes_(string_as_bytes) {
            this->fmt_.cursor = (const unsigned char *) fmt;
            this->fmt_.end = this->fmt_.cursor + length;
            this->fmt_.args = args;
        }

        ~StringFormatter();

        /** @return true if any formatting step encountered an error. */
        [[nodiscard]] bool HasError() const {
            return this->failed_;
        }

        /**
         * @brief Format all arguments and return the raw output buffer.
         *
         * On success the buffer is owned by this StringFormatter object until
         * ReleaseOwnership() is called.  On failure nullptr is returned and an
         * error has already been set on the isolate.
         *
         * @param out_len Set to the number of bytes in the formatted output
         *                (excluding the null terminator).
         * @param out_cap Set to the total capacity of the allocated buffer.
         */
        unsigned char *Format(MSize *out_len, MSize *out_cap);

        /**
         * @brief Format all arguments and return the result as an ORString.
         *
         * Convenience wrapper that calls Format() and creates an ORString copy.
         * The internal buffer is freed automatically; callers do NOT need to call
         * ReleaseOwnership().
         *
         * @return Handle to the formatted string, or an empty handle on error.
         */
        HORString FormatToString();

        /**
         * @brief Transfer ownership of the output buffer to the caller.
         *
         * After this call the destructor will NOT free the buffer.
         * Use when passing the buffer directly to ORStringNew or similar.
         */
        void ReleaseOwnership();
    };
}

ENUMBITMASK_ENABLE(orbiter::datatype::FormatFlags);

#endif // !ORBIT_ORBITER_DATATYPE_STRINGFORMATTER_H_
