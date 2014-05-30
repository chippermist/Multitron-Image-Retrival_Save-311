MULTITRON IMAGE SAVE & RETRIVAL DRIVER
--------------------------------------
--------------------------------------

Multitron driver for binary images. It uses a binary image 3ii fictitious format. This driver saves the current image from the entire multitron and also opens the image and so on. The image format can be checked using the 4-byte hex in the starting of every file. 

You can view the complete hexdump using `hexdump -c image.3ii`
  


MORE ABOUT 3ii FORMAT
---------------------
The “3ii” format is a ﬁctitious image ﬁle format which stores three main pieces of information:
• A comment describing the image
• The image pixel data
• (Optional) A bitwise mask describing what parts of the pixel data to draw

