#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <zlib.h>
#include <openssl/sha.h>

int myGit(int argc, char **pString);

int zlib_cd();

void hash();

void command_ls_tree();

int main(int argc, char *argv[]) {
    return myGit(argc, argv);
}

void hash() {
    unsigned char hash[SHA_DIGEST_LENGTH];

    const char *msg = "Hello world";
    SHA1((unsigned char *) msg, strlen(msg), hash);

    printf("%s\n", hash);
}

int zlib_cd() {
    const char *input = "Hello, world! This is a zlib compression test.";
    uLong input_size = strlen(input) + 1; // include '\0'

    // Buffers for compressed and decompressed data
    unsigned char compressed[1024];
    unsigned char decompressed[1024];

    uLong compressed_size = sizeof(compressed);
    uLong decompressed_size = sizeof(decompressed);

    // Compress the input string
    if (compress(compressed, &compressed_size, (const Bytef *) input, input_size) != Z_OK) {
        printf("Compression failed!\n");
        return 1;
    }

    printf("Original size: %lu bytes\n", input_size);
    printf("Compressed size: %lu bytes\n", compressed_size);

    // Decompress back into decompressed buffer
    if (uncompress(decompressed, &decompressed_size, compressed, compressed_size) != Z_OK) {
        printf("Decompression failed!\n");
        return 1;
    }

    printf("Decompressed size: %lu bytes\n", decompressed_size);
    printf("Decompressed string: %s\n", decompressed);

    return 0;
}


int myGit(int argc, char *argv[]) {

    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);


    if (argc < 2) {
        fprintf(stderr, "Usage: ./your_program.sh <command> [<args>]\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "init") == 0) {
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        fprintf(stderr, "Logs from your program will appear here!\n");

        // Uncomment this block to pass the first stage

        if (mkdir(".git", 0755) == -1 ||
            mkdir(".git/objects", 0755) == -1 ||
            mkdir(".git/refs", 0755) == -1) {
            fprintf(stderr, "Failed to create directories: %s\n", strerror(errno));
            return 1;
        }

        FILE *headFile = fopen(".git/HEAD", "w");
        if (headFile == NULL) {
            fprintf(stderr, "Failed to create .git/HEAD file: %s\n", strerror(errno));
            return 1;
        }
        fprintf(headFile, "ref: refs/heads/main\n");
        fclose(headFile);

        printf("Initialized git directory\n");
    } else if (strcmp(command, "cat-file") == 0) {
        if (argc != 4 || strcmp(argv[2], "-p") != 0) {
            fprintf(stderr, "Usage ./your_program.sh cat-file -p <hash>\n");
            return 1;
        }

        const char *object_hash = argv[3];
        //put string(path) to buffer
        char object_path[256];
        sprintf(object_path, ".git/objects/%c%c/%s", object_hash[0], object_hash[1], object_hash + 2);

        FILE *object_file = fopen(object_path, "rb");
        if (object_file == NULL) {
            fprintf(stderr, "Failed to open object file: %s\n", strerror(errno));
            return 1;
        }

        fseek(object_file, 0, SEEK_END); //jump cursor to end
        long size = ftell(object_file);
        printf("size of file: %ld\n", size);
        fseek(object_file, 0, SEEK_SET); //back cursor to start

        unsigned char *compressed_data = malloc(size);
        if (fread(compressed_data, 1, size, object_file) != size) {
            fprintf(stderr, "Failed to read object file: %s\n", strerror(errno));
            fclose(object_file);
            free(compressed_data);
            return 1;
        }
        fclose(object_file);

        unsigned char decompressed[65536]; // Adjust as needed
        //z_stream is a struct used by zlib to keep track of compression/decompression state.
        z_stream stream = {
                .next_in = compressed_data,
                .avail_in = size, //size of compress input
                .next_out = decompressed,
                .avail_out = sizeof(decompressed),
        };

        //inflateInit → prepares the z_stream for decompression.
        if (inflateInit(&stream) != Z_OK) {
            fprintf(stderr, "inflateInit failed\n");
            free(compressed_data);
            return 1;
        }


        //inflate → actually decompresses the data.
        //Z_FINISH → tells zlib we expect all input to be consumed and output finished.
        //Expected return: Z_STREAM_END = success, full data decompressed.
        if (inflate(&stream, Z_FINISH) != Z_STREAM_END) {
            fprintf(stderr, "inflate failed\n");
            inflateEnd(&stream);
            free(compressed_data);
            return 1;
        }


        //inflateEnd → releases memory zlib allocated internally.
        //free(compressed_data) → releases the input buffer since it’s no longer needed.
        inflateEnd(&stream);
        free(compressed_data);



        //"blob <size>\0<actual file data>"
        unsigned char *content = memchr(decompressed, 0, stream.total_out);
        if (!content) {
            fprintf(stderr, "Invalid object format\n");
            return 1;
        }
        //skip past the \0, so now content points to the actual file data.
        content++;
        //stream.total_out - (content - decompressed) ==> how many bytes to print
        //(total decompressed size minus the header length)
        //content - decompressed = 8 (because "blob 12\0" is 8 bytes long)
        fwrite(content, 1, stream.total_out - (content - decompressed), stdout);

    } else if (strcmp(command, "hash-object") == 0) {
        // 1 - check command is ok
        if (argc != 4 || strcmp(argv[2], "-w") != 0) {
            fprintf(stderr, "Usage ./your_program.sh hash-object -w <object>\n");
            return 1;
        }

        char *file_path = argv[3];
        printf("%s\n", file_path);

        //just a new file ready to create object from it.
        FILE *fp = fopen(file_path, "rb");
        if (!fp) {
            fprintf(stderr, "Failed to open file\n");
            return 78;
        }

        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        rewind(fp); //back to beginning

        //content of file
        char *content = malloc(file_size);
        if (content == NULL) {
            fprintf(stderr, "Failed to get memory for content: %s\n", strerror(errno));
        }
        fread(content, 1, file_size, fp);
        fclose(fp);

        //format of compressed file blob <size>\0<content>
        //header of compressed file: blob <size>\0
        char header[64];
        int header_len = snprintf(header, sizeof(header), "blob %ld", file_size);

        //full_len is header + \0 + file_size
        int full_len = header_len + 1 + file_size;
        unsigned char *buffer = malloc(full_len);
        //copy header to buffer
        memcpy(buffer, header, header_len);
        //adding \0 char
        buffer[header_len] = '\0';
        //copy content to end of buffer
        memcpy(buffer + header_len + 1, content, file_size);

        //hash with openssl/sha.h
        //sha-1 digest is always 20 byte
        unsigned char sha1_hash[SHA_DIGEST_LENGTH];
        SHA1(buffer, full_len, sha1_hash);

        //Each byte will be shown as two hexadecimal characters (00–ff).
        //20 bytes × 2 chars = 40 characters plus 1 byte for the string terminator '\0'.
        char sha1_hex[41];
        //convert each byte to two hex digit
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
            sprintf(sha1_hex + i * 2, "%02x", sha1_hash[i]);
        }
        sha1_hex[40] = '\0';

        //Returns the maximum possible size the compressed data could take
        uLong compressed_size = compressBound(full_len);
        //holds “biggest size we might need
        Bytef *compressed_data = malloc(compressed_size);
        if (compressed_data == NULL) {
            fprintf(stderr, "Failed to get memory for compressed_data: %s\n", strerror(errno));
        }

        if (compress(compressed_data, &compressed_size, (Bytef *) buffer, full_len) != Z_OK) {
            fprintf(stderr, "Compression failed!");
            exit(99);
        }

        char dir[64];
        snprintf(dir, sizeof(dir), ".git/objects/%.2s", sha1_hex);
        if (mkdir(dir, 0755) == -1) {
            fprintf(stderr, "Failed to create directory: %s\n", strerror(errno));
        }

        char file[128];
        sprintf(file, ".git/objects/%.2s/%s", sha1_hex, sha1_hex + 2);
        printf("file: %s\n", file);

        FILE *ffp = fopen(file, "wb");
        if (!ffp) {
            fprintf(stderr, "Failed to open file for writing binary: %s\n", strerror(errno));
            return 79;
        }

        fwrite(compressed_data, 1, compressed_size, ffp);

        fclose(ffp);

        printf("%s\n", sha1_hex);

        free(buffer);
        free(compressed_data);
        free(content);

    } else if (strcmp(command, "ls-tree") == 0) {
        if (argc != 4 || strcmp(argv[2], "--name-only") != 0) {
            fprintf(stderr, "Usage ./your_program.sh ls-tree --name-only <tree_sha>\n");
            return 1;
        }
        command_ls_tree(argv[2], argv[3]);
    } else {
        fprintf(stderr, "Unknown command %s\n", command);
        return 1;
    }

    return 0;
}

void command_ls_tree(char *flag, char *tree_sha) {
    if (!tree_sha) {
        fprintf(stderr, "Invalid tree_sha: %s\n", strerror(errno));
        exit(99);
    }

    char path[128];
    sprintf(path, ".git/objects/%.2s/%s", tree_sha, tree_sha + 2);

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "Could not open tree object: %s\n", strerror(errno));
        exit(99);
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    Bytef *content = malloc(file_size);
    if (!content) {
        fprintf(stderr, "malloc error: %s\n", strerror(errno));
        fclose(fp);
        exit(99);
    }
    if (fread(content, 1, file_size, fp) != file_size) {
        fprintf(stderr, "File size is diff with malloc size: %s\n", strerror(errno));
        fclose(fp);
        free(content);
        exit(99);
    }
    fclose(fp);


    uLongf buffer_size = 4096;
    Bytef *buffer = malloc(buffer_size);
    if (!buffer) {
        fprintf(stderr, "malloc error: %s\n", strerror(errno));
        free(content);
        exit(99);
    }

    int status = uncompress(buffer, &buffer_size, content, file_size);

    if (status == Z_BUF_ERROR) {
        fprintf(stderr, "Buffer error: %sa\n", strerror(errno));
        free(content);
        free(buffer);
        exit(100);
    }

    if (status != Z_OK) {
        fprintf(stderr, "Decompression failed!: %s\n", strerror(errno));
        free(buffer);
        exit(897);
    }

    free(content);

    Bytef *end = buffer + buffer_size;
    Bytef *ptr = memchr(buffer, '\0', buffer_size) + 1;
    while (ptr != end) {
        Bytef *name = memchr(ptr, ' ', end - ptr) + 1;
        printf("%s\n", name);
        Bytef *nul = memchr(name, '\0', end - name);
        ptr = nul + 21;
    }

    free(buffer);
}
