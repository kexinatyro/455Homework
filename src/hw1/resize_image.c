#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"

/**
Homework1
CSE455 Computer Vision SP20
Keixn Liu
**/


/***********************************************************************
  We've been talking a lot about resizing and interpolation in class,
  now's your time to do it!
  In order to make grading easier, please only edit the files we mention to submit.
  You will submit the resize_image.c file on Canvas.
************************************************************************/


/******************************** Resizing *****************************
  To resize we'll need some interpolation methods and a function to create
  a new image and fill it in with our interpolation methods.
************************************************************************/

float nn_interpolate(image im, float x, float y, int c)
{
    // TODO
    /***********************************************************************
      This function performs nearest-neighbor interpolation on image "im"
      given a floating column value "x", row value "y" and integer channel "c",
      it interpolates and returns the interpolated value.
      Remember to use the closest "int", not just type-cast because in C that
      will truncate towards zero.
    ************************************************************************/
    return get_pixel(im, round(x), round(y), c);
}

image nn_resize(image im, int w, int h)
{
    // TODO Fill in (also fix the return line)
    /***********************************************************************
      This function uses nearest-neighbor interpolation on image "im" to
      construct a new image of size "w x h". Hint:
      - Create a new image that is "w x h" and the same number of channels as "im"
      - Loop over the pixels and map back to the old coordinates.
      - Use nearest-neighbor interpolate to fill in the image.
    ************************************************************************/
    // new image
    image result = make_image(w, h, im.c);
    // resize scale
    float scale_w = (float)im.w / w;
    float scale_h = (float)im.h / h;
    // resize function body
    for (int c = 0; c < im.c; c++){
        for (int y = 0; y < h; y++){
            for (int x = 0; x < w; x++){
                // match up coords: new_coord = a * old_coord + b
                // -0.5 * a + b = -0.5
                // a = scale
                // b = 0.5 * scale - 0.5
                // new = a * old + b = scale * old + (scale * 0.5 - 0.5)
                float old_w = scale_w * x + 0.5 * scale_w - 0.5;
                float old_h = scale_h * y + 0.5 * scale_h - 0.5;
                float old_value = nn_interpolate(im, old_w, old_h, c);
                set_pixel(result, x, y, c, old_value);
            }
        }
    }
    return result;
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    // TODO
    /***********************************************************************
      This function performs bilinear interpolation on image "im" given
      a floating column value "x", row value "y" and integer channel "c".
      It interpolates and returns the interpolated value.
    ************************************************************************/
    
    // find closest four piexels in a box
    // coords
    int up = floorf(y);
    int down = ceilf(y);
    int left = floorf(x);
    int right = ceilf(x);
    // pixel value
    float v1 = get_pixel(im, left, up, c);
    float v2 = get_pixel(im, right, up, c);
    float v3 = get_pixel(im, left, down, c);
    float v4 = get_pixel(im, right, down, c);
    // weight
    float d1 = x - left;
    float d2 = right - x;
    float d3 = y - up;
    float d4 = down - y;
    // weighted sum
    float q = v1 * d2 * d4 + v2 * d1 * d4 + v3 * d2 * d3 + v4 * d1 * d3;
    return q;
}

image bilinear_resize(image im, int w, int h)
{
    // TODO
    /***********************************************************************
      This function uses bilinear interpolation on image "im" to construct
      a new image of size "w x h". Hint:
      - Create a new image that is "w x h" and the same number of channels as "im".
      - Loop over the pixels and map back to the old coordinates.
      - Use bilinear interpolate to fill in the image.
    ************************************************************************/
    // new image
    image result = make_image(w, h, im.c);
    // resize scale
    float scale_w = (float)im.w / w;
    float scale_h = (float)im.h / h;
    // resize function body
    for (int c = 0; c < im.c; c++){
        for (int y = 0; y < h; y++){
            for (int x = 0; x < w; x++){
                // match up coords
                // new = a * old + b = sclae * new + (scale * 0.5 - 0.5)
                float old_w = scale_w * x + 0.5 * scale_w - 0.5;
                float old_h = scale_h * y + 0.5 * scale_h - 0.5;
                float old_value = bilinear_interpolate(im, old_w, old_h, c);
                set_pixel(result, x, y, c, old_value);
            }
        }
    }
    return result;
}



