#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"

/**
Homework4
CSE455 Computer Vision SP20
Kexin Liu
**/


void draw_line(image im, float x, float y, float dx, float dy)
{
    /***********************************************************************
      This function draws a line on an image with color corresponding to the direction of line.
      image im: image to draw line on
      float x, y: starting point of line
      float dx, dy: vector corresponding to line angle and magnitude
    ************************************************************************/
    assert(im.c == 3);
    float angle = 6*(atan2(dy, dx) / TWOPI + .5);
    int index = floor(angle);
    float f = angle - index;
    float r, g, b;
    if(index == 0){
        r = 1; g = f; b = 0;
    } else if(index == 1){
        r = 1-f; g = 1; b = 0;
    } else if(index == 2){
        r = 0; g = 1; b = f;
    } else if(index == 3){
        r = 0; g = 1-f; b = 1;
    } else if(index == 4){
        r = f; g = 0; b = 1;
    } else {
        r = 1; g = 0; b = 1-f;
    }
    float i;
    float d = sqrt(dx*dx + dy*dy);
    for(i = 0; i < d; i += 1){
        int xi = x + dx*i/d;
        int yi = y + dy*i/d;
        set_pixel(im, xi, yi, 0, r);
        set_pixel(im, xi, yi, 1, g);
        set_pixel(im, xi, yi, 2, b);
    }
}


image make_integral_image(image im)
{
    image integ = make_image(im.w, im.h, im.c);
    /***********************************************************************
      This function makes an integral image or summed area table from an image.
      image im: image to process
      returns: image I such that I[x,y] = sum{i<=x, j<=y}(im[i,j])
    ************************************************************************/
    // TODO: fill in the integral image
        
    //I[x,y] = sum{i<=x, j<=y}(im[i,j])
    for(int i = 0; i < im.w; i++) {
        for(int j = 0; j < im.h; j++) {
            for(int k = 0; k < im.c; k++) {
                float ix, iy, ixy, i_old, i_new;
                i_old = get_pixel(im, i, j, k);
                if (i==0&&j==0) {
                    ixy = 0;
                    ix = 0;
                    iy = 0;
                }
                else if (i==0) {
                    ix = 0;
                    ixy = 0;
                    iy = get_pixel(integ,i,j-1,k);
                }
                else if (j==0) {
                    iy = 0;
                    ixy = 0;
                    ix = get_pixel(integ,i-1,j,k);
                }
                else {
                    ix = get_pixel(integ,i-1,j,k);
                    iy = get_pixel(integ,i,j-1,k);
                    ixy = get_pixel(integ,i-1,j-1,k);
                }
                
                i_new = i_old + ix + iy - ixy;
                set_pixel(integ, i, j, k, i_new);
            }
        }
    }
    
    return integ;
}


image box_filter_image(image im, int s)
{
    /***********************************************************************
      This function applies a box filter to an image using an integral image for speed.
      image im: image to smooth
      int s: window size for box filter
      returns: smoothed image
    ************************************************************************/
    int i,j,k;
    int top, bottom, right, left;
    float sum;
    float value;
    image integ = make_integral_image(im);
    image S = make_image(im.w, im.h, im.c);
    // TODO: fill in S using the integral image.
    
    
    float size = s * s;
    int offset = s / 2;
    
    for(i = 0; i < im.w; i++) {
        for (j = 0; j < im.h; j++) {
            
            right = i + offset;
            left = i - offset - 1;
            top = j - offset - 1;
            bottom = j + offset;
            for (k = 0; k < im.c; k++) {
                
                sum = get_pixel(integ, right, bottom, k) +
                       get_pixel(integ, left, top, k) -
                       get_pixel(integ, right, top, k) -
                       get_pixel(integ, left, bottom, k);
                value = sum / size;
                set_pixel(S, i, j, k, value);
            }
        }
    }
    
    return S;
}

image time_structure_matrix(image im, image prev, int s)
{
    /***********************************************************************
      This function calculates the time-structure matrix of an image pair.
      image im: the input image.
      image prev: the previous image in sequence.
      int s: window size for smoothing.
      returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
               3rd channel is IxIy, 4th channel is IxIt, 5th channel is IyIt.
    ************************************************************************/
    int i;
    int converted = 0;
    if(im.c == 3){
        converted = 1;
        im = rgb_to_grayscale(im);
        prev = rgb_to_grayscale(prev);
    }

    // TODO: calculate gradients, structure components, and smooth them
    
    image Ix = convolve_image(im, make_gx_filter(), 0);
    image Iy = convolve_image(im, make_gy_filter(), 0);
    image S = make_image(im.w, im.h, 5);
    
    int size = im.w * im.h;
    for (i = 0; i < size; i++) {
        float It = im.data[i] - prev.data[i];
        S.data[i] = Ix.data[i] * Ix.data[i];
        S.data[i + size] = Iy.data[i] * Iy.data[i];
        S.data[i + 2 * size] = Ix.data[i] * Iy.data[i];
        S.data[i + 3 * size] = Ix.data[i] * It;
        S.data[i + 4 * size] = Iy.data[i] * It;
    }
    
    S = box_filter_image(S, s);
    
    
    if(converted){
        free_image(im); free_image(prev);
    }
    return S;
}


image velocity_image(image S, int stride)
{
    /***********************************************************************
      This function calculates the velocity given a structure image.
      image S: time-structure image
      int stride: only calculate subset of pixels for speed
      returns: velocity of structure image.
    ************************************************************************/
    image v = make_image(S.w/stride, S.h/stride, 3);
    int i, j;
    matrix M = make_matrix(2,2);
    matrix It = make_matrix(2,1);
    for(j = (stride-1)/2; j < S.h; j += stride){
        for(i = (stride-1)/2; i < S.w; i += stride){
            float Ixx = S.data[i + S.w*j + 0*S.w*S.h];
            float Iyy = S.data[i + S.w*j + 1*S.w*S.h];
            float Ixy = S.data[i + S.w*j + 2*S.w*S.h];
            float Ixt = S.data[i + S.w*j + 3*S.w*S.h];
            float Iyt = S.data[i + S.w*j + 4*S.w*S.h];

            // TODO: calculate vx and vy using the flow equation
            float vx = 0;
            float vy = 0;
            
            M.data[0][0] = Ixx;
            M.data[0][1] = Ixy;
            M.data[1][0] = Ixy;
            M.data[1][1] = Iyy;

            It.data[0][0] = -Ixt;
            It.data[1][0] = -Iyt;
            
            matrix invert_M = matrix_invert(M);
            matrix Vel = make_matrix(2, 1);
            // data != 0
            if (invert_M.data) {
                Vel = matrix_mult_matrix(invert_M, It);
                vx = Vel.data[0][0];
                vy = Vel.data[1][0];
            }
            set_pixel(v, i/stride, j/stride, 0, vx);
            set_pixel(v, i/stride, j/stride, 1, vy);
        }
    }
    free_matrix(M);
    return v;
}


void draw_flow(image im, image v, float scale)
{
    /***********************************************************************
      This function draws lines on an image given the velocity.
      image im: image to draw on
      image v: velocity of each pixel
      float scale: scalar to multiply velocity by for drawing
    ************************************************************************/
    int stride = im.w / v.w;
    int i,j;
    for (j = (stride-1)/2; j < im.h; j += stride) {
        for (i = (stride-1)/2; i < im.w; i += stride) {
            float dx = scale*get_pixel(v, i/stride, j/stride, 0);
            float dy = scale*get_pixel(v, i/stride, j/stride, 1);
            if(fabs(dx) > im.w) dx = 0;
            if(fabs(dy) > im.h) dy = 0;
            draw_line(im, i, j, dx, dy);
        }
    }
}


// Constrain the absolute value of each image pixel
// image im: image to constrain
// float v: each pixel will be in range [-v, v]
void constrain_image(image im, float v)
{
    int i;
    for(i = 0; i < im.w*im.h*im.c; ++i){
        if (im.data[i] < -v) im.data[i] = -v;
        if (im.data[i] >  v) im.data[i] =  v;
    }
}

image smooth_image(image im, float sigma)
{
    if(1){
        image g = make_gaussian_filter(sigma);
        image s = convolve_image(im, g, 1);
        free_image(g);
        return s;
    } else {
        
        // TODO: optional, use two convolutions with 1d gaussian filter.
        // If you implement, disable the above if check.
        return copy_image(im);
    }
}
// Calculate the optical flow between two images
// image im: current image
// image prev: previous image
// int smooth: amount to smooth structure matrix by
// int stride: downsampling for velocity matrix
// returns: velocity matrix
image optical_flow_images(image im, image prev, int smooth, int stride)
{
    image S = time_structure_matrix(im, prev, smooth);
    image v = velocity_image(S, stride);
    constrain_image(v, 6);
    image vs = smooth_image(v, 2);
    free_image(v);
    free_image(S);
    return vs;
}

// Run optical flow demo on webcam
// int smooth: amount to smooth structure matrix by
// int stride: downsampling for velocity matrix
// int div: downsampling factor for images from webcam
void optical_flow_webcam(int smooth, int stride, int div)
{
#ifdef OPENCV
    void * cap;
    cap = open_video_stream(0, 0, 1280, 720, 30);
    image prev = get_image_from_stream(cap);
    image prev_c = nn_resize(prev, prev.w/div, prev.h/div);
    image im = get_image_from_stream(cap);
    image im_c = nn_resize(im, im.w/div, im.h/div);
    while(im.data){
        image copy = copy_image(im);
        image v = optical_flow_images(im_c, prev_c, smooth, stride);
        draw_flow(copy, v, smooth*div);
        int key = show_image(copy, "flow", 5);
        free_image(v);
        free_image(copy);
        free_image(prev);
        free_image(prev_c);
        prev = im;
        prev_c = im_c;
        if(key != -1) {
            key = key % 256;
            printf("%d\n", key);
            if (key == 27) break;
        }
        im = get_image_from_stream(cap);
        im_c = nn_resize(im, im.w/div, im.h/div);
    }
#else
    fprintf(stderr, "Must compile with OpenCV\n");
#endif
}
