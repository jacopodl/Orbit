// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#ifndef ORBIT_ORBITER_DATATYPE_SUPPORT_COMMON_H_
#define ORBIT_ORBITER_DATATYPE_SUPPORT_COMMON_H_

#include <orbit/datatype.h>

#include <orbit/orbiter/datatype/list.h>

#include <orbit/orbiter/datatype/support/byteops.h>

namespace orbiter::datatype::support {
    /**
     * @brief Factory-function pointer used by the generic splitters to create per-chunk objects.
     *
     * @tparam T  Concrete datatype produced for each split segment (e.g. ORString).
     *
     * Called with a slice of the source buffer; the factory is responsible for owning the bytes
     * (copying into a fresh container, interning, whatever is appropriate for @p T).
     * Must return an empty handle on failure and set the isolate panic state.
     */
    template<typename T>
    using ChunkFn = Handle<T> (*)(Isolate *isolate, const unsigned char *buffer, MSize length);

    /**
     * @brief Generic split-on-separator: produce a @c List of @p T chunks.
     *
     * The pattern match is exact (non-overlapping), so "aaaa" / "aa" yields ["", "", ""].
     * The result always contains at least one element (the tail remainder), matching
     * Python-style semantics.
     *
     * The list capacity is sized exactly by a pre-count pass, so the append loop never
     * reallocates. When @p maxsplit is negative the split is unbounded; when 0, the whole
     * buffer is returned as a single chunk.
     *
     * @tparam T  Chunk type; combined with @p chunk_new this determines the element type
     *            of the returned list.
     *
     * @param isolate    Owning isolate; passed through to @p chunk_new and to ListNew.
     * @param buffer     Source buffer.
     * @param blen       Length of @p buffer in bytes.
     * @param pattern    Separator pattern. Must not be null.
     * @param plen       Length of @p pattern. Must be > 0 — the caller is expected to
     *                   validate and raise the appropriate error otherwise.
     * @param chunk_new  Factory invoked for every segment, including empty leading/trailing
     *                   segments and the final tail.
     * @param maxsplit   Maximum number of splits to perform (list will contain at most
     *                   @p maxsplit + 1 elements). Negative = unlimited.
     *
     * @return A new HList, empty on allocation/factory failure.
     */
    template<typename ChunkNew>
    HList Split(Isolate *isolate,
                const unsigned char *buffer, const MSize blen,
                const unsigned char *pattern, const MSize plen,
                ChunkNew chunk_new,
                const MSSize maxsplit = -1) {
        assert(pattern != nullptr && plen > 0);

        // Pre-count occurrences (capped at maxsplit) so the list is sized exactly
        // and the append loop never reallocates.
        const MSize occurrences = Count(buffer, blen, pattern, plen, maxsplit);

        auto list = ListNew(isolate, occurrences + 1);
        if (!list)
            return {};

        MSize pos = 0;

        for (MSize n = 0; n < occurrences; n++) {
            const MSSize idx = Search(buffer + pos, blen - pos, pattern, plen);
            assert(idx >= 0); // guaranteed by the pre-count above

            const auto part_len = (MSize) idx;

            auto part = chunk_new(isolate, buffer + pos, part_len);
            if (!part)
                return {};

            if (!ListAppend(list.get(), (OObject *) part.get()))
                return {};

            pos += part_len + plen;
        }

        // Tail remainder (always appended — may be empty if buffer ends with the separator)
        auto tail = chunk_new(isolate, buffer + pos, blen - pos);
        if (!tail)
            return {};

        if (!ListAppend(list.get(), (OObject *) tail.get()))
            return {};

        return list;
    }

    /**
     * @brief Generic splitlines: produce a @c List of @p T chunks, one per line.
     *
     * Line terminators recognized:
     *   - `\n`   (LF)
     *   - `\r\n` (CRLF) — only when @p universal is true
     *   - `\r`   (lone CR) — only when @p universal is true
     *
     * Semantics (Java-style):
     *   - A trailing single terminator is swallowed: `"a\n"` → `["a"]`.
     *   - Consecutive terminators produce empty lines: `"a\n\nb"` → `["a", "", "b"]`.
     *   - Empty input returns an empty list.
     *
     * When @p keepends is true the terminator is included in the emitted chunk;
     * otherwise only the line content is copied.
     *
     * The list capacity is sized exactly by a pre-count pass, so the append loop never
     * reallocates.
     *
     * @tparam T  Chunk type produced by @p chunk_new.
     *
     * @param isolate    Owning isolate; passed through to @p chunk_new and to ListNew.
     * @param buffer     Source buffer.
     * @param blen       Length of @p buffer in bytes.
     * @param chunk_new  Factory invoked for every line.
     * @param keepends   When true, include the terminator in each chunk.
     * @param universal  When true, treat `\r\n` and `\r` as terminators in addition to `\n`.
     *
     * @return A new HList, empty on allocation/factory failure.
     */
    template<typename T>
    HList SplitLines(Isolate *isolate, const unsigned char *buffer, const MSize blen, const ChunkFn<T> chunk_new,
                     const bool keepends, const bool universal) {
        const MSize occurrences = CountNewLines(buffer, blen, universal);

        // The trailing terminator (if any) is swallowed, so when the buffer ends with a
        // newline the result has exactly `occurrences` lines; otherwise `occurrences + 1`
        // (one extra for the final unterminated tail).
        const bool has_tail = blen > 0 && [&]() -> bool {
            const auto last = (unsigned char) buffer[blen - 1];

            if (last == '\n')
                return false;

            if (universal && last == '\r')
                return false;

            return true;
        }();

        const MSize capacity = occurrences + (has_tail ? 1 : 0);

        auto list = ListNew(isolate, capacity);
        if (!list)
            return {};

        MSize pos = 0;

        while (pos < blen) {
            MSize sep_len = 0;
            const auto idx = FindNewLine(buffer + pos, blen - pos, universal, &sep_len);

            if (idx < 0) {
                // Final unterminated line
                auto part = chunk_new(isolate, buffer + pos, blen - pos);
                if (!part)
                    return {};

                if (!ListAppend(list.get(), (OObject *) part.get()))
                    return {};

                break;
            }

            const auto content_len = (MSize) idx;
            const MSize chunk_len = keepends ? content_len + sep_len : content_len;

            auto part = chunk_new(isolate, buffer + pos, chunk_len);
            if (!part)
                return {};

            if (!ListAppend(list.get(), (OObject *) part.get()))
                return {};

            pos += content_len + sep_len;
        }

        return list;
    }

    /**
     * @brief Generic whitespace split: produce a @c List of @p T non-empty chunks.
     *
     * Matches Python `str.split()` with no argument:
     *   - Leading and trailing whitespace runs are stripped.
     *   - Any run of whitespace (`[ \t\n\v\f\r]`) separates words.
     *   - Empty or all-whitespace input returns an empty list.
     *
     * @tparam T  Chunk type produced by @p chunk_new.
     *
     * @param isolate    Owning isolate; passed through to @p chunk_new and to ListNew.
     * @param buffer     Source buffer.
     * @param blen       Length of @p buffer in bytes.
     * @param chunk_new  Factory invoked for every word.
     *
     * @return A new HList, empty on allocation/factory failure.
     */
    template<typename T>
    HList SplitWhitespace(Isolate *isolate, const unsigned char *buffer, const MSize blen, const ChunkFn<T> chunk_new) {
        // Pre-count words in a single pass so the list is sized exactly.
        MSize word_count = 0;
        {
            bool in_word = false;

            for (MSize i = 0; i < blen; i++) {
                const auto c = (unsigned char) buffer[i];

                if (IsAsciiWhitespace(c))
                    in_word = false;
                else if (!in_word) {
                    in_word = true;

                    word_count++;
                }
            }
        }

        auto list = ListNew(isolate, word_count);
        if (!list)
            return {};

        MSize pos = 0;

        while (pos < blen) {
            // Skip leading whitespace
            while (pos < blen && IsAsciiWhitespace((unsigned char) buffer[pos]))
                pos++;

            if (pos >= blen)
                break;

            const MSize start = pos;

            // Scan word
            while (pos < blen && !IsAsciiWhitespace((unsigned char) buffer[pos]))
                pos++;

            auto part = chunk_new(isolate, buffer + start, pos - start);
            if (!part)
                return {};

            if (!ListAppend(list.get(), (OObject *) part.get()))
                return {};
        }

        return list;
    }
} // namespace orbiter::datatype::support

#endif // !ORBIT_ORBITER_DATATYPE_SUPPORT_COMMON_H_
