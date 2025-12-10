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

struct compressedHeader
{
    LONG bitLength;
    int blueSize;
    int greenSize;
    int redSize;
};

struct tagBITMAPINFOHEADER *infoHeader;
struct compressedHeader *cmpHeader;


void readHeaders(struct tagBITMAPFILEHEADER *fileHead, struct tagBITMAPINFOHEADER *fileInfo, FILE *fp)
{
    fread(&(fileHead -> bfType), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfSize), sizeof(DWORD), 1, fp);
    fread(&(fileHead -> bfReserved1), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfReserved2), sizeof(WORD), 1, fp);
    fread(&(fileHead -> bfOffBits), sizeof(DWORD), 1, fp);
    fread(fileInfo, sizeof(struct tagBITMAPINFOHEADER), 1, fp);
    fread(cmpHeader, sizeof(struct compressedHeader), 1, fp);
}

void readData(int *blue, int *green, int *red, codeNode *blueCode, codeNode *greenCode, codeNode *redCode, BYTE *data, LONG byteSize, FILE *fp)
{
    fread(blue, sizeof(int), 256, fp);
    fread(green, sizeof(int), 256, fp);
    fread(red, sizeof(int), 256, fp);
    fread(blueCode, sizeof(codeNode), cmpHeader->blueSize, fp);
    fread(greenCode, sizeof(codeNode), cmpHeader->greenSize, fp);
    fread(redCode, sizeof(codeNode), cmpHeader->redSize, fp);
    fread(data, byteSize, 1, fp);
}

void fileWrite(struct tagBITMAPFILEHEADER *fileHead, struct tagBITMAPINFOHEADER *fileInfo, BYTE array[], DWORD size, FILE *fp)
{
    fwrite(&(fileHead -> bfType), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfSize), sizeof(DWORD), 1, fp);
    fwrite(&(fileHead -> bfReserved1), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfReserved2), sizeof(WORD), 1, fp);
    fwrite(&(fileHead -> bfOffBits), sizeof(DWORD), 1, fp);
    fwrite(fileInfo, sizeof(struct tagBITMAPINFOHEADER), 1, fp);
    fwrite(array, size, 1, fp);
}


// // fill and sort node array
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

//     // insertion sort (descending order based on frequencies)
//     int j;
//     for(int i = 1; i < 256; ++i)
//     {
//         j = i;
//         while(j > 0 && nodeArr[j].frequency > nodeArr[j-1].frequency)
//         {
//             frequencyNode temp = nodeArr[j-1];
//             nodeArr[j-1] = nodeArr[j];
//             nodeArr[j] = temp;
//             --j;
//         }
//     }
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
        frequencyNode *internalNode = malloc(sizeof(frequencyNode));
        internalNode->left = left;
        internalNode->right = right;
        internalNode->frequency = left->frequency + right->frequency;
        internalNode->nodeType = 1;
        endOfList -= 2; // shrink list by 2

        // insert parent into array keeping descending order
        int i = endOfList;
        while (i > 0 && ptrArr[i-1]->frequency < internalNode->frequency) 
        {
            ptrArr[i] = ptrArr[i-1];
            --i;
        }
        ptrArr[i] = internalNode;
        ++endOfList;
    }
}



void uncompress(BYTE *uncompressedData, BYTE *compressedData, LONG trueWidth, LONG byteWidth, frequencyNode *blueHead, frequencyNode *greenHead, frequencyNode *redHead, codeNode *blueCode, codeNode *greenCode, codeNode *redCode)
{
    frequencyNode *curr = blueHead;
    LONG uBPtr = 0; // uncompressed bit pointer

    for(LONG i = 0; i < cmpHeader->bitLength; ++i)
    {
        // check if curr is a leaf
        if(curr->nodeType != 1)
        {
            if((uBPtr % trueWidth) % 3 == 0)
            {
                uncompressedData[uBPtr] = curr->key;
                ++uBPtr;
                curr = greenHead;
            }

            else if((uBPtr % trueWidth) % 3 == 1)
            {
                uncompressedData[uBPtr] = curr->key;
                ++uBPtr;
                curr = redHead;
            }
            
            else
            {
                uncompressedData[uBPtr] = curr->key;
                ++uBPtr;
                curr = blueHead;
            }

            while(uBPtr % trueWidth >= byteWidth)
            {
                uncompressedData[uBPtr] = 0;
                ++uBPtr;
            }
        }
        
        LONG idx = i / 8;
        int bitPosition = i % 8;
        BYTE bit = 1;
        bit <<= 8 - 1 - bitPosition;
        BYTE targetBit = compressedData[idx] & bit;

        if(targetBit == 0)
            curr = curr->left;
        
        else
            curr = curr->right;

    }
}

int main(int argc, char *argv[])
{
    struct tagBITMAPFILEHEADER *fileHeader = (struct tagBITMAPFILEHEADER*)malloc(sizeof(struct tagBITMAPFILEHEADER));
    infoHeader = (struct tagBITMAPINFOHEADER*)malloc(sizeof(struct tagBITMAPINFOHEADER));
    cmpHeader = (struct compressedHeader*)malloc(sizeof(struct compressedHeader));

    char *inputName = argv[1];
    char *outputName = argv[2];

    FILE *inputFP = fopen(inputName, "rb");
    FILE *outputFP = fopen(outputName, "wb");

    readHeaders(fileHeader, infoHeader, inputFP);

    LONG byteSize;
    if(cmpHeader->bitLength % 8 == 0)
        byteSize = cmpHeader->bitLength / 8;

    else
        byteSize = cmpHeader->bitLength / 8 + 1;

    int blueFreqArr[256];
    int greenFreqArr[256];
    int redFreqArr[256];
    
    codeNode blueCodeArr[cmpHeader->blueSize];
    codeNode greenCodeArr[cmpHeader->greenSize];
    codeNode redCodeArr[cmpHeader->redSize];

    BYTE compressedData[byteSize];

    readData(blueFreqArr, greenFreqArr, redFreqArr, blueCodeArr, greenCodeArr, redCodeArr, compressedData, byteSize, inputFP);

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

    sortPointers(bluePtrArr, blueFreqArr);
    sortPointers(greenPtrArr, greenFreqArr);
    sortPointers(redPtrArr, redFreqArr);

    buildHuffman(bluePtrArr, cmpHeader->blueSize);
    buildHuffman(greenPtrArr, cmpHeader->greenSize);
    buildHuffman(redPtrArr, cmpHeader->redSize);

    LONG byteWidth = (infoHeader -> biWidth) * 3;
    int padding = (4 - ((byteWidth) % 4)) % 4;
    LONG trueWidth = byteWidth + padding;
    DWORD trueSize = trueWidth * (infoHeader -> biHeight);
    BYTE uncompressedData[trueSize];

    uncompress(uncompressedData, compressedData, trueWidth, byteWidth, bluePtrArr[0], greenPtrArr[0], redPtrArr[0], blueCodeArr, greenCodeArr, redCodeArr);

    fileWrite(fileHeader, infoHeader, uncompressedData, trueSize, outputFP);

    fclose(inputFP);
    fclose(outputFP);
    
    return 0;
}