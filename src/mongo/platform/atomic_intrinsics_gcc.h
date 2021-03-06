/*    Copyright 2012 10gen Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * Implementation of the AtomicIntrinsics<T>::* operations for IA-32 and AMD64 systems using a
 * GCC-compatible compiler toolchain.
 */
/* re-implement it on Powerpc--hcj */
#pragma once

#include <boost/utility.hpp>

namespace mongo {

    /**
     * Instantiation of AtomicIntrinsics<> for all word types T where sizeof<T> <= sizeof(void *).
     *
     * On 32-bit systems, this handles 8-, 16- and 32-bit word types.  On 64-bit systems,
     * it handles 8-, 16, 32- and 64-bit types.
     */
    template <typename T, typename IsTLarge=void>
    class AtomicIntrinsics {
    public:
#if 0
        static T compareAndSwap(volatile T* dest, T expected, T newValue) {
        T result;
        __asm__ __volatile__ (
           "0: lwarx %0, 0, %1\n\t"
                 "      xor. %0, %3, %0\n\t"
              " bne 1f\n\t"
            " stwcx. %2, 0, %1\n\t"
                 "      bne- 0b\n\t"
            " isync\n\t"
        "1: "
        : "=&r"(result)
        : "r"(dest), "r"(newValue), "r"(expected)
        : "cr0");
        return result;
        }
#else
        static T compareAndSwap(volatile T* dest, T expected, T newValue) {
        return __sync_val_compare_and_swap(dest,  expected, newValue);
        }
#endif
        static T swap(volatile T* dest, T newValue) {

        T expected;
        T actual;
            do {
                expected = *dest;
                actual = compareAndSwap(dest, expected, newValue);
            } while (actual != expected);
            return actual;
        }

        static T load(volatile const T* value) {
            return compareAndSwap(const_cast<volatile T*>(value), T(0), T(0));
        }

        static T loadRelaxed(volatile const T* value) {
            return *value;
        }

        static void store(volatile T* dest, T newValue) {
            swap(dest, newValue);
        }
#if 0
        static T fetchAndAdd(volatile T* dest, T increment) {

            T expected;
            T actual;
            do {
                expected = load(dest);
                actual = compareAndSwap(dest, expected, expected + increment);
            } while (actual != expected);
            return actual;
        }
#else
        static T fetchAndAdd(volatile T* dest, T increment) {

            return __sync_fetch_and_add(dest, increment);
        }
#endif
    private:
        AtomicIntrinsics();
        ~AtomicIntrinsics();

    };

    /**
     * Instantiation of AtomicIntrinsics<T> where sizeof<T> exceeds sizeof(void*).
     *
     * On 32-bit systems, this handles the 64-bit word type.  Not used on 64-bit systems.
     *
     * Note that the implementations of swap, store and fetchAndAdd spin until they succeed.  This
     * implementation is thread-safe, but may have poor performance in high-contention environments.
     * However, no superior solution exists for IA-32 (32-bit x86) systems.
     */
    template <typename T>
    class AtomicIntrinsics<T, typename boost::disable_if_c<sizeof(T) <= sizeof(void*)>::type> {
    public:
#if 0
        static T compareAndSwap(volatile T* dest, T expected, T newValue) {
        T result;
        __asm__ __volatile__ (
           "0: lwarx %0, 0, %1\n\t"
                 "      xor. %0, %3, %0\n\t"
              " bne 1f\n\t"
            " stwcx. %2, 0, %1\n\t"
                 "      bne- 0b\n\t"
            " isync\n\t"
        "1: "
        : "=&r"(result)
        : "r"(dest), "r"(newValue), "r"(expected)
        : "cr0");
        return result;
        }
#else
        static T compareAndSwap(volatile T* dest, T expected, T newValue) {
        T result;
        return __sync_val_compare_and_swap(dest,  expected, newValue);
        }
#endif
        static T swap(volatile T* dest, T newValue) {

        T expected;
        T actual;
            do {
                expected = *dest;
                actual = compareAndSwap(dest, expected, newValue);
            } while (actual != expected);
            return actual;
        }

        static T load(volatile const T* value) {
            return compareAndSwap(const_cast<volatile T*>(value), T(0), T(0));
        }

        static void store(volatile T* dest, T newValue) {
            swap(dest, newValue);
        }

#if 0
        static T fetchAndAdd(volatile T* dest, T increment) {

            T expected;
            T actual;
            do {
                expected = load(dest);
                actual = compareAndSwap(dest, expected, expected + increment);
            } while (actual != expected);
            return actual;
        }
#else
        static T fetchAndAdd(volatile T* dest, T increment) {

            return __sync_fetch_and_add(dest, increment);
        }
#endif

    private:
        AtomicIntrinsics();
        ~AtomicIntrinsics();
    };

}  // namespace mongo
