#include <stdio.h>
#include <stdlib.h>
#include <windows.h> // For taking a screenshot on Windows

// Function to load a BMP image into memory
unsigned char *loadBMP(const char *filename, int *width, int *height)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        fprintf(stderr, "Failed to open file\n");
        return NULL;
    }

    // Read BMP header
    fseek(file, 18, SEEK_SET);
    fread(width, sizeof(int), 1, file);
    fseek(file, 22, SEEK_SET);
    fread(height, sizeof(int), 1, file);

    int widthRGB = *width * 3;
    int imageSize = widthRGB * *height;
    unsigned char *data = (unsigned char *)malloc(imageSize);
    if (!data)
    {
        fclose(file);
        return NULL;
    }

    int padding = (4 - (widthRGB & 3)) & 3;

    fseek(file, 54, SEEK_SET);
    for (int i = 0; i < *height; ++i)
    {
        fread(data + i * widthRGB, 1, widthRGB, file);
        fseek(file, padding, SEEK_CUR); // Skip padding bytes
    }

    fclose(file);

    return data;
}

int pad(int width)
{
    int paddingSize = 0;
    if ((width * 3) % 4 != 0)
    {
        paddingSize = 4 - ((width * 3) % 4);
    }
    return (width * 3 + paddingSize);
}

int main()
{
    int cropWidth, cropHeight;
    unsigned char *croppedArray = loadBMP("crop.bmp", &cropWidth, &cropHeight);
    int strideCrop = 3 * cropWidth;

    int screenWidth, screenHeight;
    int strideScreen = 3 * screenWidth;
    unsigned char *screenArray = loadBMP("screen.bmp", &screenWidth, &screenHeight);

    int screenXMax = screenWidth - cropWidth;
    int screenYMax = screenHeight - cropHeight;
    int matchX = -1;
    int matchY = -1;
    printf("%d %d\n", screenXMax, screenYMax);

    // Iterate through the screenshot and find the matching region
    for (int screenY = 0; screenY <= screenYMax; screenY++)
    {
        for (int screenX = 0; screenX <= screenXMax; screenX++)
        {
            int match = 1;
            // Compare pixel blocks using memcmp for faster comparison
            for (int croppedY = 0; croppedY < cropHeight; croppedY++)
            {
                int offsetCropped = strideCrop * croppedY;                             // Calculate the starting position in the cropped image
                int offsetScreen = 3 * ((screenY + croppedY) * screenWidth + screenX); // Calculate the starting position in the screenshot
                // Compare entire pixel blocks using memcmp
                if (memcmp(&screenArray[offsetScreen], &croppedArray[offsetCropped], strideCrop) != 0)
                {
                    match = 0; // Not a match
                    break;
                }
            }

            if (match)
            {
                matchX = screenX;                             // Record the x-coordinate of the match
                matchY = screenHeight - cropHeight - screenY; // Record the y-coordinate of the match
                break;
            }
        }

        if (matchX >= 0) // post-checking, if not found proceed to next row
        {
            break;
        }
    }

    // Save match coordinates to a file
    FILE *matchFile = fopen("match_coordinates.txt", "w"); // Open a file to write the match coordinates
    if (matchX >= 0)
    {
        fprintf(matchFile, "Image match found at (%d, %d)\n", matchX, matchY); // Print the coordinates to the file
    }
    else
    {
        fprintf(matchFile, "Image match not found.\n"); // Print a message indicating no match
    }
    fclose(matchFile); // Close the file

    return 0;
}
