#ifndef TRASHCLI_H
#define TRASHCLI_H

#include <cstddef>
#include <string>
#include <vector>

namespace trashcli {

// ============================================================
// Trash entry
// ============================================================

struct TrashEntry {
    std::string name;
    std::string original_path;
    std::string trash_path;
    std::string deleted_at;
    std::string type;      // "file" or "directory"
    std::size_t size = 0;
};

// ============================================================
// Database
// ============================================================

/*
 * Initialize the SQLite database and create/migrate the
 * required schema.
 */
bool init_database();

/*
 * Close the SQLite database connection.
 */
void close_database();

// ============================================================
// Trash paths / IDs
// ============================================================

/*
 * Return the base trash directory.
 *
 * Usually:
 *   $XDG_DATA_HOME/trash
 *
 * or:
 *   ~/.local/share/trash
 */
std::string get_trash_dir();

/*
 * Generate a unique trash entry ID.
 *
 * Example:
 *   20260911-001816-a83f29c4e912
 */
std::string generate_trash_subdir();

// ============================================================
// Trash operations
// ============================================================

/*
 * Move a regular file into trash.
 *
 * Directories are rejected here.
 * Use trash_recursive() for directories.
 */
bool trash_file(
    const std::string& filepath
);

/*
 * Move an entire directory tree into trash.
 *
 * Despite the historical name "recursive", the implementation
 * moves the directory as one filesystem object rather than
 * manually traversing and moving every child.
 */
bool trash_recursive(
    const std::string& dirpath
);

/*
 * Restore a trash entry by its trash ID.
 *
 * force = false:
 *   Fail if the original path already exists.
 *
 * force = true:
 *   Remove the existing destination and restore.
 */
bool restore_file(
    const std::string& trash_id,
    bool force = false
);

/*
 * Permanently delete a file or directory.
 */
bool permanent_delete(
    const std::string& filepath
);

/*
 * Empty the entire trash.
 *
 * The trash database itself is preserved.
 */
bool empty_trash(
    bool keep_root = true
);

// ============================================================
// Query / information
// ============================================================

/*
 * Return all entries currently recorded in the trash database.
 */
std::vector<TrashEntry> list_trash();

/*
 * Display information about an existing filesystem path.
 *
 * detailed = false:
 *   Human-readable summary.
 *
 * detailed = true:
 *   Detailed size and permissions.
 */
bool show_info(
    const std::string& filepath,
    bool detailed = false
);

} // namespace trashcli

#endif // TRASHCLI_H

