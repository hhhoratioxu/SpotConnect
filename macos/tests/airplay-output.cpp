// Copyright (C) 2026 hhhoratioxu. GPL-3.0-or-later (links upstream RAOP).
extern "C" {
#include "raop_client.h"
#include "cross_log.h"
#include "cross_ssl.h"
log_level raop_loglevel = lERROR, util_loglevel = lERROR;
}
#include "../../spotraop/src/HoriAirplayOutput.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <thread>
#include <vector>
#include "nlohmann/json.hpp"
static void require(bool yes, const char* text) { if (!yes) throw std::runtime_error(text); }
template<class F> static void until(F check) {
    for (int i=0;i<200;++i) { if(check())return;std::this_thread::sleep_for(std::chrono::milliseconds(5)); }
    throw std::runtime_error("bounded adapter observation timed out");
}
static std::string contents(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file),{}};
}
int main(int argc,char** argv) {
    raopcl_s* output=nullptr;
    std::filesystem::path folder;
    try {
        require(argc==3,"test needs case and fixture path");
        const std::string test=argv[1];
        char temporary[]="/tmp/hori-output-test.XXXXXX";
        auto made=mkdtemp(temporary);require(made,"fixture directory");folder=made;
        setenv("HORISPOT_AIRPLAY2_SENDER",argv[2],1);
        setenv("HORI_FAKE_CAPTURE",folder.c_str(),1);
        setenv("HORI_FAKE_MODE",test=="failure"?"error":"normal",1);
        unsetenv("HORISPOT_AIRPLAY_DISCOVERY");
        if (test.rfind("txt_",0)==0) {
            const auto cache=folder/"receiver.json";
            std::ofstream fixture(cache);
            if (test=="txt_malformed") fixture<<"{not a JSON document";
            else {
                // Deliberately put another device first: only the peer's real
                // advertised properties may be forwarded to the child.
                fixture<<R"({"devices":[
                    {"service":"other._airplay._tcp.local.","host":"127.0.0.2","advertised":{"model":"Decoy","osvers":"99.0"}},
                    {"service":"selected._airplay._tcp.local.","host":")"
                    <<(test=="txt_mismatch"?"127.0.0.3":"127.0.0.1")
                    <<R"(","advertised":{"model":"AudioAccessory6,1","osvers":"27.2","features":"0x1,0x2","flags":"invalid whitespace","password":"never forwarded"}}
                ]})";
            }
            fixture.close();
            setenv("HORISPOT_AIRPLAY_DISCOVERY",cache.c_str(),1);
        }
        require(cross_ssl_load(),"SSL library initialization");
        in_addr local{};inet_pton(AF_INET,"127.0.0.1",&local);
        char cipher[]="0";
        output=raopcl_create(local,0,1,nullptr,nullptr,RAOP_ALAC,352,44100,
            RAOP_CLEAR,false,nullptr,nullptr,cipher,nullptr,"fixture",44100,16,2,-24);
        require(output,"real RAOP identity creation");
        require(hori_output_register(output,local,"AudioAccessory6,1"),"native route registration");
        require(raopcl_connect(output,local,7000,true),"child connected");
        if (test.rfind("txt_",0)==0) {
            const auto args=nlohmann::json::parse(contents(folder/"arguments.json")).get<std::vector<std::string>>();
            const auto found=std::find(args.begin(),args.end(),"--txt");
            if (test=="txt_match") {
                require(found!=args.end()&&std::next(found)!=args.end(),"matched receiver TXT must reach child");
                require(*std::next(found)=="model=AudioAccessory6,1 osvers=27.2 features=0x1,0x2",
                    "forward only actual safe peer properties as one argument");
            } else require(found==args.end(),"unmatched or malformed cache must not invent receiver capabilities");
        }
        std::array<uint8_t,1408> pcm{};
        for(size_t i=0;i<pcm.size();++i)pcm[i]=(i*31+17)%256;
        uint64_t timestamp=0;
        require(raopcl_send_chunk(output,pcm.data(),352,&timestamp),"PCM accepted");
        if(test=="failure") {
            until([&]{return !raopcl_is_sane(output);});
            require(!raopcl_accept_frames(output),"failed child must stop consumption");
            require(!raopcl_send_chunk(output,pcm.data(),352,&timestamp),"failed child must retain caller PCM");
        } else {
            until([&]{return raopcl_state(output)==RAOP_STREAMING;});
            until([&]{return contents(folder/"pcm").size()==pcm.size();});
            const auto actual=contents(folder/"pcm");
            require(!std::memcmp(actual.data(),pcm.data(),pcm.size()),"exact binary PCM must reach child stdin");
            if(test=="controls") {
                raopcl_pause(output);require(raopcl_flush(output),"flush ack");
                require(!raopcl_accept_frames(output),"paused sink must apply backpressure");
                require(raopcl_connect(output,local,7000,true),"warm resume");
                require(raopcl_set_volume(output,-15),"volume command");
                require(raopcl_set_progress_ms(output,12000,60000),"seek progress command");
                require(raopcl_send_chunk(output,pcm.data(),352,&timestamp),"new PCM after acknowledged flush");
                until([&]{return raopcl_state(output)==RAOP_STREAMING;});
                until([&]{return contents(folder/"commands").find("VOLUME=50\n")!=std::string::npos;});
                auto commands=contents(folder/"commands");
                require(commands.find("ACTION=FLUSH\n")!=std::string::npos,"actual flush control line");
                require(commands.find("PROGRESS=12\n")!=std::string::npos,"CLI progress contract uses seconds");
                require(commands.find("DURATION=60\n")!=std::string::npos,"CLI duration contract uses seconds");
            } else if(test=="bounded") {
                unsigned accepted=1;
                while(raopcl_accept_frames(output)&&accepted<1000) {
                    if(raopcl_send_chunk(output,pcm.data(),352,&timestamp))++accepted;
                }
                require(accepted<300,"pre-start PCM must remain bounded below three seconds");
            } else require(test=="pcm"||test.rfind("txt_",0)==0,"unknown adapter test");
        }
        require(raopcl_disconnect(output),"bounded disconnect");
        require(!raopcl_is_connected(output),"disconnected state");
        raopcl_destroy(output);output=nullptr;cross_ssl_free();
        std::filesystem::remove_all(folder);
        std::cout<<"Adapter test passed; synthetic child only, no physical audio claim.\n";
        return 0;
    } catch(const std::exception& error) {
        if(output)raopcl_destroy(output);
        if(!folder.empty())std::filesystem::remove_all(folder);
        std::cerr<<error.what()<<'\n';return 1;
    }
}
