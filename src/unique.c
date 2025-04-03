#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BUF_CHUNK 128
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
    unsigned char* rbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    unsigned int max_read;
    unsigned int i = 0;
    unsigned char b_i = 0;

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
            if (b_i >= BUF_CHUNK-1){
                return b_i+1;
            }
            b_i+=1;
        }
        i+=1;
        if(i > max_read){
            max_read = 0;
            i=0;
        }
    }
    free(rbyte_buffer);
    return b_i;
}

unsigned char uniq(unsigned char* r_list, FILE* fptr){
    return 0;
}

void compress(FILE* input_file, FILE* output_file){
    unsigned char* uniq_buff = (unsigned char*)calloc(BUF_CHUNK,1);
    unsigned char *f_index;
    unsigned char buff_i;
    size_t max_read;
    unsigned char* rbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    unsigned char* wbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    unsigned int rbyte_offset = 0;
    unsigned int wbyte_offset = 0;


    size_t f_size;
    
    const unsigned char X = uniques(uniq_buff,input_file);
    if (X>BUF_CHUNK) return -1;
    
    wbyte_buffer[wbyte_offset] = X;
    wbyte_offset+=1;
    qsort(uniq_buff, X, 1, compare);
    memcpy(wbyte_buffer+wbyte_offset,uniq_buff,X);
    wbyte_offset+=X;

    const int N = ceil(log2(X));

    fseek(input_file, 0L, SEEK_END);
    f_size = ftell(input_file);
    fseek(input_file, 0L, SEEK_SET);

    unsigned char offset = 3;

    const int R = (8 - (3 + f_size * N) % 8) % 8;
    unsigned int need_wb = (((f_size * N)+3+R)/8)+X;
    unsigned char byte = R<<(8-offset) & 0xff;

    printf("X:%d N:%d R:%d\n", X, N, R);

    max_read = fread(rbyte_buffer, 1, CHUNK_SIZE,input_file);
    while (max_read > 0){
        f_index = (unsigned char *)bsearch(&rbyte_buffer[rbyte_offset], uniq_buff, X, 1, compare);
        buff_i = (f_index-uniq_buff) & 0xff;
        buff_i <<= 8-N;
        byte |= (buff_i>>offset);
        if (offset >= 8-N){
            wbyte_buffer[wbyte_offset]=byte;
            wbyte_offset+=1;
            byte = buff_i << (8-offset);
            if (wbyte_offset >= min(need_wb,CHUNK_SIZE)){
                fwrite(wbyte_buffer,1,wbyte_offset,output_file);
                need_wb -= wbyte_offset;
                wbyte_offset = 0;
            }
        }
        rbyte_offset++;
        offset=(offset+N)%8;
        if (rbyte_offset >= min(CHUNK_SIZE,max_read)){
            max_read = fread(rbyte_buffer, 1, CHUNK_SIZE,input_file);
            rbyte_offset=0;
        }
    }
    if (R>0){
        fwrite(&byte,1,1,output_file);
    }
    free(uniq_buff);
    free(rbyte_buffer);
    free(wbyte_buffer);
}

void decompress(FILE* input_file, FILE* output_file){
    unsigned char X;
    unsigned char byte;
    size_t max_readed;
    size_t f_size;
    unsigned int w_offset = 0;
    unsigned char* rbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    unsigned char* point_buffer = rbyte_buffer;
    unsigned char* wbyte_buffer = (unsigned char* )malloc(CHUNK_SIZE);
    
    fread(&X, 1, 1, input_file);
    
    if (X>BUF_CHUNK) return -1;

    unsigned char* uniq_buff = malloc(X);
    fread(uniq_buff, 1, X, input_file);

    const int N = ceil(log2(X));

    unsigned char offset = 3;
    char need_bits = 0;

    fread(&byte, 1, 1, input_file);

    const unsigned char R = byte>>(8-offset);
    fseek(input_file, -1L, SEEK_CUR);
    
    fseek(input_file, 0L, SEEK_END);
    f_size = ftell(input_file)-(X+1);
    fseek(input_file, (1+X), SEEK_SET);

    const int b_size = (((f_size*8)-(3+R))/N);
    unsigned int need_size = b_size;
    
    max_readed = fread(rbyte_buffer,1,CHUNK_SIZE,input_file);
    for(int x = 0; x < b_size; x++){
        if (point_buffer-rbyte_buffer >= CHUNK_SIZE-1){
                rbyte_buffer[0] = rbyte_buffer[max_readed-1];
                max_readed = fread(rbyte_buffer+1,1,CHUNK_SIZE,input_file);
                if (max_readed >= CHUNK_SIZE){
                    fseek(input_file, -1, SEEK_CUR);
                }
                point_buffer=rbyte_buffer;
            }
            byte = *point_buffer & (((1<<N)-1)<<(8-N))>>offset;
            need_bits = (8-offset)-N;
            if (need_bits <= 0){
                byte = ((byte<<-need_bits) | (*(point_buffer+1) >> (8+need_bits)));
                point_buffer++;
            }else{
                byte >>= (8-(N+offset));
            }
            offset = (offset+N)%8;
            wbyte_buffer[w_offset] = uniq_buff[byte];
            w_offset++;
            if(w_offset>=min(CHUNK_SIZE,need_size)){
                wbyte_buffer[w_offset] = uniq_buff[byte];
                fwrite(wbyte_buffer,1, min(CHUNK_SIZE,w_offset),output_file);
                need_size-=w_offset;
                w_offset=0;
            }
    }
    free(wbyte_buffer);
    free(rbyte_buffer);
    free(uniq_buff);
}

int main(int argc, char* argv[]){
    //for (int i = argc; i>0;i--){
    //    printf("%s\n",argv[i]);
    //}

    FILE* fptr;
    FILE* wptr;
    FILE* zptr;

    fptr = fopen("test.txt","rb");
    wptr = fopen("testW.txt","wb");

    compress(fptr, wptr);

    fclose(fptr);
    fclose(wptr);

    wptr = fopen("testW.txt","rb");
    zptr = fopen("testZ.txt","wb");

    decompress(wptr, zptr);

    fclose(wptr);
    fclose(zptr);

    return 0;
}