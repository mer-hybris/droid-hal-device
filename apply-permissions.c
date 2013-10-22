/**
 * apply-permissions: Fix ownership/group/permissions in extracted CM releases
 *
 * Contact: Thomas Perl <thomas.perl@jolla.com>
 *
 * Copyright (c) 2013, Jolla Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of Jolla Ltd. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/



#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "android_filesystem_config.h"


struct context {
    // Options
    int verbose;
    int debug;
    int simulate;

    // Statistics
    struct {
        int dirs;
        int files;
    } count;

    // Global state
    char *base;
    int strip;
};

// Callback for visiting each node in a directory tree walk
typedef void (*visitor_t)(const char *path, struct stat *st, void *user_data);

// Walk the tree at root, call visitor(path, user_data) for each node
void walk(const char *root, visitor_t visitor, void *user_data);

// Lookup the symbolic names of a UID or GID from the Android system
const char *android_uid_name(unsigned uid);



#define FAIL_IF(x) do { \
    if ((x)) { \
        fprintf(stderr, "Error: %s (%s:%d)\n", #x, __FILE__, __LINE__); \
        exit(3); \
    } \
} while (0)




void apply_android_perms(const char *filename, struct stat *st, void *user_data)
{
    struct context *ctx = (struct context *)user_data;

    unsigned uid = 0;
    unsigned gid = 0;
    unsigned mode = 0;
    uint64_t capa = 0;
    const char *tuid = NULL;
    const char *tgid = NULL;

    // Strip leading components, e.g. "../system/foo" -> "system/foo"
    const char *filename_fs = filename + ctx->strip;

    fs_config(filename_fs, S_ISDIR(st->st_mode), &uid, &gid, &mode, &capa);

    tuid = android_uid_name(uid);
    tgid = android_uid_name(gid);

    if (ctx->verbose) {
        printf("chmod %04o %s\n", mode, filename);
    }

    if (!ctx->simulate && chmod(filename, mode) != 0) {
        fprintf(stderr, "Cannot 'chmod %04o %s': %s\n", mode, filename, strerror(errno));
        exit(2);
    }

    if (ctx->verbose) {
        printf("chown %d:%d %s\n", uid, gid, filename);
    }

    if (!ctx->simulate && chown(filename, uid, gid) != 0) {
        fprintf(stderr, "Cannot 'chown %s:%s %s': %s\n", tuid, tgid, filename, strerror(errno));
        exit(2);
    }

    if (ctx->debug) {
        fprintf(stderr, "%04o   %5d:%5d   %8s:%8s   %s\n", mode, uid, gid, tuid, tgid, filename_fs);
    }

    if (S_ISDIR(st->st_mode)) {
        ctx->count.dirs++;
    } else {
        ctx->count.files++;
    }
}


int main(int argc, char *argv[])
{
    static struct context ctx;

    if (argc < 2) {
        printf("Usage: %s [-v] [-d] [-s] path1 [...]\n\n", argv[0]);
        printf("  -v .... verbose (log chmod/chown commands)\n");
        printf("  -d .... debug (detailed permissions output)\n");
        printf("  -s .... simulate (don't run chown/chmod)\n");
        printf("\n");
        printf("  Example: %s system\n", argv[0]);
        printf("\n");
        return 1;
    }

    int i = 1;

    while (i < argc && argv[i][0] == '-') {
        if (strcmp(argv[i], "-v") == 0) {
            ctx.verbose = 1;
        } else if (strcmp(argv[i], "-d") == 0) {
            ctx.debug = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            ctx.simulate = 1;
        }

        i++;
    }

    while (i < argc) {
        ctx.base = argv[i++];

        // strip trailing slashes
        char *last = ctx.base + strlen(ctx.base) - 1;
        while (*last == '/') *last-- = '\0';

        last = strrchr(ctx.base, '/');
        if (last != NULL) {
            // Number of characters to strip to get Android-relative
            // path name, e.g. "../../something/system/foo" -> "system/foo"
            //                  ^--------------^
            //                     strip this
            ctx.strip = last - ctx.base + 1;
        } else {
            // No slash in the path means we have a relative path like "system"
            ctx.strip = 0;
        }

        walk(ctx.base, apply_android_perms, &ctx);
    }

    if (ctx.verbose) {
        fprintf(stderr, "Updated permissions of %d files and %d directories\n",
                ctx.count.files, ctx.count.dirs);
    }

    return 0;
}




void walk(const char *root, visitor_t visitor, void *user_data)
{
    DIR *dir;
    struct dirent *ent;
    char path[MAXPATHLEN];
    struct stat st;

    dir = opendir(root);
    FAIL_IF(dir == NULL);

    while ((ent = readdir(dir)) != NULL) {
        if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", root, ent->d_name);
        int res = stat(path, &st);
        FAIL_IF(res != 0);

        if (S_ISDIR(st.st_mode)) {
            walk(path, visitor, user_data);
        }

        visitor(path, &st, user_data);
    }

    closedir(dir);
}


const char *android_uid_name(unsigned uid)
{
    for (int i = 0; i < android_id_count; i++) {
        if (uid == android_ids[i].aid) {
            return android_ids[i].name;
        }
    }

    return NULL;
}
