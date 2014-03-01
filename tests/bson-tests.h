/*
 * Copyright 2013 MongoDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef BSON_TESTS_H
#define BSON_TESTS_H


#include <bson.h>
#include <stdio.h>
#include <time.h>


BSON_BEGIN_DECLS


#define assert_cmpstr(a, b)                                             \
   do {                                                                 \
      if (((a) != (b)) && !!strcmp((a), (b))) {                         \
         fprintf(stderr, "FAIL\n\nAssert Failure: \"%s\" != \"%s\"\n",  \
                         a, b);                                         \
         abort();                                                       \
      }                                                                 \
   } while (0)


#define assert_cmpint(a, eq, b)                                         \
   do {                                                                 \
      if (!((a) eq (b))) {                                              \
         fprintf(stderr, "FAIL\n\nAssert Failure: "                     \
                         #a " " #eq " " #b "\n");                       \
         abort();                                                       \
      }                                                                 \
   } while (0)


#ifdef BSON_OS_WIN32
#include <share.h>
static BSON_INLINE int
bson_open (const char *filename,
           int         flags)
{
   int fd;
   errno_t err;

   err = _sopen_s (&fd, filename, flags | _O_BINARY, _SH_DENYNO,
                   _S_IREAD | _S_IWRITE);

   if (err) {
      errno = err;
      return -1;
   }

   return fd;
}

static BSON_INLINE ssize_t
bson_read (int    fd,
           void  *buf,
           size_t count)
{
   return (ssize_t)_read (fd, buf, (int)count);
}
#define bson_close _close
#else
#define bson_open open
#define bson_read read
#define bson_close close
#endif


static BSON_INLINE void
bson_eq_bson (bson_t *bson,
              bson_t *expected)
{
   char *bson_json, *expected_json;
   const uint8_t *bson_data = bson_get_data (bson);
   const uint8_t *expected_data = bson_get_data (expected);
   int unequal;
   unsigned o;
   int off = -1;

   unequal = (expected->len != bson->len)
             || memcmp (bson_get_data (expected), bson_get_data (
                           bson), expected->len);

   if (unequal) {
      bson_json = bson_as_json (bson, NULL);
      expected_json = bson_as_json (expected, NULL);

      for (o = 0; o < bson->len && o < expected->len; o++) {
         if (bson_data [o] != expected_data [o]) {
            off = o;
            break;
         }
      }

      if (off == -1) {
         off = MAX (expected->len, bson->len) - 1;
      }

      fprintf (stderr, "bson objects unequal (byte %u):\n(%s)\n(%s)\n",
               off, bson_json, expected_json);

      {
         int fd1 = bson_open ("failure.bad.bson", O_RDWR | O_CREAT, 0640);
         int fd2 = bson_open ("failure.expected.bson", O_RDWR | O_CREAT, 0640);
         assert (fd1 != -1);
         assert (fd2 != -1);
         assert (bson->len == write (fd1, bson_data, bson->len));
         assert (expected->len == write (fd2, expected_data, expected->len));
         close (fd1);
         close (fd2);
      }

      assert (0);
   }
}


static BSON_INLINE void
run_test (const char *name,
          void (*func) (void))
{
   struct timeval begin;
   struct timeval end;
   struct timeval diff;
   long usec;
   double format;

   fprintf(stdout, "%-42s : ", name);
   fflush(stdout);
   bson_gettimeofday(&begin, NULL);
   func();
   bson_gettimeofday(&end, NULL);
   fprintf(stdout, "PASS");

   diff.tv_sec = end.tv_sec - begin.tv_sec;
   diff.tv_usec = usec = end.tv_usec - begin.tv_usec;
   if (usec < 0) {
      diff.tv_sec -= 1;
      diff.tv_usec = usec + 1000000;
   }

   format = diff.tv_sec + (diff.tv_usec / 1000000.0);
   fprintf(stdout, " : %lf\n", format);
}


BSON_END_DECLS

#endif /* BSON_TESTS_H */
