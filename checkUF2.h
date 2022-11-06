#pragma once

//return size UF2
unsigned char * getUF2Info(unsigned char * buffer,
                           unsigned int * returnSize)
{
    unsigned char * returnBuff = 0;
    unsigned int * intBuff = (unsigned int*)buffer;
    //printf("first number 0x%x, 0x%x, 0x%x\n",intBuff[0],intBuff[1], intBuff[142]);
    if(intBuff[0]   == 0x0A324655 &&
       intBuff[1]   == 0x9E5D5157 &&
       intBuff[127] == 0x0AB16F30)
    {
        printf("flags %x\n", intBuff[2]);
        printf("return size %x\n", intBuff[4]);
        *returnSize = intBuff[4];
        returnBuff = (unsigned char *)&buffer[32];
    }

    return returnBuff;
}