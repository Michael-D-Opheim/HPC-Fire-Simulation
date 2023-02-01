//
//  colorIt - A color mapping program for the Yellowstone Fire Visualization project
//
//  Created by Michael Opheim on 1/22/23.

#include <stdio.h>
#include <stdlib.h>
#include "colorIt.h"

// Coloring library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

int main(int argc, char **argv) {
    
    // Color dimensions and variables
    int width, height;
    int channels = 3;        // R G B
    char value;
    int fileNum = 1;
    
    // Default image dimensions
    width = height = 1000;
    
    if (argc < 1) {
        fileNum = atoi(argv[1]);
    }
    
    // Loop through all data files
    for (int i = 0; i <= fileNum; i++) {
        unsigned char imageData[width*height*3];
        int imagePos = 0;
        
        // Opening file in reading mode
        ssize_t bufSize = snprintf(NULL, 0, "ADD PATH HERE...", i);
        char* dataFilePath = malloc(bufSize + 1);
        snprintf(dataFilePath, bufSize + 1, "ADD PATH HERE...", i);
    
        FILE *fptr = fopen(dataFilePath, "r");
        if (NULL == fptr) {
            printf("file can't be opened \n");
        }
        
        // Printing what is written in the file, character by character, using a loop
        do {
            value = fgetc(fptr);
            
            // Color map
            if (value == '0'){
                imageData[imagePos++] = 47;
                imageData[imagePos++] = 79;
                imageData[imagePos++] = 79;
            } else if (value == '1'){
                imageData[imagePos++] = 54;
                imageData[imagePos++] = 135;
                imageData[imagePos++] = 24;
            }
            else if (value == '2') {
                imageData[imagePos++] = 96;
                imageData[imagePos++] = 184;
                imageData[imagePos++] = 35;
            }
            else if (value == '3'){
                imageData[imagePos++] = 133;
                imageData[imagePos++] = 199;
                imageData[imagePos++] = 54;
            }
            else if(value == '-'){
                char next = fgetc(fptr);
                if (next == '1') {
                    imageData[imagePos++] = 250;
                    imageData[imagePos++] = 147;
                    imageData[imagePos++] = 147;
                } else if(next == '2') {
                    imageData[imagePos++] = 255;
                    imageData[imagePos++] = 79;
                    imageData[imagePos++] = 79;
                } else if (next == '3') {
                    imageData[imagePos++] = 255;
                    imageData[imagePos++] = 23;
                    imageData[imagePos++] = 23;
                }
            }
        } while (value != EOF); // If the character is EOF, then stop reading
        
        // Allocate memory
        ssize_t imageSize = snprintf(NULL, 0, "image%d.png", i);
        char* imageFilePath = malloc(imageSize + 1);
        snprintf(imageFilePath, bufSize + 1, "image%d.png", i);
        
        // Write a PNG image
        int rowsize = width*channels; // Bytes per row (3 per pixel)
        stbi_write_png(imageFilePath, width, height, channels, imageData, rowsize);
        fclose(fptr); // Closing the file
    }
}
