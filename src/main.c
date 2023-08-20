#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iocslib.h>
#include <doslib.h>
#include "himem.h"
#include "largecp.h"

//
//  file read buffer
//
uint8_t* fread_buffer = NULL;

//
//  abort vector handler
//
static void abort_application() {
  
  // reclaim buffers
  if (fread_buffer != NULL) {
    himem_free(fread_buffer, 0);
    fread_buffer = NULL;
  }

  // cursor on
  C_CURON();

  // flush key buffer
  KFLUSHIO(0xff);

  // abort exit
  EXIT2(1);
}

//
//  show help message
//
static void show_help_message() {
  printf("usage: largecp [options] <src-file> <dst-path>\n");
  printf("options:\n");
  printf("    -b<n> ... buffer size in MB (1-8, default:4)\n");
  printf("    -h    ... show help message\n");
}

//
//  main
//
int32_t main(int32_t argc, char* argv[]) {

  // return code
  int32_t rc = -1;

  // buffer size in MB
  int16_t buffer_size = 4;

  // source file and destination path
  uint8_t src_file[ MAX_PATH_LEN ];
  uint8_t dst_path[ MAX_PATH_LEN ];
  src_file[0] = '\0';
  dst_path[0] = '\0';

  // file handles
  FILE* fp = NULL;
  FILE* fo = NULL;

  // set abort vectors
  uint32_t abort_vector1 = INTVCS(0xFFF1, (int8_t*)abort_application);
  uint32_t abort_vector2 = INTVCS(0xFFF2, (int8_t*)abort_application);  

  // show credit
  printf("LARGECP.X - Large file copy utility version " PROGRAM_VERSION " by tantan\n");

  // parse command lines
  for (int16_t i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && strlen(argv[i]) >= 2) {
      if (argv[i][1] == 'b') {
        if (strlen(argv[i]) < 3) {
          show_help_message();
          goto exit;
        }
        buffer_size = atoi(argv[i]+2);
        if (buffer_size < 1 || buffer_size > 8) {
          show_help_message();
          goto exit;
        }        
      } else if (argv[i][1] == 'h') {
        show_help_message();
        goto exit; 
      } else {
        show_help_message();
        goto exit;
      }
    } else {
      if (src_file[0] == '\0') {
        strcpy(src_file, argv[i]);
      } else if (dst_path[0] == '\0') {
        strcpy(dst_path, argv[i]);
      } else {
        show_help_message();
        goto exit;
      }
    }
  }

  if (src_file[0] == '\0' || dst_path[0] == '\0') {
    show_help_message();
    goto exit;
  }

  // allocate buffer memory
  fread_buffer = himem_malloc(buffer_size * 1024 * 1024, 0);
  if (fread_buffer == NULL) {
    printf("error: buffer memory allocation error.\n");
    goto exit;
  }

  // source file name check
  struct NAMECKBUF src_nmck;
  if (NAMECK(src_file, &src_nmck) < 0) {
    printf("error: incorrect source file name.\n");
    goto exit;
  }

  // destination path is a directory?
  struct FILBUF dst_filbuf;
  if (strcmp(dst_path, ".") == 0 || strcmp(dst_path, "..") == 0) {
    strcat(dst_path, "\\");
  }
  uint8_t dst_c = dst_path[ strlen(dst_path) - 1 ];
  if (dst_c == ':' || dst_c == '\\' || dst_c == '/') {
    strcat(dst_path, src_nmck.name);
    strcat(dst_path, src_nmck.ext);
  } else {
    if (FILES(&dst_filbuf, dst_path, 0x10) >= 0) {
      strcat(dst_path, "\\");
      strcat(dst_path, src_nmck.name);
      strcat(dst_path, src_nmck.ext);
    }
  }
  if (FILES(&dst_filbuf, dst_path, 0x20) >= 0) {
    printf("warning: dst file already exists. overwrite? (y/n)");
    int16_t c = INKEY();
    printf("\n");
    if (c != 'y' && c != 'Y') goto exit;
  }

  // read open source file
  fp = fopen(src_file, "rb");
  if (fp == NULL) {
    printf("error: source file open error. (%s)\n", src_file);
    goto exit;
  }

  // check source file size
  fseek(fp, 0, SEEK_END);
  size_t file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // write open destination file
  fo = fopen(dst_path, "wb");
  if (fo == NULL) {
    printf("error: destination file open error. (%s)\n", dst_path);
    goto exit;
  }

  printf("\n");
  printf("Source file: %s (%d bytes)\n", src_file, file_size);
  printf("Destination: %s\n", dst_path);
  printf("\n");

  uint32_t t0 = ONTIME();

  size_t read_len = 0;
  size_t buffer_bytes = buffer_size * 1024 * 1024;
  do {
    size_t read_size = (file_size - read_len) < buffer_bytes ? (file_size - read_len) : buffer_bytes;
    size_t len = fread(fread_buffer, 1, read_size, fp);
    if (len == 0) break;
    size_t w_len = fwrite(fread_buffer, 1, len, fo);
    if (w_len != len) {
      printf("\r\nerror: file write error.\n");
      goto exit;
    }
    read_len += len;
    printf("\rCopied %d/%d bytes ... [SHIFT] to cancel", read_len, file_size);
    if (B_SFTSNS() & 0x01) {
      printf("\r\nAborted.\n");
      goto exit;
    }
  } while (read_len < file_size);

  uint32_t t1 = ONTIME();

  printf("\rCopied %d bytes in %4.2f sec.\x1b[K\n", read_len, (t1 - t0) / 100.0);

  fclose(fo);
  fo = NULL;

  fclose(fp);
  fp = NULL;

  rc = 0;

exit:
  if (fo != NULL) {
    fclose(fo);
    fo = NULL;
  }
  if (fp != NULL) {
    fclose(fp);
    fp = NULL;
  }
  if (fread_buffer != NULL) {
    himem_free(fread_buffer, 0);
  }

  return rc;
}