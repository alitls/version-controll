#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <zlib.h>

int myGit(int argc, char **pString);

int main(int argc, char *argv[]) {
    return myGit(argc, argv);
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
        char object_path[256];
        sprintf(object_path, ".git/objects/%c%c/%s", object_hash[0], object_hash[1], object_hash + 2);

        FILE *object_file = fopen(object_path, "rb");
        if (object_file == NULL) {
            fprintf(stderr, "Failed to open object file: %s\n", strerror(errno));
            return 1;
        }

        fseek(object_file, 0, SEEK_END); //jump cursor to end
        long size = ftell(object_file);
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


        unsigned char *content = memchr(decompressed, 0, stream.total_out);
        if (!content) {
            fprintf(stderr, "Invalid object format\n");
            return 1;
        }
        content++;
        fwrite(content, 1, stream.total_out - (content - decompressed), stdout);

    } else {
        fprintf(stderr, "Unknown command %s\n", command);
        return 1;
    }

    return 0;
}
