
/* vim: set et ts=3 sw=3 sts=3 ft=c:
 *
 * Copyright (C) 2012, 2013, 2014 James McLaughlin et al.  All rights reserved.
 * https://github.com/udp/json-parser
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _JSON_H
#define _JSON_H

#ifndef json_char
   #define json_char char
#endif

#ifndef json_uchar
#define json_uchar unsigned char
#endif


#ifdef __cplusplus
#define JSON_INLINE inline
#else
#define JSON_INLINE inline
#endif

#ifndef json_int_t
   #ifndef _MSC_VER
      #include <inttypes.h>
      #define json_int_t int64_t
   #else
      #define json_int_t __int64
   #endif
#endif


#if defined(__GNUC__) || defined(__clang__)
#define JANSSON_ATTRS(...) __attribute__((__VA_ARGS__))
#else
#define JANSSON_ATTRS(...)
#endif



#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus

   #include <string.h>

   extern "C"
   {

#endif

typedef struct
{
   unsigned long max_memory;
   int settings;

   /* Custom allocator support (leave null to use malloc/free)
    */

   void * (* mem_alloc) (size_t, int zero, void * user_data);
   void (* mem_free) (void *, void * user_data);

   void * user_data;  /* will be passed to mem_alloc and mem_free */

   size_t value_extra;  /* how much extra space to allocate for values? */

} json_settings;

#define json_enable_comments  0x01

typedef enum {
	BOS_NULL = 0x00,
	BOS_BOOL = 0x01,
	BOS_INT8 = 0x02,
	BOS_INT16 = 0x03,
	BOS_INT32 = 0x04,
	BOS_INT64 = 0x05,
	BOS_UINT8 = 0x06,
	BOS_UINT16 = 0x07,
	BOS_UINT32 = 0x08,
	BOS_UINT64 = 0x09,
	BOS_FLOAT = 0x0A,
	BOS_DOUBLE = 0x0B,
	BOS_STRING = 0x0C,
	BOS_BYTES = 0x0D,
	BOS_ARRAY = 0x0E,
	BOS_OBJ = 0x0F
} bos_data_type;


typedef enum
{
   json_none,
   json_object,
   json_array,
   json_integer,
   json_double,
   json_string,
   json_boolean,
   json_null,
   json_bytes

} json_type;

/* bos structure */

typedef struct bos_t {
	const void *data;
	uint32_t size;
} bos_t;


/* error reporting */

#define JSON_ERROR_TEXT_LENGTH    160
#define JSON_ERROR_SOURCE_LENGTH   80

typedef struct json_error_t {
	int line;
	int column;
	int position;
	char source[JSON_ERROR_SOURCE_LENGTH];
	char text[JSON_ERROR_TEXT_LENGTH];
} json_error_t;

enum json_error_code {
	json_error_unknown,
	json_error_out_of_memory,
	json_error_stack_overflow,
	json_error_cannot_open_file,
	json_error_invalid_argument,
	json_error_invalid_utf8,
	json_error_premature_end_of_input,
	json_error_end_of_input_expected,
	json_error_invalid_syntax,
	json_error_invalid_format,
	json_error_wrong_type,
	json_error_null_character,
	json_error_null_value,
	json_error_null_byte_in_key,
	json_error_duplicate_key,
	json_error_numeric_overflow,
	json_error_item_not_found,
	json_error_index_out_of_range
};

static inline enum json_error_code json_error_code(const json_error_t *e) {
	return (enum json_error_code)e->text[JSON_ERROR_TEXT_LENGTH - 1];
}




extern const struct _json_value json_value_none;
       
typedef struct _json_object_entry
{
    json_char * name;
    unsigned int name_length;
    
    struct _json_value * value;
    
} json_object_entry;

typedef struct _json_value
{
   struct _json_value * parent;

   json_type type;

   union
   {
      int boolean;
      json_int_t integer;
      double dbl;

      struct
      {
         unsigned int length;
         json_char * ptr; /* null terminated */

      } string;

	  struct
	  {
		  unsigned int length;
		  json_uchar * ptr; /* null terminated */

	  } bytes;

      struct
      {
         unsigned int length;

         json_object_entry * values;

         #if defined(__cplusplus) && __cplusplus >= 201103L
         decltype(values) begin () const
         {  return values;
         }
         decltype(values) end () const
         {  return values + length;
         }
         #endif

      } object;

      struct
      {
         unsigned int length;
         struct _json_value ** values;

         #if defined(__cplusplus) && __cplusplus >= 201103L
         decltype(values) begin () const
         {  return values;
         }
         decltype(values) end () const
         {  return values + length;
         }
         #endif

      } array;

   } u;

   union
   {
      struct _json_value * next_alloc;
      void * object_mem;

   } _reserved;

   #ifdef JSON_TRACK_SOURCE

      /* Location of the value in the source JSON
       */
      unsigned int line, col;

   #endif


   /* Some C++ operator sugar */

   #ifdef __cplusplus

      public:

         inline _json_value ()
         {  memset (this, 0, sizeof (_json_value));
         }

         inline const struct _json_value &operator [] (int index) const
         {
            if (type != json_array || index < 0
                     || ((unsigned int) index) >= u.array.length)
            {
               return json_value_none;
            }

            return *u.array.values [index];
         }

         inline const struct _json_value &operator [] (const char * index) const
         { 
            if (type != json_object)
               return json_value_none;

            for (unsigned int i = 0; i < u.object.length; ++ i)
               if (!strcmp (u.object.values [i].name, index))
                  return *u.object.values [i].value;

            return json_value_none;
         }

         inline operator const char * () const
         {  
            switch (type)
            {
               case json_string:
                  return u.string.ptr;

               default:
                  return "";
            };
         }

		 inline operator const unsigned char * () const
		 {
			 switch (type)
			 {
			 case json_bytes:
				 return u.bytes.ptr;

			 default:
				 return 0;
			 };
		 }


         inline operator json_int_t () const
         {  
            switch (type)
            {
               case json_integer:
                  return u.integer;

               case json_double:
                  return (json_int_t) u.dbl;

               default:
                  return 0;
            };
         }

         inline operator bool () const
         {  
            if (type != json_boolean)
               return false;

            return u.boolean != 0;
         }

         inline operator double () const
         {  
            switch (type)
            {
               case json_integer:
                  return (double) u.integer;

               case json_double:
                  return u.dbl;

               default:
                  return 0;
            };
         }

   #endif

} json_value;
       
json_value * json_parse (const json_char * json,
                         size_t length);

#define json_error_max 128
json_value * json_parse_ex (json_settings * settings,
                            const json_char * json,
                            size_t length,
                            char * error);

void json_value_free (json_value *);


/* Not usually necessary, unless you used a custom mem_alloc and now want to
 * use a custom mem_free.
 */
void json_value_free_ex (json_settings * settings,
                         json_value *);

/*************** to fix ******************************/
json_value* json_get_val(json_value *obj, const char *key);

// todo
char* json_dumps(json_value * value, int opt);

typedef json_value json_t;
#define json_typeof(json)      ((json)->type)
#define json_is_array(json)    (json && json_typeof(json) == json_array)
#define json_is_integer(json)  (json && json_typeof(json) == json_integer)
#define json_is_double(json)   (json && json_typeof(json) == json_double)
#define json_is_string(json)   (json && json_typeof(json) == json_string)
#define json_is_null(json)     (json && json_typeof(json) == json_null)
#define json_is_bytes(json)    ((json) && json_typeof(json) == json_bytes)
#define json_is_object(json)    ((json) && json_typeof(json) == json_object)
#define json_is_boolean(json)    ((json) && json_typeof(json) == json_boolean)
#define json_is_number(json)    (json && (json_typeof(json) == json_integer || json_typeof(json) == json_double))

int json_integer_value(const json_value *json);
char* json_string_value(const json_value *json);
unsigned char* json_bytes_value(const json_value *json);
double json_double_value(const json_value *json);

/* bos */

int bos_validate(const void *data, size_t size);
unsigned int bos_sizeof(const void *data);
json_t *bos_deserialize(const void *data, json_error_t *error) JANSSON_ATTRS(warn_unused_result);

bos_t *bos_serialize(json_t *value, json_error_t *error) JANSSON_ATTRS(warn_unused_result);
void bos_free(bos_t *ptr);

typedef void *(*json_malloc_t)(size_t);
typedef void(*json_free_t)(void *);

static json_malloc_t do_malloc = malloc;
static json_free_t do_free = free;

void *jsonp_malloc(size_t size);


void jsonp_free(void *ptr);

/* declaration of methods from jsonp_error */
void jsonp_error_init(json_error_t *error, const char *source);
void jsonp_error_set_source(json_error_t *error, const char *source);
void jsonp_error_set(json_error_t *error, int line, int column,
	size_t position, enum json_error_code code,
	const char *msg, ...);
void jsonp_error_vset(json_error_t *error, int line, int column,
	size_t position, enum json_error_code code,
	const char *msg, va_list ap);


#ifdef __cplusplus
   } /* extern "C" */
#endif

#endif


