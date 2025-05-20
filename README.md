# Linux-FileOwl
Linux kernel subsystem that monitors file system events. It can be used to detect when files are created, modified, or deleted

# Linux File System

```mermaid
  graph TD
      subgraph Linux File System
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
