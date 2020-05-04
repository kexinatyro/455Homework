/**
Homework0
CSE455 Computer Vision SP20
Keixn Liu
**/

/*
EXTRA CREDIT
SUPER EXTRA CREDIT
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

float get_pixel(image im, int x, int y, int c)
{
    // check bounds
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    if (x >= im.w) {
        x = im.w - 1;
    }
    if (y >= im.h) {
        y = im.h - 1;
    }
    if (c < 0) {
        c = 0;
    }
    if (c >= im.c) {
        c = im.c - 1;
    }
    return im.data[x + im.w * y + c * im.w * im.h];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    if ((x >= 0 && x < im.w) && (y >= 0 && y < im.h) && (c >=0 && c < im.c)) {
        im.data[x + im.w * y + c * im.w * im.h] = v;
    }
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    // memcpy(copy, src, size)
    memcpy(copy.data, im.data, im.w * im.h * im.c * sizeof(float));
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    for (int x = 0; x < im.w; x++) {
        for (int y = 0; y < im.h; y++) {
            gray.data[x + y * im.w] =
                    0.299*im.data[x + y*im.w]
                    + 0.587*im.data[x + y*im.w + im.w*im.h] +0.114*im.data[x
                    + y*im.w + 2*im.w*im.h];
        }
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    if (c < 0 || c >= im.c) {
        return;
    }
    for (int x = 0; x < im.w; x++) {
        for (int y = 0; y < im.h; y++) {
            im.data[x + y*im.w + c*im.w*im.h] += v;
        }
    }
}

void clamp_image(image im)
{
    for (int i = 0; i < im.w * im.h * im.c; i++) {
        if (im.data[i] > 1) {
            im.data[i] = 1.0;
       }
        if (im.data[i] < 0) {
            im.data[i] = 0.0;
        }
    }
}

float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    float c_max, c_mini, delta, r, g, b, h, s, v;
    for (int x = 0; x < im.w; x++) {
        for (int y = 0; y < im.h; y++) {
            r = im.data[x + y * im.w];
            g = im.data[x + y * im.w + im.w * im.h];
            b = im.data[x + y * im.w + 2 * im.w * im.h];
            c_max = three_way_max(r, g, b);
            c_mini = three_way_min(r, g, b);
            delta = c_max - c_mini;
            // H
            if (delta == 0) {
                h = 0.0;
            } else {
                if (c_max == r) {
                    h = (g - b) / delta;
                }
                else if (c_max == g) {
                    h = (b - r) / delta + 2;
                }
                else {
                    h = (r - g) / delta + 4;
                }
            }
            if (h < 0) {
                h = h / 6 + 1;
            } else {
                h /= 6;
            }

            // S
            if (c_max == 0) {
                s = 0.0;
            } else {
                s = delta / c_max;
            }

            // V
            v = c_max;

            set_pixel(im, x, y, 0, h);
            set_pixel(im, x, y, 1, s);
            set_pixel(im, x, y, 2, v);
        }
    }
}

void hsv_to_rgb(image im)
{
    float h, s, v, r, g, b, f, p, q, t;
    for (int x = 0; x < im.w; x++) {
        for (int y = 0; y < im.h; y++) {
            h = im.data[x + y * im.w];
            s = im.data[x + y * im.w + im.w * im.h];
            v = im.data[x + y * im.w + 2 * im.w * im.h];
            h *= 6;
            f = h - floor(h);
            p = v * (1 - s);
            q = v * (1 - s * (1 - f));
            t = v * (1 - s * f);
            if (h >= 0 && h < 1) {
                r = v; g = q; b = p;
            } else if (h >= 1 && h < 2) {
                g = v; r = t; b = p;
            } else if (h >= 2 && h < 3) {
                g = v; b = q; r = p;
            } else if (h >= 3 && h < 4) {
                b = v; g = t; r = p;
            } else if (h >= 4 && h < 5) {
                b = v; r = q; g = p;
            } else {
                r = v; b = t; g = p;
            }

            set_pixel(im, x, y, 0, r);
            set_pixel(im, x, y, 1, g);
            set_pixel(im, x, y, 2, b);
        }
    }
}

void scale_image (image im, int c, float v)
{
    for (int x = 0; x < im.w; x++) {
        for (int y = 0; y < im.h; y++) {
            im.data[x + y*im.w + c*im.w*im.h] *= v;
        }
    }
}

// rgb -> xyz -> luv -> lch
// https://www.easyrgb.com/en/math.php
void rgb_to_lch(image im)
{
    assert(im.c == 3);
    // rgb -> standard rgb
    for (int i = 0; i < im.c * im.h * im.w; i++){
        im.data[i] = im.data[i] > 0.04045 ? powf((im.data[i]+0.055)/1.055, 2.4) : im.data[i] / 12.92;
    }

    // srgb -> xyz
    image im_xyz = make_image(im.w, im.h, im.c);
    float mat_xyz[3][3] = {{0.4124, 0.3576, 0.1805},
                         {0.2126, 0.7152, 0.0722},
                         {0.0193, 0.1192, 0.9505}}; // scale matrix
    for (int i = 0; i < im.c; i++) {
        for (int j = 0; j < im.c; j++) {
            for (int k = 0; k < im.h * im.w; k++) {
                im_xyz.data[i * im.h * im.w + k] += mat_xyz[i][j] * im.data[j * im.h * im.w + k];
            }
        }
    }

    //xyz -> luv
    float var_u, var_v, var_y, ref_u, ref_v, l, u, v;
    for (int i = 0; i < im.w; i++) {
        for (int j = 0; j < im.h; j++) {
            float x, y, z;
            x = get_pixel(im_xyz, i, j, 0);
            y = get_pixel(im_xyz, i, j, 1);
            z = get_pixel(im_xyz, i, j, 2);
            var_u = (4 * x) / (x + (15 * y) + (3 * z));
            var_v = (9 * y) / (x + (15 * y) + (3 * z));
            var_y = y / 100;
            if (var_y > 0.008856) {
                var_y = powf(var_y, (1 / 3));
            }
            else {
                var_y = (7.787 * var_y) + (16 / 116);
            }
            ref_u = (4 * x) / (x + (15 * y) + (3 * z));
            ref_v = (9 * x) / (x + (15 * y) + (3 * z));
            l = (116 * var_y) - 16;
            u = 13 * l * (var_u - ref_u);
            v = 13 * l * (var_v - ref_v);
            set_pixel(im, i, j, 0, l);
            set_pixel(im, i, j, 1, u);
            set_pixel(im, i, j, 2, v);
        }
    }
}
