how to compile:
    gcc cryptmeleon.c main.c rsg.c -o cryptmeleon

how to use :
 cryptmeleon -s -i <image_path> -> setup the image to use for encoding

 cryptmeleon -i <data_path> <data> -> encode data in <data_path>
 
 cryptmeleon -d -i <data_path> -> decode data in <data_path>

images created can be found in ~/.cryptmeleon/<data_path>
