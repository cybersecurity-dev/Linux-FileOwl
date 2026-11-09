<p align="center">
    <a href="https://en.wikibooks.org/wiki/The_Linux_Kernel/Syscalls">
      <img width="15%" src="https://github.com/cybersecurity-dev/cybersecurity-dev/blob/main/assets/Tux.svg" />
    </a>
</p>

```mermaid
flowchart LR
    subgraph K["Linux Kernel Space"]
        FS["File-System Operations"]
        IN["inotify Subsystem"]
        FS --> IN
    end

    subgraph U["User-Space Monitoring Agent"]
        WM["Watch Manager"]
        EC["Event Collector"]
        EP["Event Parser and Enrichment"]
        DE["Detection Engine"]

        WM -->|"Registers watched paths"| IN
        IN -->|"File-system events"| EC
        EC --> EP
        EP --> DE
    end

    subgraph C["Context Sources"]
        PROC["/proc Process Metadata"]
        HASH["File Hashing: SHA-256"]
        USER["User and Permission Data"]
    end

    PROC --> EP
    HASH --> EP
    USER --> EP

    subgraph O["Outputs"]
        DB[("Event Database")]
        LOG["Structured Logs: JSON"]
        ALERT["Alert Manager"]
        DASH["Monitoring Dashboard"]
    end

    DE --> DB
    DE --> LOG
    DE --> ALERT
    DB --> DASH
    ALERT --> DASH

    ALERT -->|"Optional"| SIEM["SIEM: Wazuh, Elastic, or Splunk"]
```

# Linux-FileOwl
> Linux kernel subsystem that monitors file system events. It can be used to detect when files are created, modified, or deleted
<div/>

[![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)]()
[![YouTube](https://img.shields.io/badge/YouTube-%23FF0000.svg?style=for-the-badge&logo=YouTube&logoColor=white)]()
[![Reddit](https://img.shields.io/badge/Reddit-FF4500?style=for-the-badge&logo=reddit&logoColor=white)]()

<p align="center">
    <a href="https://github.com/cybersecurity-dev/"><img height="25" src="https://github.com/cybersecurity-dev/cybersecurity-dev/blob/main/assets/github.svg" alt="GitHub"></a> &nbsp;
    <a href="https://www.youtube.com/@CyberThreatDefense"><img height="25" src="https://github.com/cybersecurity-dev/cybersecurity-dev/blob/main/assets/youtube.svg" alt="YouTube"></a> &nbsp;
    <a href="https://cyberthreatdefence.com/my_awesome_lists"><img height="20" src="https://github.com/cybersecurity-dev/cybersecurity-dev/blob/main/assets/blog.svg" alt="My Awesome Lists"></a>
    <img src="https://github.com/cybersecurity-dev/cybersecurity-dev/blob/main/assets/bar.gif">
</p>

<details>
<summary>Linux File System Hierarchy</summary>

```mermaid
  graph TD
      subgraph "Linux File System Hierarchy"
          Root(/) --> bin[bin: Essential command binaries]
          Root(/) --> boot[boot: System boot loader files]
          Root(/) --> dev[dev: Device files]
          Root(/) --> etc[etc: Host-specific system-wide configuration files]
          Root(/) --> home[home: User home directory]
          Root(/) --> lib[lib: Shared library modules]
          Root(/) --> media[media: Media file such as CD-ROM]
          Root(/) --> mnt[mnt: Temporary mounted filesystems]
          Root(/) --> opt[opt: Add-on application software packages]
          Root(/) --> proc[proc: Interface to kernel data structures]
          Root(/) --> root_dir[root: Home directory for root user]
          Root(/) --> run[run: Run-time program data]
          Root(/) --> sbin[sbin: System binaries]
          Root(/) --> srv[srv: Site-specific data served by this system]
          Root(/) --> sys[sys: Virtual directory providing information about the system]
          Root(/) --> tmp[tmp: Temporary files]
          Root(/) --> usr[usr: Unix System Resources]
          Root(/) --> var[var: File that is expected to continuously change]
    end
```
</details>

##

### Contributing

[Contributions of any kind welcome, just follow the guidelines](contributing.md)!

### Contributors

[Thanks goes to these contributors](https://github.com/cybersecurity-dev/Linux-FileOwl/graphs/contributors)!

[🔼 Back to top](#linux-fileowl)
