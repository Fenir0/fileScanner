#include "scanner.hpp"
#include <iostream>
#include <string>

void add_param(std::string param_name, std::string prm){
    if(param_name == "dir"){
        parameters::dirs.push_back(prm);
    }
    if(param_name == "x_dir"){
        parameters::x_dirs.push_back(prm);
    }
    if(param_name == "lvl"){
        parameters::level      = std::stoi(prm);
    }
    if(param_name == "mn_sz"){
        parameters::min_size   = std::stoi(prm);
    }
    if(param_name == "b_sz"){
        parameters::block_size = std::stoi(prm);
    }
    if(param_name == "hsh"){
        parameters::hash_alg   = prm;
    }
    if(param_name == "msk"){
        parameters::file_mask.push_back(prm);
    }
}

bool parameterCheck(std::string st){
    uint8_t it = st.find('=');
    if(it == std::string::npos) return false;
    std::string parameter_name = st.substr(0, it);
    st = st.substr(it+1);
    
    std::string tmp = "";
    for(int i = 0; i < st.size(); i++){
        if(st[i] == ','){
            add_param(parameter_name, tmp);
            tmp = "";
        }
        else {
            tmp += st[i];
        }
    }
    if(!tmp.empty())
        add_param(parameter_name, tmp);
    return true;
}

void setDefaultParameters(){
    add_param("dir", boost::filesystem::current_path().string());
    add_param("lvl", "1");
    add_param("b_sz", "8");
    add_param("hsh", "crc32");
}

int main(int argc, char* argv []){
    setDefaultParameters();

    for(size_t i = 1; i < argc; ++i){
        if(!parameterCheck(std::string(argv[i]))){
            std::cout << "Bad parameter: " << argv[i] << '\n';
        }
    }
    Scanner::runScan();
    return 0;
}