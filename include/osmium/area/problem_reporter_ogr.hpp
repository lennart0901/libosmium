#ifndef OSMIUM_AREA_PROBLEM_REPORTER_OGR_HPP
#define OSMIUM_AREA_PROBLEM_REPORTER_OGR_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2014 Jochen Topf <jochen@topf.org> and others (see README).

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

#define OSMIUM_COMPILE_WITH_CFLAGS_OGR `gdal-config --cflags`
#define OSMIUM_LINK_WITH_LIBS_OGR `gdal-config --libs`

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wredundant-decls"
# include <ogr_api.h>
# include <ogrsf_frmts.h>
#pragma GCC diagnostic pop

#include <memory>
#include <stdexcept>

#include <osmium/area/problem_reporter.hpp>
#include <osmium/geom/ogr.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>

namespace osmium {

    namespace area {

        /**
         * Report problems when assembling areas by adding them to
         * layers in an OGR datasource.
         */
        class ProblemReporterOGR : public ProblemReporter {

            osmium::geom::OGRFactory m_ogr_factory {};

            OGRDataSource* m_data_source;

            OGRLayer* m_layer_perror;
            OGRLayer* m_layer_lerror;

            void write_point(const char* problem_type, osmium::object_id_type id1, osmium::object_id_type id2, osmium::Location location) {
                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_perror->GetLayerDefn());
                std::unique_ptr<OGRPoint> ogr_point = m_ogr_factory.create_point(location);
                feature->SetGeometry(ogr_point.get());
                feature->SetField("id1", static_cast<double>(id1));
                feature->SetField("id2", static_cast<double>(id2));
                feature->SetField("problem_type", problem_type);

                if (m_layer_perror->CreateFeature(feature) != OGRERR_NONE) {
                    std::runtime_error("Failed to create feature on layer 'perrors'");
                }

                OGRFeature::DestroyFeature(feature);
            }

            void write_line(const char* problem_type, osmium::object_id_type id1, osmium::object_id_type id2, osmium::Location loc1, osmium::Location loc2) {
                std::unique_ptr<OGRPoint> ogr_point1 = m_ogr_factory.create_point(loc1);
                std::unique_ptr<OGRPoint> ogr_point2 = m_ogr_factory.create_point(loc2);
                std::unique_ptr<OGRLineString> ogr_linestring = std::unique_ptr<OGRLineString>(new OGRLineString());
                ogr_linestring->addPoint(ogr_point1.get());
                ogr_linestring->addPoint(ogr_point2.get());
                OGRFeature* feature = OGRFeature::CreateFeature(m_layer_lerror->GetLayerDefn());
                feature->SetGeometry(ogr_linestring.get());
                feature->SetField("id1", static_cast<double>(id1));
                feature->SetField("id2", static_cast<double>(id2));
                feature->SetField("problem_type", problem_type);

                if (m_layer_lerror->CreateFeature(feature) != OGRERR_NONE) {
                    std::runtime_error("Failed to create feature on layer 'lerrors'");
                }

                OGRFeature::DestroyFeature(feature);
            }

        public:

            explicit ProblemReporterOGR(OGRDataSource* data_source) :
                m_data_source(data_source) {

                OGRSpatialReference sparef;
                sparef.SetWellKnownGeogCS("WGS84");

                m_layer_perror = m_data_source->CreateLayer("perrors", &sparef, wkbPoint, nullptr);
                if (!m_layer_perror) {
                    std::runtime_error("Layer creation failed for layer 'perrors'");
                }

                OGRFieldDefn layer_perror_field_id1("id1", OFTReal);
                layer_perror_field_id1.SetWidth(10);

                if (m_layer_perror->CreateField(&layer_perror_field_id1) != OGRERR_NONE) {
                    std::runtime_error("Creating field 'id1' failed for layer 'perrors'");
                }

                OGRFieldDefn layer_perror_field_id2("id2", OFTReal);
                layer_perror_field_id2.SetWidth(10);

                if (m_layer_perror->CreateField(&layer_perror_field_id2) != OGRERR_NONE) {
                    std::runtime_error("Creating field 'id2' failed for layer 'perrors'");
                }

                OGRFieldDefn layer_perror_field_problem_type("problem_type", OFTString);
                layer_perror_field_problem_type.SetWidth(30);

                if (m_layer_perror->CreateField(&layer_perror_field_problem_type) != OGRERR_NONE) {
                    std::runtime_error("Creating field 'problem_type' failed for layer 'perrors'");
                }

                /**************/

                m_layer_lerror = m_data_source->CreateLayer("lerrors", &sparef, wkbLineString, nullptr);
                if (!m_layer_lerror) {
                    std::runtime_error("Layer creation failed for layer 'lerrors'");
                }

                OGRFieldDefn layer_lerror_field_id1("id1", OFTReal);
                layer_lerror_field_id1.SetWidth(10);

                if (m_layer_lerror->CreateField(&layer_lerror_field_id1) != OGRERR_NONE) {
                    std::runtime_error("Creating field 'id1' failed for layer 'lerrors'");
                }

                OGRFieldDefn layer_lerror_field_id2("id2", OFTReal);
                layer_lerror_field_id2.SetWidth(10);

                if (m_layer_lerror->CreateField(&layer_lerror_field_id2) != OGRERR_NONE) {
                    std::runtime_error("Creating field 'id2' failed for layer 'lerrors'");
                }

                OGRFieldDefn layer_lerror_field_problem_type("problem_type", OFTString);
                layer_lerror_field_problem_type.SetWidth(30);

                if (m_layer_lerror->CreateField(&layer_lerror_field_problem_type) != OGRERR_NONE) {
                    std::runtime_error("Creating field 'problem_type' failed for layer 'lerrors'");
                }
            }

            virtual ~ProblemReporterOGR() = default;

            void report_duplicate_node(osmium::object_id_type node_id1, osmium::object_id_type node_id2, osmium::Location location) override {
                write_point("duplicate_node", node_id1, node_id2, location);
            }

            void report_intersection(osmium::object_id_type way1_id, osmium::Location way1_seg_start, osmium::Location way1_seg_end,
                                     osmium::object_id_type way2_id, osmium::Location way2_seg_start, osmium::Location way2_seg_end, osmium::Location intersection) override {
                write_point("intersection", m_object_id, 0, intersection);
                write_line("intersection", m_object_id, way1_id, way1_seg_start, way1_seg_end);
                write_line("intersection", m_object_id, way2_id, way2_seg_start, way2_seg_end);
            }

            void report_ring_not_closed(osmium::Location end1, osmium::Location end2) override {
                write_point("ring_not_closed", m_object_id, 0, end1);
                write_point("ring_not_closed", m_object_id, 0, end2);
            }

            void report_role_should_be_outer(osmium::object_id_type way_id, osmium::Location seg_start, osmium::Location seg_end) override {
                write_line("role_should_be_outer", m_object_id, way_id, seg_start, seg_end);
            }

            void report_role_should_be_inner(osmium::object_id_type way_id, osmium::Location seg_start, osmium::Location seg_end) override {
                write_line("role_should_be_inner", m_object_id, way_id, seg_start, seg_end);
            }

        }; // class ProblemReporterOGR

    } // namespace area

} // namespace osmium

#endif // OSMIUM_AREA_PROBLEM_REPORTER_OGR_HPP
