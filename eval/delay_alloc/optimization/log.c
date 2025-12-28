/* * Copyright (c) 2020 Institution of Parallel and Distributed System, Shanghai Jiao Tong University
 * AtomFS is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <stdio.h>
#include "log.h"

#ifdef DEBUG
FILE *logfile;
FILE *log_open()
{
    logfile = fopen("fs.log", "w");
    if (logfile == NULL) {
		perror("logfile");
		exit(EXIT_FAILURE);
    }
    setvbuf(logfile, NULL, _IOLBF, 0);
    return logfile;
}

void log_msg(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(logfile, format, ap);
}

#else

FILE *log_open() {return NULL;}

void log_msg(const char *format, ...) { }

#endif
