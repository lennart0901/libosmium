#ifndef OSMIUM_TAGS_FILTER_HPP
#define OSMIUM_TAGS_FILTER_HPP

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

#include <string>
#include <type_traits>
#include <vector>

#include <boost/iterator/filter_iterator.hpp>

#include <osmium/memory/collection.hpp>
#include <osmium/osm/tag.hpp>

namespace osmium {

    namespace tags {

        template <class TKey>
        struct match_key {
            bool operator()(const TKey& rule_key, const char* tag_key) {
                return rule_key == tag_key;
            }
        };

        struct match_key_prefix {
            bool operator()(const std::string& rule_key, const char* tag_key) {
                return rule_key.compare(0, std::string::npos, tag_key, 0, rule_key.size()) == 0;
            }
        };

        template <class TValue>
        struct match_value {
            bool operator()(const TValue& rule_value, const char* tag_value) {
                return rule_value == tag_value;
            }
        };

        template <>
        struct match_value<void> {
            bool operator()(const bool, const char*) {
                return true;
            }
        };

        template <class TKey, class TValue=void, class TKeyComp=match_key<TKey>, class TValueComp=match_value<TValue>>
        class Filter {

            typedef TKey key_type;
            typedef typename std::conditional<std::is_void<TValue>::value, bool, TValue>::type value_type;

            struct Rule {
                key_type key;
                value_type value;
                bool ignore_value;
                bool result;

                explicit Rule(bool r, bool ignore, const key_type& k, const value_type& v) :
                    key(k),
                    value(v),
                    ignore_value(ignore),
                    result(r) {
                }

                explicit Rule(bool r, bool ignore, const key_type& k) :
                    key(k),
                    value(),
                    ignore_value(ignore),
                    result(r) {
                }

            };

            std::vector<Rule> m_rules {};
            bool m_default_result;

        public:

            typedef Filter<TKey, TValue> filter_type;
            typedef const osmium::Tag& argument_type;
            typedef bool result_type;
            typedef boost::filter_iterator<filter_type, osmium::TagList::const_iterator> iterator;

            explicit Filter(bool default_result = false) :
                m_default_result(default_result) {
            }

            template <class V=TValue, typename std::enable_if<!std::is_void<V>::value, int>::type = 0>
            Filter& add(bool result, const key_type& key, const value_type& value) {
                m_rules.emplace_back(result, false, key, value);
                return *this;
            }

            Filter& add(bool result, const key_type& key) {
                m_rules.emplace_back(result, true, key);
                return *this;
            }

            bool operator()(const osmium::Tag& tag) const {
                for (const Rule& rule : m_rules) {
                    if (TKeyComp()(rule.key, tag.key()) && (rule.ignore_value || TValueComp()(rule.value, tag.value()))) {
                        return rule.result;
                    }
                }
                return m_default_result;
            }

        }; // Filter

        typedef Filter<std::string, std::string> KeyValueFilter;
        typedef Filter<std::string> KeyFilter;
        typedef Filter<std::string, void, match_key_prefix> KeyPrefixFilter;

    } // namespace tags

} // namespace osmium

#endif // OSMIUM_TAGS_FILTER_HPP
