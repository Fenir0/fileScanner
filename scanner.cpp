#include "scanner.hpp"

std::map<boost::filesystem::path, std::vector<uint32_t>> Scanner::partFileList = {};
std::map<std::vector<uint32_t>, std::vector<boost::filesystem::path>> Scanner::fullFileList = {};
std::vector<boost::regex> Scanner::file_masks;

std::string Scanner::readFile(const boost::filesystem::path &p, int block_number)
{
    std::ifstream f(p, std::ios::binary);
    if(!f.is_open()) return {};

    std::vector<char> buffer (parameters::block_size, 0);

    std::uint64_t offset = static_cast<std::uint64_t> (block_number)*parameters::block_size;
    f.seekg(offset, std::ios::beg);
    if(!f.good()) return {};
    f.read(buffer.data(), static_cast<std::streamsize> (parameters::block_size));

    std::size_t bytes_read = static_cast<std::size_t>(f.gcount());
    if(bytes_read == 0) return {};
    
    return  std::string (buffer.data(), f.gcount());
}

uint32_t Scanner::hash_crc16(const std::string& data)
{
    boost::crc_16_type result;
    result.process_bytes(data.data(), data.length());
    return result.checksum();
}

uint32_t Scanner::hash_crc32(const std::string& data)
{
    boost::crc_32_type result;
    result.process_bytes(data.data(), data.length());
    return result.checksum();
}

uint32_t Scanner::hash(const std::string& data)
{   
    if(parameters::hash_alg == "crc16"){
        return hash_crc16(data);
    }
    else if(parameters::hash_alg == "crc32"){
        return hash_crc32(data);
    }
    return 0;
}

std::string glob_to_regex(const std::string& mask) {
    std::string regex;
    regex.reserve(mask.size() * 2);

    for (char c : mask) {
        switch(c) {
            case '*': regex += ".*"; break;
            case '?': regex += "."; break;
            case '.': regex += "\\."; break;
            default:
                if(std::isalnum(static_cast<unsigned char>(c)))
                    regex += c;           // safe alphanumerics
                else {
                    // escape special regex characters: ()[]{}+^$|\ etc.
                    regex += '\\';
                    regex += c;
                }
        }
    }
    return regex;
}

bool Scanner::mask_check(const boost::filesystem::path &p)
{   const std::string filename = p.filename().string();
    if(file_masks.empty()) return true;
    for(auto & rgx: file_masks ){
        if(boost::regex_match(filename, rgx))
            return true;
    }
    return false;
}

void Scanner::init_masks()
{
    if(parameters::file_mask.empty()) return;
    for(const auto& m : parameters::file_mask) {
        file_masks.emplace_back(
            glob_to_regex(m),
            boost::regex::icase   // case-insensitive
        );
    }
}

void Scanner::runScan(){
    std::deque<boost::filesystem::path> dirs_list;
    for(auto d: parameters::dirs) dirs_list.emplace_back(d);

    init_masks();

    while(!dirs_list.empty()) {
        auto dir = dirs_list.front();
        dirs_list.pop_front();

        if(!boost::filesystem::exists(dir)) continue;
        
        boost::filesystem::directory_iterator begin(dir);
        boost::filesystem::directory_iterator end;

        for(; begin != end; ++begin){
            boost::filesystem::file_status fs = boost::filesystem::status(*begin);
            std::cout << "Checking file: " << *begin << '\n';
            
            // if file is excluded
            if(std::find(parameters::x_dirs.begin(),parameters::x_dirs.end(), *begin) != parameters::x_dirs.end()){
                std::cout << *begin << " skipped\n";
                continue;
            }

            // If this is a directory
            if(fs.type() == boost::filesystem::directory_file){
                // 0 => no diving
                if(parameters::level)
                    dirs_list.emplace_back(*begin);
                continue;
            }
        /* check for parameters settings */ 

            // if file is too small
            if(boost::filesystem::file_size(*begin) < parameters::min_size){
                std::cout << *begin << " skipped\n";
                continue;
            }
            // check for mask  (TO DO TO DO)
            if(!mask_check(*begin)) continue;

            
            /* - - - */
            compare_with_all(*begin);
        }
    }
    report();
}

void Scanner::compare_with_all(const boost::filesystem::path &THIS_FILE){
    std::vector<uint32_t> hash_this = {};
    if(partFileList.empty()){ 
        partFileList[THIS_FILE] = {};
        return;
    }
    else{
        std::vector<uint32_t> hash_cur = {};

        for(auto it = partFileList.begin(); it != partFileList.end();){
            const boost::filesystem::path & fileName = it->first; 
            std::vector<uint32_t>         & hash_past = it->second; 

            auto[new_hash_this, hash_cur] = compare(THIS_FILE, fileName, hash_this, hash_past);
            hash_this = std::move(new_hash_this);
            if(hash_cur == hash_this){ 
                // If hashes are equal => both files are fully read
                fullFileList[hash_this].push_back(THIS_FILE);
                fullFileList[hash_this].push_back(fileName);
                it = partFileList.erase(it);
                return;
            }
            if(hash_cur.size() > hash_past.size()){
                // not equal => current file was read into further
                it->second = hash_cur;
            }
            it++;
        }
    }


    for(auto & [current_hash, filelist]: fullFileList){
        hash_this = compare(THIS_FILE, hash_this, current_hash);
        if(hash_this == current_hash){
            filelist.push_back(THIS_FILE);
            return;
        }
    }
    partFileList[THIS_FILE] = hash_this;

}

/// @brief  Compare two files
/// @param p1 filename
/// @param p2 filename
std::tuple<std::vector<uint32_t>,std::vector<uint32_t>> 
Scanner::compare(const boost::filesystem::path &p1, const boost::filesystem::path &p2, std::vector<uint32_t> hash1, std::vector<uint32_t> hash2){
    uint32_t f1_cur, f2_cur;
    while(hash1.size() < hash2.size()){
        std::string s1 = readFile(p1, static_cast<int>(hash1.size()));
        if(s1.empty()) break;
        f1_cur = hash(s1);
        hash1.push_back(f1_cur);
        if(hash2[hash1.size()-1] != f1_cur) return {hash1, hash2};
    }
    while(hash2.size() < hash1.size()){
        std::string s2 = readFile(p2, static_cast<int>(hash2.size()));
        if(s2.empty()) break;
        f2_cur = hash(s2);
        hash2.push_back(f2_cur);
        if(hash1[hash2.size()-1] != f2_cur) return {hash1, hash2};
    }
    while(true){
        std::string s1 = readFile(p1, static_cast<int>(hash1.size()));
        std::string s2 = readFile(p2, static_cast<int>(hash2.size()));
        if(s1.empty() && s2.empty()) return {hash1, hash2};
        if(!s1.empty()) hash1.push_back(hash(s1));
        if(!s2.empty()) hash2.push_back(hash(s2));

        if(hash1.empty() != hash2.empty() && hash1.back() != hash2.back()) return {hash1, hash2};
    }
    return {hash1, hash2}; 
}

std::vector<uint32_t> Scanner::compare(const boost::filesystem::path &THIS_FILE, std::vector<uint32_t> hash1, const std::vector<uint32_t> hash2)
{
    while(hash1.size() < hash2.size()){
        std::string s1 = readFile(THIS_FILE, static_cast<int>(hash1.size()));
        if(s1.empty()) break;
        hash1.push_back(hash(s1));
        if(hash2[hash1.size()-1] != hash1.back()) break;
    }
    return hash1;
}


void Scanner::report()
{
    if(fullFileList.empty()) {
        std::cout << "No duplicates found\n";
        return;
    }
    std::cout << "Found:\n";
    int i = 1;
    for(auto [hash, filename]: Scanner::fullFileList){
        std::printf("№%d:\n",i);
        for(auto & s: filename){
            std::cout << s << '\n';
        }
        i++;
    }
}

void Scanner::logHash(std::vector<uint32_t> hash){
    for(auto i: hash){
        std::cout << i << ' ';
    }
    std::cout << '\n';
}
