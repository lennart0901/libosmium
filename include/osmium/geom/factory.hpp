#ifndef OSMIUM_GEOM_FACTORY_HPP
#define OSMIUM_GEOM_FACTORY_HPP

/*

This file is part of Osmium (http://osmcode.org/osmium).

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

#include <stdexcept>
#include <string>

#include <osmium/memory/collection.hpp>
#include <osmium/memory/item.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/noderef.hpp>
#include <osmium/osm/way.hpp>

namespace osmium {

    /**
     * @brief Namespace for everything related to geometry.
     */
    namespace geom {

        struct geometry_error : public std::runtime_error {

            geometry_error(const std::string& what) :
                std::runtime_error(what) {
            }

            geometry_error(const char* what) :
                std::runtime_error(what) {
            }

        };

        /**
         * Abstract base class for geometry factories.
         */
        template <class G, class T>
        class GeometryFactory {

            /**
             * Add all points of an outer or inner ring to a multipolygon.
             */
            void add_points(const osmium::OuterRing& nodes) {
                osmium::Location last_location;
                for (const osmium::NodeRef& n : nodes) {
                    if (last_location != n.location()) {
                        last_location = n.location();
                        if (last_location) {
                            static_cast<G*>(this)->multipolygon_add_location(last_location);
                        } else {
                            throw geometry_error("location is undefined");
                        }
                    }
                }
            }

        protected:

            GeometryFactory<G, T>() {
            }

        public:

            typedef typename T::point_type        point_type;
            typedef typename T::linestring_type   linestring_type;
            typedef typename T::polygon_type      polygon_type;
            typedef typename T::multipolygon_type multipolygon_type;
            typedef typename T::ring_type         ring_type;

            /* Point */

            point_type create_point(const osmium::Location location) {
                if (location) {
                    return static_cast<G*>(this)->make_point(location);
                } else {
                    throw geometry_error("location is undefined");
                }
            }

            point_type create_point(const osmium::Node& node) {
                return create_point(node.location());
            }

            point_type create_point(const osmium::NodeRef& way_node) {
                return create_point(way_node.location());
            }

            /* LineString */

            linestring_type create_linestring(const osmium::WayNodeList& wnl, bool unique=true, bool reverse=false) {
                static_cast<G*>(this)->linestring_start();

                if (unique) {
                    osmium::Location last_location;
                    if (reverse) {
                        for (int i = wnl.size()-1; i >= 0; --i) {
                            if (last_location != wnl[i].location()) {
                                last_location = wnl[i].location();
                                if (last_location) {
                                    static_cast<G*>(this)->linestring_add_location(last_location);
                                } else {
                                    throw geometry_error("location is undefined");
                                }
                            }
                        }
                    } else {
                        for (auto& wn : wnl) {
                            if (last_location != wn.location()) {
                                last_location = wn.location();
                                if (last_location) {
                                    static_cast<G*>(this)->linestring_add_location(last_location);
                                } else {
                                    throw geometry_error("location is undefined");
                                }
                            }
                        }
                    }
                } else {
                    if (reverse) {
                        for (int i = wnl.size()-1; i >= 0; --i) {
                            if (wnl[i].location()) {
                                static_cast<G*>(this)->linestring_add_location(wnl[i].location());
                            } else {
                                throw geometry_error("location is undefined");
                            }
                        }
                    } else {
                        for (auto& wn : wnl) {
                            if (wn.location()) {
                                static_cast<G*>(this)->linestring_add_location(wn.location());
                            } else {
                                throw geometry_error("location is undefined");
                            }
                        }
                    }
                }

                return static_cast<G*>(this)->linestring_finish();
            }

            linestring_type create_linestring(const osmium::Way& way, bool unique=true, bool reverse=false) {
                return create_linestring(way.nodes(), unique, reverse);
            }

            /* MultiPolygon */

            multipolygon_type create_multipolygon(const osmium::Area& area) {
                int num_rings = 0;
                static_cast<G*>(this)->multipolygon_start();

                for (auto it = area.cbegin(); it != area.cend(); ++it) {
                    const osmium::OuterRing& ring = static_cast<const osmium::OuterRing&>(*it);
                    if (it->type() == osmium::item_type::outer_ring) {
                        ++num_rings;
                        static_cast<G*>(this)->multipolygon_add_outer_ring();
                        add_points(ring);
                    } else if (it->type() == osmium::item_type::inner_ring) {
                        ++num_rings;
                        static_cast<G*>(this)->multipolygon_add_inner_ring();
                        add_points(ring);
                    }
                }

                // if there are no rings, this area is invalid
                if (num_rings == 0) {
                    throw geometry_error("invalid area");
                }

                return static_cast<G*>(this)->multipolygon_finish();
            }

        }; // class GeometryFactory

    } // namespace geom

} // namespace osmium

#endif // OSMIUM_GEOM_FACTORY_HPP
