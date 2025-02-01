#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>

#include <unistd.h>
#include <sys/inotify.h>
#include <limits.h>
#include <dirent.h>

// Mutex for thread safety
std::mutex mtx;

// Buffer to store events temporarily
std::vector<std::string> eventBuffer;

// Function to add a watch recursively
void add_watch(int fd, const std::string& path) {
    // Add a watch for the current directory
    int wd = inotify_add_watch(fd, path.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE);
    if (wd == -1) {
        perror("inotify_add_watch");
        return;
    }

    // Open the directory and iterate through its entries
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string entry_path = path + "/" + entry->d_name;

        // Skip "." and ".."
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
            continue;
        }

        // If it's a directory, add a watch for it and its contents
        if (entry->d_type == DT_DIR) {
            add_watch(fd, entry_path);
        }
    }

    closedir(dir);
}

// Function to write buffered events to CSV file
void write_to_csv(const std::string& csv_file) {
    std::lock_guard<std::mutex> lock(mtx);

    if (eventBuffer.empty()) {
        return;
    }

    std::ofstream ofs(csv_file, std::ios_base::app);  // Append mode
    if (!ofs.is_open()) {
        std::cerr << "Failed to open CSV file: " << csv_file << std::endl;
        return;
    }

    for (const auto& event : eventBuffer) {
        ofs << event << std::endl;
    }

    ofs.close();
    eventBuffer.clear();  // Clear the buffer after writing
}

// Function to periodically flush the buffer to CSV
void periodic_flush(const std::string& csv_file) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::minutes(1));  // Wait for one minute
        write_to_csv(csv_file);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <directory_to_watch>" << " <CSV file to save>" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string directory_to_watch = argv[1];
    const std::string csv_file = argv[2];

    // Create an inotify instance
    int fd = inotify_init();
    if (fd == -1) {
        perror("inotify_init");
        return EXIT_FAILURE;
    }

    // Add watches recursively
    add_watch(fd, directory_to_watch);

    // Start the periodic flush thread
    std::thread flush_thread(periodic_flush, csv_file);

    // Buffer to store inotify events
    char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    while (true) {
        int length = read(fd, buffer, sizeof(buffer));
        if (length < 0) {
            perror("read");
            break;
        }

        for (char* ptr = buffer; ptr < buffer + length; ) {
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>(ptr);

            std::string event_name = (event->len > 0) ? event->name : "unknown";
            std::string event_desc;

            // Get the current timestamp
            time_t now = time(nullptr);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

            event_desc = timestamp;
            event_desc += ", ";

            if (event->mask & IN_CREATE) {
                event_desc += "CREATE";
            } else if (event->mask & IN_DELETE) {
                event_desc += "DELETE";
            } else if (event->mask & IN_MODIFY) {
                event_desc += "MODIFY";
            } else if (event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO) {
                event_desc += "MOVE";
            } else {
                event_desc += "UNKNOWN";
            }

            event_desc += ", ";
            event_desc += event_name;

            // Lock the mutex and add the event to the buffer
            std::lock_guard<std::mutex> lock(mtx);
            eventBuffer.push_back(event_desc);

            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    // Clean up
    close(fd);
    flush_thread.join();  // Ensure the flush thread is finished before exiting

    return EXIT_SUCCESS;
}