/* Authors : Alia ElKattan , Chunxiao Wang
Version:2.1
*/

#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 512       /* floppy sector size in bytes */
#define OS_SIZE_LOC 2         /* OS position within bootblock */  
#define BOOT_MEM_LOC 0x7c00   /* bootblock memory location */
#define OS_MEM_LOC 0x1000     /* kernel memory location */
#define BOOT_LOADER_SIG_OFFSET 0x1fe /* offset for boot loader signature */
#define BOOT_LOADER_SIG_1 0x55  /* signature at BOOT_LOADER_SIG_OFFSET */
#define BOOT_LOADER_SIG_2 0xaa  /* signature at BOOT_LOADER_SIG_OFFSET + 1 */

// ignore the --vm argument when implementing

/*
read_exec_file(): reads in an executable file in ELF format
write_bootblock(): writes the bootblock to the image file
write_kernel(): writes the kernel to the image file
count_kernel_sectors(): counts the number of sectors in the kernel
record_kernel_sectors(): tells the bootloader how many sectors the kernel has
extended_opt(): prints the information for --the extended option
*/

/*
 Function reference:
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
int fseek(FILE *stream, long int offset, int whence)read_exec_file()
int fputc(int char, FILE *pointer)
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
int fprintf(FILE *stream, const char *format, ...)
void *malloc(size_t size)
void *calloc(size_t nitems, size_t size)
*/
 
//former declaration for function not yet defined
void write_image(FILE **image_file, FILE *read_file, Elf32_Ehdr *elf_hdr, Elf32_Phdr *phdr); 

//read excutables in ELF format
Elf32_Phdr* read_exec_file(FILE **excutables, Elf32_Ehdr *ehdr, char* filename){ //compound reference??
    //open file
    if (!(*excutables = fopen(filename, "r"))) {
        fprintf(stderr, "Error opening excutable file");
        exit(EXIT_FAILURE);
    }
    //read ELF header from the beginning of the file
    fread(ehdr,1,sizeof(Elf32_Ehdr),*excutables);
    
    fseek(*excutables,(*ehdr).e_phoff, SEEK_SET);// set the position indicator to the offset where ph table is
    size_t num_phdr = (*ehdr).e_phnum; //number of entries in the program header table
    size_t size_phdr = (*ehdr).e_phentsize; //size of entries in the program header table
    
    //initialize pointer for program header
    Elf32_Phdr *phdr = calloc(num_phdr,size_phdr);
    
    //read program header
    fread(phdr,size_phdr,num_phdr,*excutables);
    
    return phdr;
    
}

void write_bootblock(FILE **image_file, FILE *boot_file, Elf32_Ehdr *boot_hdr, Elf32_Phdr *boot_phdr){
    
    //fseek: stream (pointer to file), offset, origin (seek_set = beginning of file)
    fseek(*image_file, 0, SEEK_SET);
    
    //write_image(); //1 function to write to image file... what goes here?
    write_image(image_file, boot_file, boot_hdr, boot_phdr);
    
    fseek(*image_file, BOOT_LOADER_SIG_OFFSET, SEEK_SET);

    //adding the 55aa signature for end of bootblock
    fputc(0x55, *image_file);
    fputc(0xAA, *image_file); 
}

//write kernel to image file
void write_kernel(FILE **image_file, FILE *kernel_file, Elf32_Ehdr *kernel_hdr, Elf32_Phdr *kernel_phdr) {
    //fseek image file from offset sector_size (defined above) at beginning of file
    fseek(*image_file, SECTOR_SIZE, SEEK_SET);
    
    //added a write_image function because its reused for write_image
    write_image(image_file, kernel_file, kernel_hdr, kernel_phdr);
}


//add error detection??

//write kernel or bootblock to image file
void write_image(FILE **image_file, FILE *read_file, Elf32_Ehdr *elf_hdr, Elf32_Phdr *phdr) {
    
    size_t padding;

    //index of program header
    int phdr_index;
    char buffer[SECTOR_SIZE];
    size_t bytes_read=0;
    
    //track progress in current loop
    size_t bytes_remaining, bytes_to_read, current_read;
    
    
    //in each phdr
    
    //loop through program headers
    for (phdr_index = 0; phdr_index < (*elf_hdr).e_phnum; phdr_index++) {
        
        bytes_remaining = phdr[phdr_index].p_filesz;
        padding = phdr[phdr_index].p_memsz - bytes_remaining;
        
        fseek(read_file, phdr[phdr_index].p_offset, SEEK_SET);
        
        while (bytes_remaining > 0) {

            //if bytes remaining to read are less than sector size, we need to read the number of bytes remaining
            
            if (bytes_remaining < SECTOR_SIZE)
                bytes_to_read = bytes_remaining;

            else bytes_to_read = SECTOR_SIZE;
            
            current_read = fread(buffer, 1, bytes_to_read, read_file);
            current_read = fwrite(buffer, 1, bytes_to_read, *image_file);
            
            bytes_read+= current_read;
            bytes_remaining -= current_read;
            
        }
        
        while(padding > 0) {
            
            fputc(0, *image_file);
            padding--;
            bytes_read++;
            
        }
    }
    
    padding = SECTOR_SIZE - (bytes_read % SECTOR_SIZE);
    
    while (padding > 0){
        
        fputc(0, *image_file);
        padding--;
    }
           
    }



//counts # of sectors in kernel
int count_kernel_sectors(Elf32_Ehdr *kernel_hdr, Elf32_Phdr *kernel_phdr){
	size_t kernel_size = 0;
	int sector_number;
	int x;

    //loop through kernel headers
	for (x = 0; x<(*kernel_hdr).e_phnum;x++) {
		kernel_size+=kernel_phdr[x].p_memsz;
	}

	sector_number = (kernel_size-1) / SECTOR_SIZE +1;

	return sector_number;
}



//record number of sectors in kernel
void record_kernel_sectors(FILE **image_file, int sector_number){

	fseek(*image_file, 2, SEEK_SET); //go back to this
	fwrite(&sector_number, sizeof(int), 1, *image_file);

}

//prints information for the extended option
void extended_opt(Elf32_Ehdr* beh,Elf32_Phdr* bph, Elf32_Ehdr* keh,Elf32_Phdr* kph,size_t num_phdr,int sector_num){
    
    size_t size_after_padding;
    int index =0;
    //printing bootblock segment info
    printf("0x%04x: ./bootblock\n",(*beh).e_entry);
    printf("\tsegment 0\n");
    printf("\t\toffset 0x%04x\t\tvaddr 0x%04x\n",(bph[index]).p_offset,(bph[index]).p_vaddr);
    printf("\t\tfilesz 0x%04x\t\tmemsz 0x%04x\n",(bph[index]).p_filesz,(bph[index]).p_memsz);
    printf("\t\twriting 0x%04x bytes\n",(bph[index]).p_filesz);
    size_after_padding = ((bph[index].p_memsz-1)/SECTOR_SIZE+1)*SECTOR_SIZE;
    printf("\t\tpadding up to 0x%04x\n",size_after_padding);

	//printing kernel segment info
    printf("0x%04x: ./kernel\n",(*keh).e_entry);
    for (index = 0; index<(int)num_phdr;index++){
		
        printf("\tsegment 0\n");
        printf("\t\toffset 0x%04x\t\tvaddr 0x%04x\n",(kph[index]).p_offset,(kph[index]).p_vaddr);
        printf("\t\tfilesz 0x%04x\t\tmemsz 0x%04x\n",(kph[index]).p_filesz,(kph[index]).p_memsz);
        printf("\t\twriting 0x%04x bytes\n",(kph[index]).p_filesz);
        size_after_padding += kph[index].p_memsz;
        if(index ==(int)num_phdr -1) 
	    {size_after_padding = ((size_after_padding-1)/SECTOR_SIZE+1)*SECTOR_SIZE;}
        printf("\t\tpadding up to 0x%04x\n",size_after_padding);
    }
	//printing size of os
    printf("os_size: %d sectors\n",sector_num);
} 


int main(int argc, char **argv)
{
	//pointers for kernel, boot, and image files
	FILE *image_file, *boot_file, *kernel_file;

	//check elf documentation for elf32_ehdr etc

	//boot header & program header
	Elf32_Ehdr *boot_hdr = malloc(sizeof(Elf32_Ehdr)); 
   	Elf32_Phdr *boot_phdr;

	//kernel header & program header
	Elf32_Ehdr *kernel_hdr = malloc(sizeof(Elf32_Ehdr));
	Elf32_Phdr *kernel_phdr;

	int sector_number;

	//open image file
	if (!(image_file = fopen(IMAGE_FILE, "w+"))) {
	fprintf(stderr, "Error opening image file");
	exit(EXIT_FAILURE);	
	}
    
	//read bootblock
    boot_phdr = read_exec_file(&boot_file,boot_hdr,"./bootblock");

	//write bootblock
	write_bootblock(&image_file, boot_file, boot_hdr, boot_phdr);

	//read kernel
    kernel_phdr = read_exec_file(&kernel_file,kernel_hdr,"./kernel");

	//write kernel
	write_kernel(&image_file, kernel_file, kernel_hdr, kernel_phdr);

	//# sector
	sector_number = count_kernel_sectors(kernel_hdr, kernel_phdr);
	record_kernel_sectors(&image_file, sector_number);


	//extended option
    if(!strncmp(argv[1],"--extended",10)){
        extended_opt(boot_hdr,boot_phdr,kernel_hdr,kernel_phdr,(*kernel_hdr).e_phnum,sector_number);
    }


	//free memory
	free(boot_hdr);
	free(boot_phdr);
	free(kernel_hdr);
	free(kernel_phdr);

	//close files
	fclose(image_file);
	fclose(boot_file);
	fclose(kernel_file);

	return 0;
}

