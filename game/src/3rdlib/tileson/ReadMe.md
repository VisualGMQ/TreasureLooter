I changed some code in `Tileson` to make it compile under Android. All compile error are from `std::filesystem`:

* remove all `<filesystem>` header file
* replace all `std::filesystem::path` to `std::string`
* replace `path::parent_path()` to my own `ParentPath()`
* remove read & parse world file due it heavily dependent on `std::filesystem`