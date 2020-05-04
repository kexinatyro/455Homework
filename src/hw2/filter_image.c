#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853


/**
Homework2
CSE455 Computer Vision SP20
Kexin Liu
**/


/***************************** Box filter *******************************
  We want to create a box filter. We will only use square box filters.
  One way to do this is make an image,fill it in with all 1s, and then
  normalize it.
************************************************************************/

void l1_normalize(image im)
{
    // TODO
    /***********************************************************************
      This function divides each value in an image "im" by the sum of all the
      values in the image and modifies the image in place.
    ************************************************************************/
    float sum_all = 0;
    
    // compute the total value
    int total = im.w * im.h * im.c;
    for (int i = 0; i < total; i++) {
        sum_all += im.data[i];
    }
    //normalize
    if (sum_all == 0) { // denominator cannot be 0
        return;
    }
    for (int j = 0; j < total; j++) {
        im.data[j] /= sum_all;
    }
}

image make_box_filter(int w)
{
    // TODO
    /***********************************************************************
      This function makes a square filter of size "w x w". Hint:
      Change the "make_image" arguments so that it is a square image of
      width = height = w and number of channels = 1, with all entries equal
      to 1. Then use "l1_normalize" to normalize your filter.
    ************************************************************************/
    
    // new image all entries is 1
    image result = make_image(w, w, 1);
    for (int i = 0; i < w * w; i++) {
        result.data[i] = 1.0;
    }
    // normalize
    l1_normalize(result);
    return result;
}

image convolve_image(image im, image filter, int preserve)
{
    // TODO
    /***********************************************************************
    im: an image with shape "h x w x c"
    filter: a convolution filter with shape "k1 x k2 x k3". 
    Preserve: an integer, which is usually either 0 or 1.

    - If `filter` and `im` have the same number of channels then it's just a normal 
    convolution. We sum over spatial and channel dimensions and produce a 1 channel image. 
    UNLESS:
        If `preserve` is set to 1 we should produce an image with the same number of 
        channels as the input. This is useful if, for example, we want to run a box 
        filter over an RGB image and get out an RGB image. This means each channel in 
        the image will be filtered by the corresponding channel in the filter.
    If the `filter` only has one channel but `im` has multiple channels we want to
    apply the filter to each of those channels. Then we either sum between channels 
    or not depending on if `preserve` is set.

    Also, `filter` better have either the same number of channels as `im` or have one channel. 
    I check this with an `assert`.
    ************************************************************************/

    // same channels or only have 1 channel
    assert(filter.c == im.c || filter.c == 1);
    // create a new image with the same shape as image im
    image result = make_image(im.w, im.h, im.c);
    
    for (int c = 0; c < im.c; c++) {
        for (int y = 0; y < im.h; y++) {
            for (int x = 0; x < im.w; x++) {
                float q = 0;
                // loop over the filter for (x, y, c)
                for (int fx = 0; fx < filter.w; fx++) {
                    for (int fy = 0; fy < filter.h; fy++) {
                        float filter_value = get_pixel(filter, fx, fy, filter.c == 1 ? 0 : c);
                        int imx = x - filter.w / 2 + fx;
                        int imy = y - filter.h / 2 + fy;
                        float image_value = get_pixel(im, imx, imy, c);
                        q += filter_value * image_value;
                    }
                }
                set_pixel(result, x, y, c, q);
            }
        }
    }
    
    if (!preserve) {
        image new = make_image(im.w, im.h, 1);
        for (int x = 0; x < im.w; x++) {
            for (int y = 0; y < im.h; y++) {
                float v = 0;
                for (int c = 0; c < im.c; c++) {
                    v += get_pixel(result, x, y, c);
                }
                set_pixel(new, x, y, 0, v);
            }
        }
        return new;
    } else {
        return result;
    }
}

// helper method to make a 3*3 filter with given values
image filter_helper(float *array)
{
    image result = make_box_filter(3);
    for (int i = 0; i < 3 * 3; i++) {
        result.data[i] = array[i];
    }
    return result;
}

image make_highpass_filter()
{
    // TODO
    /***********************************************************************
    Create a 3x3 highpass filter and return it
    ************************************************************************/
    
    //image result = make_box_filter(3);
    float value[9] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
    /*
    for (int i = 0; i < 9; i++) {
        result.data[i] = value[i];
    }
     */
    return filter_helper(value);
}

image make_sharpen_filter()
{
    // TODO
    /***********************************************************************
    Create a 3x3 sharpen filter and return it
    ************************************************************************/
    
    //image result = make_box_filter(3);
    float value[9] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
    /*
    for (int i = 0; i < 9; i++) {
        result.data[i] = value[i];
    }
     */
    return filter_helper(value);
    //return make_image(1,1,1);
}

image make_emboss_filter()
{
    // TODO
    /***********************************************************************
    Create a 3x3 emboss filter and return it
    ************************************************************************/
    
    //image result = make_box_filter(3);
    float value[9] = {-2, -1, 0, -1, 1, 1, 0, 1, 2};
    /*
    for (int i = 0; i < 9; i++) {
        result.data[i] = value[i];
    }
     */
    return filter_helper(value);
    //return make_image(1,1,1);
}

// Question 2.2.1: Which of these filters should we use `preserve = 1` when we run our convolution and which ones should we not? Why?
// Answer: Sharpen and Emboss use 'preserve = 1', Highpass does not. Because when we use Sharpen and Emboss, we don't want
//         to lose color so we will use preserve

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: All of them need post-processing like clamp to make sure they won't be out of boundary

image make_gaussian_filter(float sigma)
{
    // TODO
    /***********************************************************************
    sigma: a float number for the Gaussian.
    
    Create a Gaussian filter with the given sigma. Note that the kernel size 
    is the next highest odd integer from 6x sigma.

    Return the Gaussian filter.
    ************************************************************************/
    
    // kernel size is next odd integer from 6xsigma
    // size = ceilf(6xsigma) is even then size += 1
    int size = (int)ceilf(6 * sigma);
    size = (size % 2) == 0 ? size + 1 : size;
    
    image result = make_image(size, size, 1);
    
    // the center coord value
    int midpoint = size / 2;
    // loop over the result
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            // compute probability density function for a 2D gaussian
            float g = 1 / (sigma * sigma * TWOPI) * \
            expf(-((x - midpoint) * (x - midpoint) + (y - midpoint) * (y - midpoint)) / (sigma * sigma * 2));
            set_pixel(result, x, y, 0, g);
        }
    }
    // all weights sum to 1:
    l1_normalize(result);
    
    return result;
}

image add_image(image a, image b)
{
    // TODO
    /***********************************************************************
    The input images a and image b have the same height, width, and channels.
    Sum the given two images and return the result. 
    The result image should also have the same height, width, and channels as the inputs.
    ************************************************************************/
    
    // a and b have the same size
    assert(a.w == b.w && a.h == b.h && a.c == b.c);
    // create result image
    image result = make_image(a.w, a.h, a.c);
    for (int i = 0; i < a.w * a.h * a.c; i++) {
        result.data[i] = a.data[i] + b.data[i];
    }
    return result;
}

image sub_image(image a, image b)
{
    // TODO
    /***********************************************************************
    The input image a and image b have the same height, width, and channels.
    Subtract the given two images (a - b) and return the result.
    The result image should also have the same height, width, and channels as the inputs.
    ************************************************************************/
    
    // a and b have the same size
    assert(a.w == b.w && a.h == b.h && a.c == b.c);
    // create result image
    image result = make_image(a.w, a.h, a.c);
    for (int i = 0; i < a.w * a.h * a.c; i++) {
        result.data[i] = a.data[i] - b.data[i];
    }
    return result;
}

image make_gx_filter()
{
    // TODO
    /***********************************************************************
    Create a 3x3 Sobel Gx filter and return it
    ************************************************************************/
    float value[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    return filter_helper(value);
}

image make_gy_filter()
{
    // TODO
    /***********************************************************************
    Create a 3x3 Sobel Gy filter and return it
    ************************************************************************/
    float value[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    return filter_helper(value);
}

image *sobel_image(image im)
{
    // TODO
    /***********************************************************************
    im is the input image with "h x w x 1".
    Apply Sobel filter to the given image, get the magnitude and gradient, 
    and return the result. 

    Hint: the "calloc" function can allocate the memory for your output. You can
    assess the first image (magnitute) by calling rst[0] and the second image 
    by calling rst[1]
    ************************************************************************/
    image *rst = calloc(2, sizeof(image));
    
    image gx_filter = make_gx_filter();
    image gy_filter = make_gy_filter();
    image gx = convolve_image(im, gx_filter, 0);
    image gy = convolve_image(im, gy_filter, 0);
    
    image magnitude = make_image(im.w, im.h, 1);
    image gradient = make_image(im.w, im.h, 1);
    
    for (int i = 0; i < im.w * im.h; i++) {
          magnitude.data[i] = sqrtf(gx.data[i] * gx.data[i] + gy.data[i] * gy.data[i]);
          gradient.data[i] = atan2(gy.data[i], gx.data[i]);
    }
    rst[0] = magnitude;
    rst[1] = gradient;
    
    return rst;
}


void normalize_image(image im)
{
    /***********************************************************************
    Calculate minimum and maximum pixel values. Normalize the image by
    subtracting the minimum and dividing by the max-min difference.
    This is a helper function to visualize the sobel magnitude image better.
    No TODO here :)
    ***********************************************************************/
    int i;
    float min = im.data[0];
    float max = im.data[0];
    for(i = 0; i < im.w*im.h*im.c; ++i){
        if(im.data[i] > max) max = im.data[i];
        if(im.data[i] < min) min = im.data[i];
    }
    for(i = 0; i < im.w*im.h*im.c; ++i){
        im.data[i] = (im.data[i] - min)/(max-min);
    }
}


// EXTRA CREDITS BELOW
int compare_float(const void * a, const void * b)
{
    // This function is provided for your convenience
    float fa = *(const float*) a;
    float fb = *(const float*) b;
    return (fa > fb) - (fa < fb);
}


image apply_median_filter(image im, int kernel_size)
{
    image out = make_image(im.w, im.h, im.c);

    // TODO (EXTRA CREDIT)
    /***********************************************************************
    im is the input image.
    kernel_size is a positive odd number.

    We assume a median filter is a square, with the same height and width.
    The kernel size is always a positive odd number. We use "clamp" padding
    for borders and corners. The output image should have the same width, 
    height, and channels as the input image. You should apply median filter
    to each channel of the input image `im`.

    Hint: use the qsort() function to sort an array. Make use of compare_float() as needed.
    ************************************************************************/
    
    // window contains pixels around (x,y,c) need to be sorted and find the median
    int window_size = kernel_size * kernel_size;
    float window[window_size];
    int halfsize = kernel_size / 2;
    // loop over the image
    for (int c = 0; c < im.c; c++) {
        for (int x = 0; x < im.w; x++) {
            for (int y = 0; y < im.h; y++) {
                // get_pixel to use clamp padding for borders and corners
                int k = 0;
                // loop over the window
                for (int i = 0; i < kernel_size; i++) {
                    for (int j = 0; j < kernel_size; j++) {
                        window[k] = get_pixel(im, x + i - halfsize, y + j - halfsize, c);
                        k++;
                    }
                }
                // sort window and get the median
                qsort(window, window_size, sizeof(float), compare_float);
                float value = window[window_size / 2];
                set_pixel(out, x, y, c, value);
            }
        }
    }
    
    return out;
}
