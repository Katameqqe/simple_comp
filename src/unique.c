#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define CHUNK 256
#define CHUNK_SIZE 1024*1024

#define min(x, y) (x < y ? x : y)


int compare (const void * a, const void * b)
{
  return ( *(unsigned char*)a - *(unsigned char*)b );
}
/*
Function to make set of unique bytes from text
*/
unsigned short int uniques(unsigned char* r_list,FILE* fptr){
    char f_dup = 0x00;
    //unsigned char* list = (unsigned char*)calloc(256,1);
    unsigned char* rbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    unsigned int max_read;
    unsigned int i = 0;
    unsigned short int b_i = 0;

    //int res = fread(&buff,1,1,fptr);
    max_read = fread(rbyte_buffer,1,CHUNK_SIZE,fptr);

    r_list[0] = rbyte_buffer[0];
    i++;
    b_i++;
    while (max_read > 0){

        f_dup = 0x00;
        for (int j=0; j<b_i; j++){
            if (r_list[j] == rbyte_buffer[i]){
                f_dup = 0x01;
                break;
            }
        }
        /*
        qsort(list, b_i, 1, compare);
        byte = rbyte_buffer[i];
        f = (unsigned char*)bsearch(&byte,list,b_i,1,compare);
        */
        if (f_dup == 0x00){
            r_list[b_i] = rbyte_buffer[i];
            if (b_i == 255){
                printf("\n%d\n",b_i);
                return b_i;
            }
            b_i+=1;
            printf("%d ",b_i);
        }
        i+=1;
        if(i > max_read){
            //max_read = fread(rbyte_buffer,1,CHUNK_SIZE,fptr);
            max_read = 0;
            i=0;
        }
        //printf("%s",list);
    }
    free(rbyte_buffer);
    printf("\n%d \nEND\n",b_i);
    return b_i;
}

unsigned char uniq(unsigned char* r_list, FILE* fptr){
    return 0;
}

void compress(FILE* input_file, FILE* output_file){
    unsigned char* buf = (unsigned char*)calloc(CHUNK,1);
    unsigned char *found;
    unsigned char index;
    size_t max_read;
    unsigned char* rbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    unsigned char* wbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    unsigned int rbyte_ofset = 0;
    unsigned int wbyte_ofset = 0;


    size_t f_size;
    
    const unsigned char X = uniques(buf,input_file);
    //printf("%d\n", X);
    wbyte_buffer[wbyte_ofset] = X;
    wbyte_ofset+=1;
    qsort(buf, X, 1, compare);
    printf("%x\n",wbyte_buffer);
    memcpy(wbyte_buffer+wbyte_ofset,buf,X);
    printf("%x\n",wbyte_buffer);
    wbyte_ofset+=X;
    printf("%s\n",buf);
    //fwrite(&X, 1, 1, output_file);
    //fwrite(buf, X, 1, output_file);

    const int N = ceil(log2(X));

    fseek(input_file, 0L, SEEK_END);
    f_size = ftell(input_file);
    fseek(input_file, 0L, SEEK_SET);

    unsigned char ofset = 3;

    const int R = (8 - (3 + f_size * N) % 8) % 8;
    unsigned int need_wb = (((f_size * N)+3+R)/8)+X;
    unsigned char bytes = R<<(8-ofset) & 0xff;

    printf("%d %d %d %d %d\n", X, N, R, f_size, need_wb);

    max_read = fread(rbyte_buffer, 1, CHUNK_SIZE,input_file);
    //printf("%s\nEND\n",rbyte_buffer);
    while (max_read > 0){
        found = (unsigned char *)bsearch(&rbyte_buffer[rbyte_ofset], buf, X, 1, compare);
        index = (found-buf) & 0xff;
        index <<= 8-N;
        bytes |= (index>>ofset);
        if (ofset >= 8-N){
            wbyte_buffer[wbyte_ofset]=bytes;
            wbyte_ofset+=1;
            bytes = index << (8-ofset);
            if (wbyte_ofset >= min(need_wb,CHUNK_SIZE)){
                fwrite(wbyte_buffer,1,wbyte_ofset,output_file);
                need_wb -= wbyte_ofset;
                //printf("%s\nEND OF STRING\n\n",wbyte_buffer);
                wbyte_ofset = 0;
            }
        }
        rbyte_ofset++;
        ofset=(ofset+N)%8;
        if (rbyte_ofset >= min(CHUNK_SIZE,max_read)){
            max_read = fread(rbyte_buffer, 1, CHUNK_SIZE,input_file);
            //printf("%s\nEND\n",rbyte_buffer);
            rbyte_ofset=0;
        }
    }
    if (R>0){
        printf("\n%d %d\n",wbyte_ofset, need_wb+wbyte_ofset);
        fwrite(&bytes,1,1,output_file);
    }
    printf("1\n");
    free(rbyte_buffer);
    printf("2\n");
    free(wbyte_buffer);
    printf("3\n");
}

void decompress(FILE* input_file, FILE* output_file){
    unsigned char X;
    unsigned char byte;
    size_t max_readed;
    size_t d_size;
    unsigned int wi = 0;
    unsigned char* rbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    unsigned char* point_buffer = rbyte_buffer;
    unsigned char* wbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    
    fread(&X, 1, 1, input_file);
    printf("%d \n",X);
    
    unsigned char* buf = malloc(X);
    fread(buf, 1, X, input_file);
    for (unsigned char i = 0; i < X; i++){
        //printf("%X ",buf[i]);
    }
    printf("\n");
    printf("%d ",X);
    const int N = ceil(log2(X));
    printf("%d ",N);

    unsigned char ofset = 3;
    char need_bits = 0;

    fread(&byte, 1, 1, input_file);
    const unsigned char R = byte>>(8-ofset);
    fseek(input_file, -1L, SEEK_CUR);
    printf("%d ",R);
    
    fseek(input_file, 0L, SEEK_END);
    d_size = ftell(input_file)-(X+1);
    fseek(input_file, (1+X), SEEK_SET);
    printf("%d ",d_size);
    const int b_size = (((d_size*8)-(3+R))/N);
    printf("%d\n",b_size);
    unsigned int need_size = b_size;
    
    // Bottom algorythm not working FIX IT
    max_readed = fread(rbyte_buffer,1,CHUNK_SIZE,input_file);
    //while(fread(&rbyte,1,1,input_file) > 0){
    for(int x = 0; x < b_size; x++){
        if (point_buffer-rbyte_buffer >= CHUNK_SIZE-1){
                rbyte_buffer[0] = rbyte_buffer[max_readed-1];
                max_readed = fread(rbyte_buffer+1,1,CHUNK_SIZE,input_file);
                if (max_readed >= CHUNK_SIZE){
                    fseek(input_file, -1, SEEK_CUR);
                }
                point_buffer=rbyte_buffer;
            }
            byte = *point_buffer & (((1<<N)-1)<<(8-N))>>ofset;
            need_bits = (8-ofset)-N;
            if (need_bits <= 0){
                byte = ((byte<<-need_bits) | (*(point_buffer+1) >> (8+need_bits)));
                point_buffer++;
            }else{
                byte >>= (8-(N+ofset));
            }
            ofset = (ofset+N)%8;
                //printf("%d ",byte);
            wbyte_buffer[wi] = buf[byte];
            wi++;
            if(wi>=min(CHUNK_SIZE,need_size)){
                wbyte_buffer[wi] = buf[byte];
                printf("%d %x %x %x\n", wi,wbyte_buffer[wi-2],wbyte_buffer[wi-1],wbyte_buffer[wi]);
                fwrite(wbyte_buffer,1, min(CHUNK_SIZE,wi),output_file);
                need_size-=wi;
                wi=0;
                //printf("%d \n",min(CHUNK_SIZE,b_size));
            }
            //fwrite(wbyte_buffer,1,1,output_file);
    }
    printf("\n");
    free(wbyte_buffer);
    free(rbyte_buffer);
    free(buf);
    //fwrite(wbyte_buffer,1,(1+X+b_size),output_file);
}

int main(void)
{
    FILE* fptr;
    FILE* wptr;
    FILE* zptr;

    fptr = fopen("sample.txt","rb");
    wptr = fopen("testW.txt","wb");

    //unsigned char* c[1024]; 

    //fread(c,1,1024,fptr);

    //fseek(fptr,-1,SEEK_CUR);

    compress(fptr, wptr);

    fclose(fptr);
    fclose(wptr);

    wptr = fopen("testW.txt","rb");
    zptr = fopen("sample1.txt","wb");

    decompress(wptr, zptr);

    fclose(wptr);
    fclose(zptr);

    return 0;
}