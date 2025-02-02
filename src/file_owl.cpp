#include <iostream>
#include <string>
#include <ctime>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>
#include <csignal>
#include <set>
#include <cstring>

#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include <tinyxml2.h>
#include <pwd.h>
#include <dirent.h>

// Mutex for thread safety
std::mutex mtx;

// Buffer to store events temporarily
std::vector<std::string> eventBuffer;

// Struct to store protection rules
struct ProtectionRule {
    std::string path;
    std::string action; // "create", "delete", "modify", "copy", or "all"
};

// Vector to store protection rules
std::vector<ProtectionRule> protectionRules;

// Function to read protection rules from protect.xml
void read_protection_rules(const std::string& protect_file) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(protect_file.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load protect file: " << protect_file << std::endl;
        return;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("protection");
    if (!root) {
        std::cerr << "Invalid XML format: Missing 'protection' root element." << std::endl;
        return;
    }

    for (tinyxml2::XMLElement* rule = root->FirstChildElement("rule"); rule; rule = rule->NextSiblingElement("rule")) {
        const char* type = rule->Attribute("type");
        const char* action = rule->Attribute("action");

        if (type && strcmp(type, "protection") == 0 && action) {
            tinyxml2::XMLElement* fileElement = rule->FirstChildElement("file");
            if (fileElement && fileElement->GetText()) {
                ProtectionRule r;
                r.path = fileElement->GetText();
                r.action = action;
                protectionRules.push_back(r);
            }
        }
    }
}

bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
    if (str1.length() != str2.length()) return false;
    return std::equal(str1.begin(), str1.end(), str2.begin(),
                      [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}
// Function to check if a file is protected based on the rule
bool is_protected(const std::string& filename, const std::string& action) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Checking protection for file: " << filename << ", Action: " << action << std::endl;

    for (const auto& rule : protectionRules) {
        std::cout << "Rule: Path=" << rule.path << ", Action=" << rule.action << std::endl;
        if (filename == rule.path) {
            if (rule.action == "all" || caseInsensitiveCompare(rule.action, action)) {
                std::cout << "File is protected!" << std::endl;
                return true;
            }
        }
    }
    std::cout << "File is NOT protected." << std::endl;
    return false;
}

// Function to log alerts to alert.txt
void log_alert(const std::string& filename, const std::string& action, const std::string& username) {
    std::ofstream alertFile("/home/alert.csv", std::ios_base::app); // Append mode
    if (!alertFile.is_open()) {
        std::cerr << "Failed to open alert file: /home/alert.csv" << std::endl;
        return;
    }

    time_t now = time(nullptr);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    alertFile << timestamp << " - User: " << username << " attempted to " << action
              << " protected file: " << filename << std::endl;

    alertFile.close();
}

// Function to add a watch recursively
void add_watch(int fd, const std::string& path) {
    int wd = inotify_add_watch(fd, path.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE);
    if (wd == -1) {
        perror("inotify_add_watch");
        return;
    }

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string entry_path = path + "/" + entry->d_name;

        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
            continue;
        }

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
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <directory_to_watch>"
                  << " <CSV file to save>"
                  << " <xml file to protect>" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string directory_to_watch = argv[1];
    const std::string csv_file = argv[2];
    const std::string protect_file = argv[3];


    // Read protection rules
    read_protection_rules(protect_file);

    int fd = inotify_init();
    if (fd == -1) {
        perror("inotify_init");
        return EXIT_FAILURE;
    }

    add_watch(fd, directory_to_watch);

    std::thread flush_thread(periodic_flush, csv_file);

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

            time_t now = time(nullptr);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

            event_desc = timestamp;
            event_desc += ", ";

            std::string action;
            if (event->mask & IN_CREATE) {
                action = "CREATE";
            } else if (event->mask & IN_DELETE) {
                action = "DELETE";
            } else if (event->mask & IN_MODIFY) {
                action = "MODIFY";
            } else if (event->mask & IN_MOVED_FROM || event->mask & IN_MOVED_TO) {
                action = "COPY"; // Treat move as copy
            } else {
                action = "UNKNOWN";
            }


            event_desc += action;
            event_desc += ", ";
            event_desc += event_name;

            // Check if the file is protected based on the operation
            if (is_protected(event_name, action)) {
                uid_t uid = geteuid(); // Get the effective user ID
                struct passwd* pw = getpwuid(uid); // Get user information
                std::string username = pw ? pw->pw_name : "unknown";

                // Log the alert
                log_alert(event_name, action, username);

                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "Protected file detected: " << event_name << ". Skipping operation: " << action
                          << ". User: " << username << std::endl;
                // Skip this event and continue processing the next one
                ptr += sizeof(struct inotify_event) + event->len;
                continue;
            }
            // Process non-protected events
            std::lock_guard<std::mutex> lock(mtx);
            eventBuffer.push_back(event_desc);

            // Move to the next event
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    close(fd);
    flush_thread.join();

    return EXIT_SUCCESS;
}