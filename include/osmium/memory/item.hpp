#ifndef OSMIUM_MEMORY_ITEM_HPP
#define OSMIUM_MEMORY_ITEM_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <cstddef>
#include <cstdint>

namespace osmium {

    // forward declaration, see osmium/osm/item_type.hpp for declaration
    enum class item_type : uint16_t;

    namespace builder {
        class Builder;
    }

    namespace memory {

        // align datastructures to this many bytes
        constexpr size_t align_bytes = 8;

        inline size_t padded_length(size_t length) {
            return (length + align_bytes - 1) & ~(align_bytes - 1);
        }

        /**
         * @brief Namespace for Osmium internal use
         */
        namespace detail {

            /**
            * This class contains only a helper method used in several
            * other classes.
            */
            class ItemHelper {

            protected:

                ItemHelper() = default;

                ~ItemHelper() = default;

                ItemHelper(const ItemHelper&) = default;
                ItemHelper(ItemHelper&&) = default;

                ItemHelper& operator=(const ItemHelper&) = default;
                ItemHelper& operator=(ItemHelper&&) = default;

            public:

                unsigned char* data() {
                    return reinterpret_cast<unsigned char*>(this);
                }

                const unsigned char* data() const {
                    return reinterpret_cast<const unsigned char*>(this);
                }

            };

        } // namespace detail

        typedef uint32_t item_size_type;

        class Item : public osmium::memory::detail::ItemHelper {

            item_size_type m_size;
            item_type m_type;
            bool m_removed : 1;

            template <class TMember>
            friend class CollectionIterator;

            friend class osmium::builder::Builder;

            unsigned char* next() {
                return data() + padded_size();
            }

            const unsigned char* next() const {
                return data() + padded_size();
            }

            Item& add_size(const item_size_type size) {
                m_size += size;
                return *this;
            }

        protected:

            explicit Item(item_size_type size=0, item_type type=item_type()) :
                m_size(size),
                m_type(type),
                m_removed(false) {
            }

            Item(const Item&) = delete;
            Item(Item&&) = delete;

            Item& operator=(const Item&) = delete;
            Item& operator=(Item&&) = delete;

            Item& type(const item_type item_type) {
                m_type = item_type;
                return *this;
            }

        public:

            item_size_type byte_size() const {
                return m_size;
            }

            item_size_type padded_size() const {
                return padded_length(m_size);
            }

            item_type type() const {
                return m_type;
            }

            bool removed() const {
                return m_removed;
            }

            void removed(bool removed) {
                m_removed = removed;
            }

        }; // class Item

        static_assert(sizeof(Item) == 8, "Class osmium::Item has wrong size!");
        static_assert(sizeof(Item) % align_bytes == 0, "Class osmium::Item has wrong size to be aligned properly!");

    } // namespace memory

} // namespace osmium

#endif // OSMIUM_MEMORY_ITEM_HPP
