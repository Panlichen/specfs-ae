#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

std::string expand_home(const std::string& path) {
    if (path[0] == '~') {
        const char* home = getenv("HOME");
        return std::string(home) + path.substr(1);
    }
    return path;
}

int main(int argc, char* argv[]) {
    // Default parameters
    std::string file_path;
    const int page_size = 4096;
    int initial_write_pages = 1;
    int prealloc_stride_pages = 64;
    int initial_write_count = 20;
    int random_write_count = 500;
    int random_offset_page = 2;

    // Allow override from command line
    if (argc > 1) file_path = argv[1];
    if (argc > 2) initial_write_count = std::atoi(argv[2]);
    if (argc > 3) random_write_count = std::atoi(argv[3]);

    file_path = expand_home(file_path);

    // Open the file
    int fd = open(file_path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Allocate write buffer
    std::vector<char> buffer(initial_write_pages * page_size, 0);

    std::cout << "[Phase 1] Generating prealloc pool..." << std::endl;
    for (int i = 0; i < initial_write_count; ++i) {
        off_t offset = static_cast<off_t>(i * prealloc_stride_pages * page_size);
        if (lseek(fd, offset, SEEK_SET) < 0) {
            perror("lseek (phase 1)");
            close(fd);
            return 1;
        }
        if (write(fd, buffer.data(), buffer.size()) != (ssize_t)buffer.size()) {
            perror("write (phase 1)");
            close(fd);
            return 1;
        }
    }

    std::cout << "[Phase 2] Random fragment write to trigger prealloc scan..." << std::endl;
    std::vector<char> small_buf(page_size, 1);
    srand(42);

    for (int i = 0; i < random_write_count; ++i) {
        int base = rand() % initial_write_count;
        int random_offset_pages = (rand() + random_offset_page) % prealloc_stride_pages;
        off_t offset = static_cast<off_t>((base * prealloc_stride_pages + random_offset_pages) * page_size);
        if (lseek(fd, offset, SEEK_SET) < 0) {
            perror("lseek (phase 2)");
            close(fd);
            return 1;
        }
        if (write(fd, small_buf.data(), page_size) != page_size) {
            perror("write (phase 2)");
            close(fd);
            return 1;
        }
    }

    std::cout << "Benchmark completed successfully." << std::endl;
    close(fd);
    return 0;
}
