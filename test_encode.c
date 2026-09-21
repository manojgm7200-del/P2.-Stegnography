#include <stdio.h>
#include "encode.h"
#include "types.h"

int main(int argc,char *argv[])
{
    EncodeInfo encInfo;
    //->Call check_operation_type(argv[1][1]==e_encode)
    /* -> call read_and_validate_encode
    */


    return 0;
}

OperationType check_operation_type(char *argv[]);
{
    /*check opt is 'e' 
    return e_encode;
    *check opt is 'd'
    return e_decode;
    *else
    return e_unsupported;
    */
   if(opt == 'e')
   {
    return e_e_encode;
   }
   else if(opt == 'd')
   {
    return e_decode;
   }
   else
   {
    return e_unsupported;
   }


}

