// Copyright (C) 2011  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#undef DLIB_SCAN_iMAGE_ABSTRACT_H__
#ifdef DLIB_SCAN_iMAGE_ABSTRACT_H__

#include <vector>
#include <utility>
#include "../algs.h"

namespace dlib
{

// ----------------------------------------------------------------------------------------

    template <
        typename image_array_type
        >
    bool all_images_same_size (
        const image_array_type& images
    );
    /*!
        requires
            - image_array_type       == an implementation of array/array_kernel_abstract.h
            - image_array_type::type == an implementation of array2d/array2d_kernel_abstract.h
        ensures
            - if (all elements of images have the same dimensions (i.e. 
              for all i and j: get_rect(images[i]) == get_rect(images[j]))) then
                - returns true
            - else
                - returns false
    !*/

// ----------------------------------------------------------------------------------------

    template <
        typename image_array_type
        >
    double sum_of_rects_in_images (
        const image_array_type& images,
        const std::vector<std::pair<unsigned int, rectangle> >& rects,
        const point& origin
    );
    /*!
        requires
            - image_array_type             == an implementation of array/array_kernel_abstract.h
            - image_array_type::type       == an implementation of array2d/array2d_kernel_abstract.h
            - image_array_type::type::type == a scalar pixel type (e.g. int rather than rgb_pixel)
            - all_images_same_size(images) == true
            - for all valid i: rects[i].first < images.size()
              (i.e. all the rectangles must reference valid elements of images)
        ensures
            - returns the sum of the pixels inside the given rectangles.  To be precise, 
              let RECT_SUM[i] = sum of pixels inside the rectangle translate_rect(rects[i].second, origin) 
              from the image images[rects[i].first].  Then this function returns the 
              sum of RECT_SUM[i] for all the valid values of i.
    !*/

// ----------------------------------------------------------------------------------------

    template <
        typename image_array_type
        >
    void scan_image (
        std::vector<std::pair<double, point> >& dets,
        const image_array_type& images,
        const std::vector<std::pair<unsigned int, rectangle> >& rects,
        const double thresh,
        const unsigned long max_dets
    );
    /*!
        requires
            - image_array_type             == an implementation of array/array_kernel_abstract.h
            - image_array_type::type       == an implementation of array2d/array2d_kernel_abstract.h
            - image_array_type::type::type == a scalar pixel type (e.g. int rather than rgb_pixel)
            - images.size() > 0
            - rects.size() > 0 
            - all_images_same_size(images) == true
            - for all valid i: rects[i].first < images.size()
              (i.e. all the rectangles must reference valid elements of images)
        ensures
            - slides the set of rectangles over the image space and reports the locations
              which give a sum bigger than thresh. 
            - Specifically, we have:
                - #dets.size() <= max_dets
                  (note that dets is cleared before new detections are added by scan_image())
                - for all valid i:
                    - #dets[i].first == sum_of_rects_in_images(images,rects,#dets[i].second) >= thresh
    !*/

// ----------------------------------------------------------------------------------------

}

#endif // DLIB_SCAN_iMAGE_ABSTRACT_H__



