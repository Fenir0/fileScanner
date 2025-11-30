#ifndef SCANNER
#define SCANNER

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/crc.hpp>
#include <boost/regex.hpp>

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <map>
#include <deque>

namespace parameters {
    inline std::vector<std::string> dirs;
    inline std::vector<std::string> x_dirs;
    
    inline bool           level;
    inline uint32_t    min_size;
    inline uint32_t  block_size;

    inline std::string hash_alg;
    
    inline std::vector<std::string> file_mask;
};

class Scanner{
    
    // check file -> hash file -> store -> compare -> output
    private:
    static bool     mask_check(const boost::filesystem::path &p);   
    static std::vector<boost::regex> file_masks;

    static void init_masks();

    // files that are not fully read yet
    static std::map<boost::filesystem::path , std::vector<std::uint32_t> > 
         partFileList; 

    // FULLY READ&HASHED files 
    static std::map<std::vector<std::uint32_t>, std::vector<boost::filesystem::path> > 
        fullFileList; 

    static std::string readFile(const boost::filesystem::path &p, int block_number);

    static uint32_t              hash_crc16(const std::string& data);
    static uint32_t              hash_crc32(const std::string& data);

    static uint32_t hash      (const std::string& data);

    static void     compare_with_all(const boost::filesystem::path &p);

    /// @brief Compare two files from scratch or with given unfull hash
    /// @param p1 File 1
    /// @param p2 File 2
    /// @param hash1 Hash of file1
    /// @param hash2 Hash of file2
    /// @return 
    static std::tuple<std::vector<uint32_t>,std::vector<uint32_t>> compare(
            const boost::filesystem::path &p1, const boost::filesystem::path &p2, 
                 std::vector<uint32_t> hash1 = {}, std::vector<uint32_t> hash2 = {});
    static std::vector<uint32_t> compare(
            const boost::filesystem::path &p1, std::vector<uint32_t> hash1, const std::vector<uint32_t> hash2);
    static void report();

    public:
    
    static void     runScan();

    static void     logHash(std::vector<uint32_t> hash);
};

#endif