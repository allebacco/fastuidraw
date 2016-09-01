/*!
 * \file stroked_path.hpp
 * \brief file stroked_path.hpp
 *
 * Copyright 2016 by Intel.
 *
 * Contact: kevin.rogovin@intel.com
 *
 * This Source Code Form is subject to the
 * terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with
 * this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * \author Kevin Rogovin <kevin.rogovin@intel.com>
 *
 */


#pragma once

#include <fastuidraw/util/fastuidraw_memory.hpp>
#include <fastuidraw/util/vecN.hpp>
#include <fastuidraw/util/c_array.hpp>
#include <fastuidraw/util/reference_counted.hpp>

namespace fastuidraw  {

///@cond
class PainterAttributeData;
class TessellatedPath;
class Path;
///@endcond

/*!\addtogroup Core
  @{
 */

/*!
  A StrokedPath represents the data needed to draw a path stroked.
  It contains -all- the data needed to stroke a path regardless of
  stroking style. in particular, for a given TessellatedPath,
  one only needs to construct a StrokedPath <i>once</i> regardless
  of how one strokes the original path for drawing.
 */
class StrokedPath:
    public reference_counted<StrokedPath>::non_concurrent
{
public:
  /*!
    Enumeration for specifing a point type; the point
    type is used to tell how to position the point
    within stroking (see \ref
    StrokedPath::point::offset_vector()).
   */
  enum point_type_t
    {
      /*!
        The point is for an edge of the path.
       */
      edge_point,

      /*!
        The point is for a boundary point of a rounded join of the path
       */
      rounded_join_point,

      /*!
        The point is for a boundary point of a miter join of the path
       */
      miter_join_point,

      /*!
        The point is for a boundary point of a rounded cap of the path
       */
      rounded_cap_point,

      /*!
        The point is for a boundary point of a square cap of the path
       */
      square_cap_point,

      /*!
        The point is for a boundary point of a flat cap of the path
       */
      flat_cap_point,

      /*!
        The point is for a boundary point of a sqaure-cap join point.
        These points are for dashed stroking when the point of the join
        is NOT covered by the dash pattern. Their layout of data is the
        same as \ref miter_join_point. The purpose of this point type is
        to make sure caps of dashed stroking is drawn if a cap would be
        drawn in the area covered by a miter join with miter limit at
        0.5.
       */
      cap_join_point,

      /*!
        Number different point types with respect to rendering
       */
      number_point_types
    };

  /*!
    Enumeration encoding how bits of point::m_tag are used.
   */
  enum tag_bit_layout_t
    {
      point_type_num_bits = 4,
      point_type_bit0 = 0,

      normal0_y_sign_bit = point_type_bit0 + point_type_num_bits,
      normal1_y_sign_bit = normal0_y_sign_bit + 1,
      sin_sign_bit = normal1_y_sign_bit + 1,

      point_type_mask = FASTUIDRAW_MAX_VALUE_FROM_NUM_BITS(point_type_num_bits) << point_type_bit0,
      normal0_y_sign_mask = 1 << normal0_y_sign_bit,
      normal1_y_sign_mask = 1 << normal1_y_sign_bit,
      sin_sign_mask = 1 << sin_sign_bit,
    };

  /*!
    Enumeration to select what points of stroking to select.
   */
  enum point_set_t
    {
      /*!
        Select the set of points for edges
       */
      edge_point_set,

      /*!
        Select the set of points for bevel joins
       */
      bevel_join_point_set,

      /*!
        Select the set of points for rounded joins
       */
      rounded_join_point_set,

      /*!
        Select the set of points for miter joins
       */
      miter_join_point_set,

      /*!
        Select the set of points for cap joins
       */
      cap_join_point_set,

      /*!
        Select the set of points for square caps
       */
      square_cap_point_set,

      /*!
        Select the set of points for rounded caps
       */
      rounded_cap_point_set,

      /*!
        Select the set of points for flat caps
       */
      flat_cap_point_set,

      /*!
        Number point set types
       */
      number_point_set_types
    };

  /*!
    A point holds the data for a point of stroking.
    The data is so that changing the stroking width
    or miter limit does not change the stroking data.
   */
  class point
  {
  public:
    /*!
      The base position of a point, taken directly from
      the TessellatedPath from which the
      StrokedPath is constructed.
     */
    vec2 m_position;

    /*!
      Gives the offset vector used to help compute
      the point offset vector.
    */
    vec2 m_pre_offset;

    /*!
      Provides an auxilary offset data, used ONLY for
      StrokedPath::miter_join_point points.
     */
    vec2 m_auxilary_offset;

    /*!
      Gives the distance of the point from the start
      of the -edge- on which the point resides.
     */
    float m_distance_from_edge_start;

    /*!
      Gives the distance of the point from the start
      of the -contour- on which the point resides.
     */
    float m_distance_from_contour_start;

    /*!
      Has value -1, 0 or +1. If the value is 0,
      then the point is on the path. If the value has
      absolute value 1, then indicates a point that
      is on the boundary of the stroked path. The triangles
      produced from stroking are so that when
      m_on_boundary is interpolated across the triangle
      the center of stroking the value is 0 and the
      value has absolute value +1 on the boundary.
     */
    int m_on_boundary;

    /*!
      When stroking the data, the depth test to only
      pass when the depth value is -strictly- larger
      so that a fixed pixel is not stroked twice by
      a single path. The value m_depth stores
      a relative z-value for a vertex. The points
      drawn first have the largest z-values.
     */
    unsigned int m_depth;

    /*!
      Tag is a bit field where
       - m_tag & point_type_mask gives the value to point_type()
       - m_tag & normal0_y_sign_mask up if the y-component of n0 vector is negative (rounded join points only)
       - m_tag & normal1_y_sign_mask up if the y-component of n1 vector is negative (rounded join points only)
       - m_tag & sin_sign_mask  up if the y-component of sin value is negative (rounded join points only)
     */
    uint32_t m_tag;

    /*!
      Provides the point type for the point. The value is one of the
      enumerations of StrokedPath::point_type_t. NOTE: if a point comes
      from the geometry of an edge and the point is for a cap or join,
      it is the value StrokedPath::edge_point; a point of a cap or join
      is viewed as taking geometry from an edge if the point is shared
      with drawing the edge. As a side note, since the points of a bevel
      join are always shared geometry with an edge, the point type is
      StrokedPath::edge_point and thus there is no enumeration for bevel
      joins.
     */
    enum point_type_t
    point_type(void) const
    {
      return static_cast<enum point_type_t>(m_tag & point_type_mask);
    }

    /*!
      Computes the offset vector for a point. The final position
      of a point when stroking with a width of W is given by
      \code
      m_position + 0.5f * W * offset_vector().
      \endcode
      The computation for offset_vector() is as follows.
      - For those with point_type() being StrokedPath::edge_point,
        the offset is given by
        \code
        m_pre_offset
        \endcode
      - For those with point_type() being StrokedPath::square_cap_point,
        or StrokedPath::flat_cap_point the value is given by
        \code
        m_pre_offset + 0.5 * m_auxilary_offset
        \endcode
        However, for those of type StrokedPath::flat_cap_point, the
        value for W should be just thick enough to allow for anti-aliasing
      - For those with point_type() being StrokedPath::miter_join_point
        or StrokedPath::cap_join_point, the value is given by the following
        code
        \code
        vec2 n = m_pre_offset, v = vec2(-n.y(), n.x());
        float r, lambda;
        lambda = -sign(dot(v, m_auxilary_offset));
        r = (dot(m_pre_offset, m_auxilary_offset) - 1.0) / dot(v, m_auxilary_offset);
        //result:
        offset = lambda * (n - r * v);
        \endcode
        To enfore a miter limit M, clamp the value r to [-M,M].
      - For those with point_type() being StrokedPath::rounded_cap_point,
        the value is given by the following code
        \code
        vec2 n(m_pre_offset), v(n.y(), -n.x());
        offset = m_auxilary_offset.x() * v + m_auxilary_offset.y() * n;
        \endcode
      - For those with point_type() being StrokedPath::rounded_join_point,
        the value is given by the following code
        \code
        vec2 cs;

        cs.x() = m_auxilary_offset.y();
        cs.y() = sqrt(1.0 - cs.x() * cs.x());

        if(m_tag & sin_sign_mask)
          cs.y() = -cs.y();

        offset = cs
        \endcode
        In addition, the source data for join is encoded as follows:
        \code
        float t;
        vec2 n0, n1;

        t = m_auxilary_offset.x();
        n0.x() = m_offset.x();
        n1.x() = m_offset.y();

        n0.y() = sqrt(1.0 - n0.x() * n0.x());
        n1.y() = sqrt(1.0 - n1.x() * n1.x());

        if(m_tag & normal0_y_sign_mask)
          n0.y() = -n0.y();

        if(m_tag & normal1_y_sign_mask)
          n1.y() = -n1.y();
        \endcode
        The vector n0 represents the normal of the path going into the join,
        the vector n1 represents the normal of the path going out of the join
        and t represents how much to interpolate from n0 to n1.
     */
    vec2
    offset_vector(void);

    /*!
      When point_type() is miter_join_point, returns the distance
      to the miter point. For other point types, returns 0.0.
     */
    float
    miter_distance(void) const;
  };

  /*!
    Ctor. Construct a StrokedPath from the data
    of a TessellatedPath.
    \param P source TessellatedPath
   */
  explicit
  StrokedPath(const TessellatedPath &P);

  ~StrokedPath();

  /*!
    Returns the geomtric data for stroking the path. The backing data
    store for with and without closing edge data is shared so that
    \code
    points(tp, false) == points(tp, true).sub_array(0, points(tp,false).size())
    \endcode
    i.e., the geometric data for the closing edge comes at the end.
    \param tp what data to fetch, i.e. edge data, join data (which join data), etc.
    \param including_closing_edge if true, include the geometric data for of the
                                  closing edge. Asking for caps ignores the value
                                  for closing edge.
   */
  const_c_array<point>
  points(enum point_set_t tp, bool including_closing_edge) const;

  /*!
    Return the index data into as returned by points() for stroking
    the path. The backing data store for with and without closing edge
    data is shared so that
    \code
    unsigned int size_with, size_without;
    size_with = indices(tp, true);
    size_without  = indices(tp, false);
    assert(size_with >= size_without);
    assert(indices(tp, true).sub_array(size_with - size_without) == indices(tp, false))
    \endcode
    i.e., the index data for the closing edge is at the start of the
    index array.
    \param tp what data to fetch, i.e. edge data, join data (which join data), etc.
    \param including_closing_edge if true, include the index data for of the
                                  closing edge. Asking for caps ignores the value
                                  for closing edge.
   */
  const_c_array<unsigned int>
  indices(enum point_set_t tp, bool including_closing_edge) const;

  /*!
    Points returned by points(tp, including_closing_edge) have that
    their value for point::m_depth are in the half-open range
    [0, number_depth(tp, including_closing_edge))
    \param tp what data to fetch, i.e. edge data, join data (which join
              data), etc.
    \param including_closing_edge if true, include the index data for
                                  the closing edge.
   */
  unsigned int
  number_depth(enum point_set_t tp, bool including_closing_edge) const;

  /*!
    Returns the number of contours of the generating path.
   */
  unsigned int
  number_contours(void) const;

  /*!
    Returns the number of joins for the named contour
    of the generating path. Joint numbering is so that
    join A is the join that connects edge A to A + 1.
    In particular the joins of a closing edge of contour
    C are then at number_joins(C) - 2 and number_joins(C) - 1.
    \param contour which contour, with contour < number_contours().
   */
  unsigned int
  number_joins(unsigned int contour) const;

  /*!
    Returns the range into points(tp, true) for the
    indices of the named join of the named contour.
    \param tp what join type to query. If tp is not a type for a
              join, returns an empty range.
    \param contour which contour, with contour < number_contours().
    \param join which join with join < number_joins(contour)
   */
  range_type<unsigned int>
  join_points_range(enum point_set_t tp, unsigned int contour, unsigned int join) const;

  /*!
    Returns the range into indices(tp, true) for the
    indices of the named join of the named contour.
    \param tp what join type to query. If tp is not a type for a
              join, returns an empty range.
    \param contour which contour, with contour < number_contours().
    \param join which join with join < number_joins(contour)
   */
  range_type<unsigned int>
  join_indices_range(enum point_set_t tp, unsigned int contour, unsigned int join) const;

  /*!
    Returns data that can be passed to a PainterPacker
    to stroke a path.
   */
  const PainterAttributeData&
  painter_data(void) const;

private:
  void *m_d;
};

/*! @} */

}
