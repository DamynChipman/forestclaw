/*
Copyright (c) 2012-2022 Carsten Burstedde, Donna Calhoun, Scott Aiton
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <fclaw_packing.h>
#include <fclaw_base.h>


size_t fclaw_packsize_string(const char * string){
  return sizeof(size_t) + (string == NULL ? 0 : strlen(string)+1);
}

size_t fclaw_pack_string(const char* string, char* buffer){
  char * buffer_start = buffer;
  if(string == NULL){
    buffer += fclaw_pack_size_t(0, buffer);
  }else{
    size_t length = strlen(string)+1;
    buffer += fclaw_pack_size_t(length, buffer);
    memcpy(buffer,string,length);
    buffer += length;
  }
  return buffer - buffer_start;
}

size_t fclaw_unpack_string(const char * buffer, char** string){
  const char* buffer_start = buffer;
  size_t length;
  buffer += fclaw_unpack_size_t(buffer, &length);
  if(length == 0){
    *string = NULL;
  }else{
    *string = FCLAW_ALLOC(char,length);
    memcpy(*string,buffer,length);
    buffer += length;
  }
  return buffer - buffer_start;
}

size_t fclaw_pack_int(int value, char * buffer){
  return FCLAW_PACK(value,buffer);
}

size_t fclaw_unpack_int(const char* buffer, int* value){
  return FCLAW_UNPACK(buffer,value);
}

size_t fclaw_pack_size_t(size_t value, char* buffer){
  return FCLAW_PACK(value,buffer);
}

size_t fclaw_unpack_size_t(const char* buffer, size_t* value){
  return FCLAW_UNPACK(buffer,value);
}

size_t fclaw_pack_double(double value, char* buffer){
  return FCLAW_PACK(value,buffer);
}

size_t fclaw_unpack_double(const char * buffer, double* value){
  return FCLAW_UNPACK(buffer,value);
}