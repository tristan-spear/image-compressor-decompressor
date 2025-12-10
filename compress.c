#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;
struct tagBITMAPFILEHEADER
{
    WORD bfType; //specifies the file type
    DWORD bfSize; //specifies the size in bytes of the bitmap file
    WORD bfReserved1; //reserved; must be 0
    WORD bfReserved2; //reserved; must be 0
    DWORD bfOffBits; //species the offset in bytes from the bitmapfileheader to the bitmap bits
};
struct tagBITMAPINFOHEADER
{
    DWORD biSize; //specifies the number of bytes required by the struct
    LONG biWidth; //specifies width in pixels
    LONG biHeight; //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage; //size of image in bytes
    LONG biXPelsPerMeter; //number of pixels per meter in x axis
    LONG biYPelsPerMeter; //number of pixels per meter in y axis
    DWORD biClrUsed; //number of colors used by th ebitmap
    DWORD biClrImportant; //number of colors that are important
};

typedef struct frequencyNode
{
    int key; // the value (0-255) that this node represents
    LONG frequency; // the frequency that the value occurs
    BYTE nodeType; // 1 if combinedNode, 2 if not used
    struct frequencyNode *left;
    struct frequencyNode *right;
} frequencyNode;

typedef struct codeNode
{
    int key;
    int codePtr;
    BYTE code[100];
} codeNode;

struct tagBITMAPINFOHEADER *infoHeader;

void fileRead(struct tagBITMAPFILEHEADER *fileHead, struct tagBITMAPINFOHEADER *fileInfo, FILE *fp)
{
    fread(&(fileHead -> bfType), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfSize), sizeof(DWORD), 1, fp);
    fread(&(fileHead -> bfReserved1), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfReserved2), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfOffBits), sizeof(DWORD), 1, fp);
    fread(fileInfo, sizeof(struct tagBITMAPINFOHEADER), 1, fp);
}

void fileWrite(struct tagBITMAPFILEHEADER *fileHead, struct tagBITMAPINFOHEADER *fileInfo, LONG *bitPtr, int* sizes, int *blue, int *green, int *red, codeNode *blueCode, codeNode *greenCode, codeNode *redCode, BYTE *data, FILE *fp)
{
    fwrite(&(fileHead -> bfType), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfSize), sizeof(DWORD), 1, fp);
    fwrite(&(fileHead -> bfReserved1), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfReserved2), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfOffBits), sizeof(DWORD), 1, fp);
    fwrite(fileInfo, sizeof(struct tagBITMAPINFOHEADER), 1, fp);
    fwrite(bitPtr, sizeof(LONG), 1, fp);
    fwrite(sizes, sizeof(int), 3, fp);
    fwrite(blue, sizeof(int), 256, fp);
    fwrite(green, sizeof(int), 256, fp);
    fwrite(red, sizeof(int), 256, fp);
    fwrite(blueCode, sizeof(codeNode), sizes[0], fp);
    fwrite(greenCode, sizeof(codeNode), sizes[1], fp);
    fwrite(redCode, sizeof(codeNode), sizes[2], fp);
    fwrite(data, (*bitPtr / 8) + 1, 1, fp);
}

void findFrequencies(BYTE *arr, DWORD size, int *frequency, int offset, LONG trueWidth, int qualityVal)
{
    for(int y = 0; y < infoHeader->biHeight; ++y)
        for(int x = 0; x < infoHeader->biWidth; ++x)
        {
            int currVal = arr[y*trueWidth + x*3 + offset];

            // adjust for quality
            BYTE qualityBit = currVal - (currVal % qualityVal);
            frequency[qualityBit]++;
        }   
}

// int getSize(int *freqArr)
// {
//     int size = 0;

//     for(int i = 0; i < 256; ++i)
//         if(freqArr[i] != 0)
//             ++size;
    
//     return size;
// }

// fill and sort node array
// void sortNodes(frequencyNode *nodeArr, int *frequencies)
// {
//     int nodeItr = 0;
//     for (int i = 0; i < 256; i++)
//     {
//         if(frequencies[i] != 0)
//         {
//             nodeArr[nodeItr].key = i;
//             nodeArr[nodeItr].frequency = frequencies[i];
//             nodeArr[nodeItr].nodeType = 0;
//             nodeItr++;
//         }
//     }

//     // // insertion sort (descending order based on frequencies)
//     // int j;
//     // for(int i = 1; i < 256; ++i)
//     // {
//     //     j = i;
//     //     while(j > 0 && nodeArr[j].frequency > nodeArr[j-1].frequency)
//     //     {
//     //         frequencyNode temp = nodeArr[j-1];
//     //         nodeArr[j-1] = nodeArr[j];
//     //         nodeArr[j] = temp;
//     //         --j;
//     //     }
//     // }
// }

// fill and sort node array
void sortPointers(frequencyNode **ptrArr, int *frequencies)
{
    int ptrItr = 0;
    for (int i = 0; i < 256; i++)
    {
        if(frequencies[i] != 0)
        {
            frequencyNode *currPtr = ptrArr[ptrItr];
            currPtr->key = i;
            currPtr->frequency = frequencies[i];
            currPtr->nodeType = 0;
            ptrItr++;
        }
    }

    // insertion sort (descending order based on frequencies)
    int j;
    for(int i = 1; i < 256; ++i)
    {
        j = i;
        while(j > 0 && ptrArr[j]->frequency > ptrArr[j-1]->frequency)
        {
            frequencyNode *temp = ptrArr[j-1];
            ptrArr[j-1] = ptrArr[j];
            ptrArr[j] = temp;
            --j;
        }
    }
}

int getSize(frequencyNode **ptrArr)
{
    int size = 0;
    int i = 0;

    while(ptrArr[size]->nodeType != 2)
        ++size;
            
    return size;
}

void getKeys(codeNode *codeArr, frequencyNode **ptrArr, int size)
{
    for(int i = 0; i < size; ++i)
        codeArr[i].key = ptrArr[i]->key;
}

// void insertElement(frequencyNode **ptrArr, frequencyNode *insertNode, int endOfList)
// {
//     ptrArr[endOfList] = insertNode;

//     while(endOfList > 0)
//     {
//         if(ptrArr[endOfList]->frequency > ptrArr[endOfList - 1]->frequency)
//         {
//             frequencyNode* temp = ptrArr[endOfList - 1];
//             ptrArr[endOfList - 1] = ptrArr[endOfList];
//             ptrArr[endOfList] = temp;
//             --endOfList;
//         }
//         else 
//         {
//             endOfList = 0;
//         }
//     }

//     // frequencyNode *temp = ptrArr[i];
//     // ptrArr[i] = insertNode;
//     // ++i;

//     // while(ptrArr[i]->nodeType != 2)
//     // {
//     //     ptrArr[i] = temp;
//     //     temp = ptrArr[i+1];
//     //     ++i;
//     // }
//     // ptrArr[i] = temp;
// }

// void buildHuffman(frequencyNode **ptrArr, int size)
// {
//     int endOfList = size - 1;

//     while(endOfList > 1)
//     {
//         frequencyNode *internalNode = (frequencyNode*)malloc(sizeof(frequencyNode));;
//         internalNode->left = ptrArr[endOfList];
//         internalNode->right = ptrArr[endOfList - 1];
//         internalNode->frequency = ptrArr[endOfList]->frequency + ptrArr[endOfList - 1]->frequency;   
//         internalNode->nodeType = 1;// set nodeType to 1 so we know this is an internal node

//         --endOfList;
//         // insert the combined node and make sure its children are no longer in the list
//         insertElement(ptrArr, internalNode, endOfList);
//     }
// }

void buildHuffman(frequencyNode **ptrArr, int endOfList) 
{
    while (endOfList > 1) 
    {
        frequencyNode *left  = ptrArr[endOfList-1];
        frequencyNode *right = ptrArr[endOfList-2];
        frequencyNode *parent = malloc(sizeof(frequencyNode));
        parent->left = left;
        parent->right = right;
        parent->frequency = left->frequency + right->frequency;
        parent->nodeType = 1;
        endOfList -= 2; // shrink list by 2

        // insert parent into array keeping descending order
        int i = endOfList;
        while (i > 0 && ptrArr[i-1]->frequency < parent->frequency) 
        {
            ptrArr[i] = ptrArr[i-1];
            --i;
        }
        ptrArr[i] = parent;
        ++endOfList;
    }
}



int findBitPattern(frequencyNode *head, codeNode *searchNode, BYTE *temp, int arrPtr, BYTE binaryVal)
{
    if(head != NULL)
    {
        if(binaryVal != 's') 
        {
            temp[arrPtr] = binaryVal;
            arrPtr++;
        }
        
        if(head->nodeType == 1)
        {
            if(findBitPattern(head->left, searchNode, temp, arrPtr, 0) == 1)
            return 1;
            
            else
            return (findBitPattern(head->right, searchNode, temp, arrPtr, 1));
        }
        
        else if(head->key == searchNode->key)
        {
            memcpy(searchNode->code, temp, arrPtr);
            searchNode->codePtr = arrPtr;
            return 1;
        }
        
        else return 0;
        
    }
    
    return 0;
}

void getCodes(codeNode *codeArr, frequencyNode *head, int *freqArr, int size)
{
    for(int i = 0; i < size; ++i)
    {
        BYTE temp[100];
        findBitPattern(head, &codeArr[i], temp, 0, 's'); // use 's' so the method knows this is the starting call 
    }
}

void putBit(BYTE *workOn, LONG *bitPtr, BYTE insert)
{
    int bitPosition = *bitPtr % 8;
    insert <<= 8 - 1 - bitPosition;
    *workOn |= insert;
}

void putBitPattern(BYTE *data, codeNode code, LONG *bitPtr)
{
    for(int i = 0; i < code.codePtr; ++i)
    {
        BYTE bit = code.code[i];
        
        if(bit == 1)
        {
            LONG idx = *bitPtr / 8;
            putBit(&data[idx], bitPtr, bit);
        }
        ++(*bitPtr);
    }
}

codeNode getBitPattern(BYTE target, codeNode *searchArr)
{
    int i = 0;

    while(searchArr[i].key != target)
        ++i;

    return searchArr[i];
}

void writeCompressedData(BYTE *origData, BYTE *compressedData, LONG size, LONG trueWidth, LONG byteWidth, LONG *bitPtr, codeNode *blue, codeNode *green, codeNode *red, int qualityVal)
{

    for(int y = 0; y < infoHeader->biHeight; ++y)
    {
        for(int x = 0; x < (infoHeader->biWidth * 3); ++x)
        {
            codeNode target;
            BYTE workOn = origData[y*trueWidth + x];
            BYTE qualityWorkOn = workOn - (workOn % qualityVal);

            // insert blue
            if(x % 3 == 0)
                target = getBitPattern(qualityWorkOn, blue);

            // insert green
            else if(x % 3 == 1)
                target = getBitPattern(qualityWorkOn, green);

            // insert red
            else
                target = getBitPattern(qualityWorkOn, red);

            putBitPattern(compressedData, target, bitPtr);
        }
    }
}

int main(int argc, char *argv[])
{
    struct tagBITMAPFILEHEADER *fileHeader = (struct tagBITMAPFILEHEADER*)malloc(sizeof(struct tagBITMAPFILEHEADER));
    infoHeader = (struct tagBITMAPINFOHEADER*)malloc(sizeof(struct tagBITMAPINFOHEADER));

    char *inputName = argv[1];
    char outputName[strlen(inputName)];
    strcpy(outputName, inputName);
    int qualityInput = atoi(argv[2]);
    int qualityVal = 31 - 3 * qualityInput;

    // give output file a .slo extension
    outputName[strlen(inputName) - 3] = 's';
    outputName[strlen(inputName) - 2] = 'l';
    outputName[strlen(inputName) - 1] = 'o';

    FILE *inputFP = fopen(inputName, "rb");
    FILE *outputFP = fopen(outputName, "wb");
    
    fileRead(fileHeader, infoHeader, inputFP);

    LONG byteWidth = (infoHeader -> biWidth) * 3;
    int padding = (4 - ((byteWidth) % 4)) % 4;
    LONG trueWidth = byteWidth + padding;
    DWORD trueSize = trueWidth * (infoHeader -> biHeight);
    BYTE byteArr[trueSize];
    fread(byteArr, trueSize, 1, inputFP);
    int blueFreqArr[256];
    int greenFreqArr[256];
    int redFreqArr[256];

    for(int i = 0; i < 256; ++i)
    {
        blueFreqArr[i] = 0;
        greenFreqArr[i] = 0;
        redFreqArr[i] = 0;
    }

    frequencyNode blueNodeArr[1000];
    frequencyNode greenNodeArr[1000];
    frequencyNode redNodeArr[1000];

    frequencyNode *bluePtrArr[1000];
    frequencyNode *greenPtrArr[1000];
    frequencyNode *redPtrArr[1000];

    // set array nodeType to 2, to mark as unused
    for(int i = 0; i < 1000; ++i)
    {
        blueNodeArr[i].nodeType = 2;
        greenNodeArr[i].nodeType = 2;
        redNodeArr[i].nodeType = 2;

        blueNodeArr[i].frequency = 0;
        greenNodeArr[i].frequency = 0;
        redNodeArr[i].frequency = 0;

        bluePtrArr[i] = &blueNodeArr[i];
        greenPtrArr[i] = &greenNodeArr[i];
        redPtrArr[i] = &redNodeArr[i];
    }

    findFrequencies(byteArr, trueSize, blueFreqArr, 0, trueWidth, qualityVal);
    findFrequencies(byteArr, trueSize, greenFreqArr, 1, trueWidth, qualityVal);
    findFrequencies(byteArr, trueSize, redFreqArr, 2, trueWidth, qualityVal);

    sortPointers(bluePtrArr, blueFreqArr);
    sortPointers(greenPtrArr, greenFreqArr);
    sortPointers(redPtrArr, redFreqArr);

    int blueSize = getSize(bluePtrArr);
    int greenSize = getSize(greenPtrArr);
    int redSize = getSize(redPtrArr);

    codeNode blueCodeArr[blueSize];
    codeNode greenCodeArr[greenSize];
    codeNode redCodeArr[redSize];

    getKeys(blueCodeArr, bluePtrArr, blueSize);
    getKeys(greenCodeArr, greenPtrArr, greenSize);
    getKeys(redCodeArr, redPtrArr, redSize);

    buildHuffman(bluePtrArr, blueSize);
    buildHuffman(greenPtrArr, greenSize);
    buildHuffman(redPtrArr, redSize);

    getCodes(blueCodeArr, bluePtrArr[0], blueFreqArr, blueSize);
    getCodes(greenCodeArr, greenPtrArr[0], greenFreqArr, greenSize);
    getCodes(redCodeArr, redPtrArr[0], redFreqArr, redSize);

    BYTE compressedData[trueSize];
    LONG bitPtr = 0;
    for(int i = 0; i < trueSize; ++i)
        compressedData[i] = 0;

    writeCompressedData(byteArr, compressedData, trueSize, trueWidth, byteWidth, &bitPtr, blueCodeArr, greenCodeArr, redCodeArr, qualityVal);


    int sizes[3] = {blueSize, greenSize, redSize};

    fileWrite(fileHeader, infoHeader, &bitPtr, sizes, blueFreqArr, greenFreqArr, redFreqArr, blueCodeArr, greenCodeArr, redCodeArr, compressedData, outputFP);

    fclose(inputFP);
    fclose(outputFP);
    
    return 0;
}