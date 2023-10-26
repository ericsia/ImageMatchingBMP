#include <stdio.h>   // Include standard input/output library for printing messages
#include <stdlib.h>  // Include standard library for memory management
#include <windows.h> // Include the Windows API for screen capture

int main()
{
    FILE *log = fopen("log.txt", "w");

    // Load the BMP image from Microsoft Paint 24-bit BMP
    int croppedWidth, croppedHeight;

    FILE *file = fopen("cropped.bmp", "rb"); // Open the BMP image file in binary read mode
    if (!file)                               // Check if the file couldn't be opened
    {
        fprintf(log, "Failed to open the cropped image file.\n"); // Print an error message to the standard error stream
        return 1;                                                    // Return an error code and exit
    }

    // Read the BMP header to get image dimensions
    fseek(file, 18, SEEK_SET);                   // Move the file pointer to the width field in the BMP header
    fread(&croppedWidth, sizeof(int), 1, file);  // Read the width of the image
    fseek(file, 22, SEEK_SET);                   // Move the file pointer to the height field in the BMP header
    fread(&croppedHeight, sizeof(int), 1, file); // Read the height of the image

    // Calculate the size of the cropped image
    int cropWidthRGB = 3 * croppedWidth;                                     // Calculate the size of one row RGB of the image
    int croppedImageSize = cropWidthRGB * croppedHeight;                     // Calculate the total size of the image needed
    unsigned char *croppedArray = (unsigned char *)malloc(croppedImageSize); // Allocate memory to store the image
    if (!croppedArray)                                                       // Check if memory allocation failed
    {
        fclose(file);                                                          // Close the image file
        fprintf(log, "Failed to allocate memory for the cropped image.\n"); // Print an error message
        return 1;                                                              // Return an error code and exit
    }

    // Read the pixel data of the cropped image
    fseek(file, 54, SEEK_SET);                      // Move the file pointer to the start of the pixel data
    fread(croppedArray, 1, croppedImageSize, file); // Read the pixel data into the allocated memory
    fclose(file);                                   // Close the image file

    // Capture a 1920x1080 screenshot
    int screenWidth = 1920;
    int screenHeight = 1080;
    HDC hScreenDC = GetDC(NULL); // Get a handle to the screen device context

    // Create a compatible DC and bitmap to store the screenshot
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);                                        // Create a compatible device context
    HBITMAP hBitmapScreen = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight); // Create a compatible bitmap
    SelectObject(hMemoryDC, hBitmapScreen);                                               // Select the bitmap into the device context
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);         // Copy the screen content to the bitmap

    // Get the pixel data of the screenshot
    BITMAPINFOHEADER bmiHeader;
    bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmiHeader.biWidth = screenWidth;
    bmiHeader.biHeight = screenHeight; // Use a positive value
    bmiHeader.biPlanes = 1;
    bmiHeader.biBitCount = 24;
    bmiHeader.biCompression = BI_RGB;

    int screenImageSize = screenWidth * 3 * screenHeight;                                                            // Calculate the size of RGB of the screenshot
    unsigned char *screenshotArray = (unsigned char *)malloc(screenImageSize);                                       // Allocate memory to store the screenshot
    GetDIBits(hScreenDC, hBitmapScreen, 0, screenHeight, screenshotArray, (BITMAPINFO *)&bmiHeader, DIB_RGB_COLORS); // Retrieve the pixel data

    // Clean up screenshot resources
    DeleteObject(hBitmapScreen); // Delete the bitmap
    DeleteDC(hMemoryDC);         // Delete the device context
    ReleaseDC(NULL, hScreenDC);  // Release the screen device context

    // Find the exact match
    int screenXMax = screenWidth - croppedWidth;
    int screenYMax = screenHeight - croppedHeight;
    int matchX = -1;
    int matchY = -1;

    // Iterate through the screenshot and find the matching region
    for (int screenY = 0; screenY <= screenYMax; screenY++)
    {
        for (int screenX = 0; screenX <= screenXMax; screenX++)
        {
            int match = 1;
            // Compare pixel blocks using memcmp for faster comparison
            for (int croppedY = 0; croppedY < croppedHeight; croppedY++)
            {
                int offsetCropped = cropWidthRGB * croppedY;                           // Calculate the starting position in the cropped image
                int offsetScreen = 3 * ((screenY + croppedY) * screenWidth + screenX); // Calculate the starting position in the screenshot
                // Compare entire pixel blocks using memcmp
                if (memcmp(&screenshotArray[offsetScreen], &croppedArray[offsetCropped], cropWidthRGB) != 0)
                {
                    match = 0; // Not a match
                    break;
                }
            }

            if (match)
            {
                matchX = screenX;                                // Record the x-coordinate of the match
                matchY = screenHeight - croppedHeight - screenY; // Record the y-coordinate of the match
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
    if (matchX != -1)
    {
        fprintf(matchFile, "Image match found at (%d, %d)\n", matchX, matchY); // Print the coordinates to the file
    }
    else
    {
        fprintf(matchFile, "Image match not found.\n"); // Print a message indicating no match
    }
    fclose(matchFile); // Close the file
    fclose(log);       // Close the file


    // Clean up memory
    free(croppedArray);    // Free the memory allocated for the cropped image
    free(screenshotArray); // Free the memory allocated for the screenshot

    return 0; // Exit the program
}
