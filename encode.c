#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    for(int i = 2; i < 4; i++)
    {
        if(argv[i] == NULL)
        {
            printf("Invalid input\n");
            return e_failure;
        }
    }
    char *dot = strrchr(argv[2], '.');
    if(dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        printf("Error extension must be '.bmp' only\n");
        return e_failure;
    }
    encInfo -> src_image_fname = argv[2]; 
    
    encInfo -> secret_fname = argv[3];

    //Check argv[4] == NULL
    if(argv[4] == NULL)
    {
        encInfo -> stego_image_fname = "output.bmp";
    }
    else
    {
        char *out = strrchr(argv[4], '.');
        //Validate last 4 character is ".bmp" or not
        if(out == NULL || strcmp(out, ".bmp") != 0)
        {
            printf("Error extension must be '.bmp' only\n");
            return e_failure;
        }
        encInfo -> stego_image_fname = argv[4];
    }
    // Call open_files(encInfo)
    if(open_files(encInfo) == e_failure)
    {
        printf("Error extension must be '.bmp' onlFile doesnot opened\n");
        return e_failure;
    }
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    //Open source file in read 'r' mode
    encInfo -> fptr_src_image = fopen(encInfo -> src_image_fname, "r");
    //Validate file is opening  or not
    if(encInfo -> fptr_src_image == NULL)
    {
        printf("Source file is not opened.\n");
        return e_failure;
    } 

    //Open secret file in read 'r' mode
    encInfo -> fptr_secret = fopen(encInfo -> secret_fname, "r");
    //Validate file is opening  or not
    if(encInfo -> fptr_secret == NULL)
    {
        printf("Secret file is not opened.\n");
        return e_failure;
    }

    //Open output file in read 'w' mode
    encInfo -> fptr_stego_image = fopen(encInfo -> stego_image_fname, "w");
    //Validate file is opening  or not
    if(encInfo -> fptr_stego_image == NULL)
    {
        printf("Output file is not opened.\n");
        return e_failure;
    }
    printf("File is opened successfully\n");
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //Call check_capacity
    if(check_capacity(encInfo) == e_failure)
    {
        printf("Error.... Insufficient image capacity\n");
        return e_failure;
    }

    //Call copy_bmp_header
    if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_failure)
    {
        printf("Error.... Unable to copy the BMP header\n");
        return e_failure;
    }

    //Call encode_magic_string(const char *magic_string, EncodeInfo *encInfo) == e_failure
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("Error.... Unable to encode the magic string\n");
        return e_failure;
    }

    //Call encode secret file extention size
    if(encode_secret_file_extn_size(encInfo) == e_failure)
    {
        printf("Error.... Unable to encode the secret file extention size\n");
        return e_failure;
    }

    //Call encode secret file extension
    if(encode_secret_file_extn(encInfo -> extn_secret_file, encInfo) == e_failure)
    {
        printf("Error.... Unable to copy the BMP headerencode the secret file extention\n");
        return e_failure;
    }

    //Call encode secret file size
    if(encode_secret_file_size(encInfo -> size_secret_file, encInfo) == e_failure)
    {
        printf("Error.... Unable to copy the BMP headerencode secret file size\n");
        return e_failure;
    }

    //Call encode secret file data
    if(encode_secret_file_data(encInfo) == e_failure)
    {
        printf("Error.... Unable to encode the secret file data\n");
        return e_failure;
    }

    //Call copy remaining img data
    if(copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_failure)
    {
        printf("Error.... Unable to copy the remaining image data\n");
        return e_failure;
    }

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    //Get image capacity
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);

    //Get secret file size
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);

    //Check ((14 + size_secret_file) * 8) > image_capacity
    if(((14 + encInfo -> size_secret_file) * 8) > encInfo -> image_capacity)
    {
        return e_failure;
    }
    return e_success;
}

uint get_file_size(FILE *fptr)
{
    //Move the offset to last pos
    fseek(fptr, 0, SEEK_END);

    //return ftell()
    printf("File size = %lu\n",ftell(fptr));
    return ftell(fptr);
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    //Move the file pointer to the SEEK_SET
    rewind(fptr_src_image);
    //Declare the buff[54]
    char buff[54];
    //Read 54 bytes from src file
    if(fread(buff, 54, 1, fptr_src_image) != 1)
    {
        return e_failure;
    }
    //Write 54 bytes to dest file
    if(fwrite(buff, 54, 1, fptr_dest_image) != 1)
    {
        return e_failure;
    }
    printf("BMP file is copied successfully...\n");
    return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    //Declare the buff of 8 bytes
    char buff[8];
    for(int i = 0; magic_string[i] != 0; i++)
    {
        //Read 8 byte from src_file into buff
        fread(buff, 8, 1, encInfo -> fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buff);
        //Write the encoded buff to output_file
        fwrite(buff, 8, 1, encInfo -> fptr_stego_image);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 7; i >= 0; i--)
    {
        //Get the ith bit is set or not
        if((data >> i) & 1)
        {
            //If set, Set the LSB of image_buffer[]
            image_buffer[7-i] = image_buffer[7 - i] | 1;
        }
        else
        {
            //If clear, Clear the LSB of image_buffer[]
            image_buffer[7 - i] = image_buffer[7 - i] & (~1); 
        }
    }
    return e_success;
}

Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    char *dot = strrchr(encInfo -> secret_fname, '.');
    if(dot == NULL)
    {
        printf("Secret file extention is not found\n");
        return e_failure;
    }
    strcpy(encInfo -> extn_secret_file,dot);

    //Declare a buff[32]
    char buff[32];
    //Read 32 bytes from src file into buff
    fread(buff, 32, 1, encInfo -> fptr_src_image);
    //call encode_size_to_lsb(strlen(extn_secret_file), buff)
    if(encode_size_to_lsb(strlen(encInfo -> extn_secret_file), buff) == e_failure)
    {
        printf("Error...Secret file extn size failed\n");
        return e_failure;
    }
    //Write 32 bytes to output file
    fwrite(buff, 32, 1, encInfo -> fptr_stego_image);
    printf("Secret file extension size is encoded successfully\n");
    return e_success;
}

Status encode_size_to_lsb(int size,char *Image_buff)
{
    for(int i=31;i>=0;i--)
    {
        //get the ith bit is or not
        if((size >> i) & 1)
        {
            //If set ,set the LSB of image_buffer[]
            Image_buff[31 - i] = Image_buff[31 - i] | 1;
        }
        else
        {
            //clear the LSB of image_buffer[] 
            Image_buff[31 - i] = Image_buff[31 - i] & (~1);
        }
    }
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    //Declare buffer[8]
    char buff[8];
    for(int i =0; file_extn[i] != 0; i++)
    {
        //Read 8 bytes from src_image
        fread(buff, 8, 1, encInfo -> fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buff);
        //Write the 8 bytes of buff to output_file
        fwrite(buff, 8, 1, encInfo -> fptr_stego_image); 
    }
    printf("Secret file extension is encoded successfully\n");
    return e_success;
}

Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    //Declare the buff[32]
    char buff[32];
    //Read the 32 bytes from src_image
    fread(buff, 32, 1, encInfo -> fptr_src_image);
    encode_size_to_lsb(file_size, buff);
    //Write the 32 bytes of buff to output_file
    fwrite(buff, 32, 1, encInfo -> fptr_stego_image);

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    //Declare the buffer[8], data
    char buffer[8];
    char data;
    //Loop till EOF of secret_file
    while(fread(&data, 1, 1,encInfo -> fptr_secret) == 1)
    {
        //Read 8 bytes from src_file
        fread(buffer, 8, 1, encInfo -> fptr_src_image);
        //Read 1 byte from secret_file
        encode_byte_to_lsb(data, buffer);
        //write 8 bytes to output file
        fwrite(buffer, 8, 1, encInfo -> fptr_stego_image);
    }
    printf("Secret file dat is encoded successfully\n");
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    
    //Declare a char as data
    char data;

    //Loop till EOF of src_file and Read a char from src_file
    while(fread(&data, 1, 1, fptr_src) == 1)
    {
        //Write thebdata to dest_file
        fwrite(&data, 1, 1, fptr_dest);
    }
    printf("The remaining image data is copied successfully\n");     
    return e_success;
}