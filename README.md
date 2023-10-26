# ImageMatchingBMP
Perform image matching of 24bit BMP from mspaint (Microsoft Paint) with screenshot for c code.<br>
A typical BMP format store pixel data from the bottom-left corner to the top-right corner.
```
456
123
```

Microsoft Paint stores BMP images in the Windows Device Independent Bitmap (DIB) format, which is typically stored in a bottom-up manner too. <br>
In this format, the pixel data is arranged from the bottom-left corner of the image to the top-right corner, row by row. <br>
Each row is padded to a multiple of 4 bytes if the width is not already a multiple of 4.
1. The first pixel is at the bottom-left corner.
2. Pixels are stored from left to right within each row.
3. Rows are stored from bottom to top.

The positive sign in bmiHeader.biHeight tells GetDIBits to save the pixel order in a bottom-up manner. 


Here's a summary of the code's functionality:

1. It loads a cropped image from a BMP file (assuming it's also in RGB format).
2. It captures a screenshot of the entire screen.
3. It iterates over the screenshot and the cropped image, comparing the RGB values.
4. If a match is found, it records the coordinates of the match.
