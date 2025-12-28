#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <random>
#include <algorithm>

std::string expand_home(const std::string& path) {
    if (!path.empty() && path[0] == '~') {
        const char* home = getenv("HOME");
        return std::string(home) + path.substr(1);
    }
    return path;
}

int main(int argc, char* argv[]) {
    std::string file_path;
    const size_t page_size = 4096;
    const size_t total_size = 5 * 1024 * 1024;  // 5MB
    const size_t total_pages = total_size / page_size;
    const int iterations = 500;

    // Configurable parameters
    size_t interval_size = 1 * 1024 * 1024;  // default 1MB
    size_t chunk_size_kb = 16;               // default 16KB

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path> [chunk_size_kb]" << std::endl;
        return 1;
    } else {
        file_path = argv[1];
    }
    if (argc > 2) chunk_size_kb = std::atoi(argv[2]);

    size_t chunk_size = chunk_size_kb * 1024;

    if (interval_size >= total_size) {
        std::cerr << "Interval size must be less than total size (5MB)" << std::endl;
        return 1;
    }

    if (chunk_size == 0 || chunk_size > total_size) {
        std::cerr << "Invalid chunk size: " << chunk_size_kb << "KB" << std::endl;
        return 1;
    }

    file_path = expand_home(file_path);

    std::cout << "Configuration:" << std::endl;
    std::cout << "  File Path:     " << file_path << std::endl;
    std::cout << "  Total Size:    " << total_size / (1024 * 1024) << " MB" << std::endl;
    std::cout << "  Interval Size: " << interval_size / 1024 << " KB" << std::endl;
    std::cout << "  Chunk Size:    " << chunk_size_kb << " KB" << std::endl;

    // Open file
    int fd = open(file_path.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    std::vector<char> page_buffer(page_size, 'A');

    std::cout << "[Phase 1] Randomly writing 5MB in " << chunk_size_kb << "KB chunks..." << std::endl;
    const size_t chunks = total_size / chunk_size;
    
    // Create a vector of offsets and shuffle them
    std::vector<off_t> offsets(chunks);
    for (size_t i = 0; i < chunks; i++) {
        offsets[i] = i * chunk_size;
    }
    
    std::mt19937 rng((unsigned int)time(nullptr));
    std::shuffle(offsets.begin(), offsets.end(), rng);
    
    // Prepare a 16KB buffer
    std::vector<char> chunk_buffer(chunk_size, 'A');
    
    // Perform random writes
    for (size_t i = 0; i < chunks; i++) {
        if (lseek(fd, offsets[i], SEEK_SET) < 0) {
            perror("lseek (phase 1)");
            close(fd);
            return 1;
        }
        
        if (write(fd, chunk_buffer.data(), chunk_size) != (ssize_t)chunk_size) {
            perror("write (phase 1)");
            close(fd);
            return 1;
        }
    }

    std::cout << "[Phase 2] Random region access with interval size = " << interval_size / 1024 << "KB..." << std::endl;

    std::vector<char> rw_buffer(interval_size, 'B');
    std::vector<char> read_buffer(interval_size);
    std::uniform_int_distribution<off_t> dist(0, total_size - interval_size);

    for (int i = 0; i < iterations; ++i) {
        off_t offset = dist(rng);
        if (lseek(fd, offset, SEEK_SET) < 0) {
            perror("lseek (phase 2)");
            close(fd);
            return 1;
        }

        if (i % 2 == 0) {
            // Read
            if (read(fd, read_buffer.data(), interval_size) != (ssize_t)interval_size) {
                perror("read (phase 2)");
                close(fd);
                return 1;
            }
        } else {
            // Write
            if (lseek(fd, offset, SEEK_SET) < 0) {
                perror("lseek (phase 2 write)");
                close(fd);
                return 1;
            }
            if (write(fd, rw_buffer.data(), interval_size) != (ssize_t)interval_size) {
                perror("write (phase 2)");
                close(fd);
                return 1;
            }
        }
    }

    std::cout << "Benchmark completed successfully." << std::endl;
    close(fd);
    return 0;
}
