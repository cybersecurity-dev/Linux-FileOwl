#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/inotify.h>
#include <limits.h>
#include <dirent.h>

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

int main() {
    const std::string directory_to_watch = "/home/dir/owl/";  // Change this to your target directory

    // Create an inotify instance
    int fd = inotify_init();
    if (fd == -1) {
        perror("inotify_init");
        return EXIT_FAILURE;
    }

    // Add watches recursively
    add_watch(fd, directory_to_watch);

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
            std::cout << "Event: ";
            if (event->mask & IN_CREATE) {
                std::cout << "CREATE";
            } else if (event->mask & IN_DELETE) {
                std::cout << "DELETE";
            } else if (event->mask & IN_MODIFY) {
                std::cout << "MODIFY";
            } else if (event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO) {
                std::cout << "MOVE";
            } else {
                std::cout << "UNKNOWN";
            }
            std::cout << " on " << event_name << std::endl;

            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    // Clean up
    close(fd);
    return EXIT_SUCCESS;
}
