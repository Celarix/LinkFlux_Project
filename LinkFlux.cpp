//g++ main.cpp -I"/Path/To/Crow-1.3.3/include" -lws2_32 -lmswsock -o server.exe
#include <iostream>
#include <cstdio>
#include <string>
#include "json.hpp"
#include "crow.h"
#include <thread>
#include <chrono>

using json = nlohmann::json;

    crow::response make_response (int status_code, const std::string& body_message)
        {
            crow::response res (status_code, body_message);
                res.set_header("Access-Control-Allow-Origin", "*");
                res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS, GET");
                res.set_header("Access-Control-Allow-Headers", "Content-Type");
            return res;
        }

int main() {
    
    system("pip install --upgrade yt-dlp > nul 2>&1");

    system("del /Q Oven\\*"); //Clean up before cook (\\ = \ in C++. Q = Quiet. no comfirmation needed)
    
    // Timer 1 : Cleaner
    std::thread([]() {

        while (true)
        {
            std::this_thread::sleep_for(std::chrono::hours(4));
            system("del /Q Oven\\*");
        }
    }).detach();
    
    // Timer 2 : Upgrade the Kitchen
    std::thread([]() {

        while (true) 
        {
            std::this_thread::sleep_for(std::chrono::hours(24));
            system("pip install --upgrade yt-dlp > nul 2>&1");
        }
    }).detach();
    
    crow::SimpleApp app;
        // ----- API 1: used to return the title and thumbnail to the frontend. ensure the video that the user wants to download
    CROW_ROUTE(app, "/getVideo")
    .methods(crow::HTTPMethod::POST)
    ([](const crow::request& req)
    {
        std::string video_URL = req.body;   
        std::string command = "yt-dlp.exe --dump-json " + video_URL;

        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            return crow::response(500, "We got some problem, please try again later");
        }

        std::string raw_json = ""; //A empty bag of json files
        char buffer[128];
        while(fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            raw_json += buffer; //put them in da bag dawg!
        }
        pclose(pipe);

            try 
            {
                json data = json::parse(raw_json); //parse json files in the bag
                //take the TITLE out of the bag
                std::string video_title = data["title"];
                //take the THUMBNAIL out of the bag
                std::string video_thumbnail = data["thumbnail"];
            
                //now pack it and send it to the user

            json product;
            product["title"] = video_title;
            product["thumbnail"] = video_thumbnail;

            std::string Package = product.dump();

                auto res = crow::response(Package);
                res.set_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            catch (const std::exception& e)
            {
                return crow::response(500, "JSON Parse error!");
            }
    });

/*------------------------------------------------------------------------------------------*/

        // ----- API 2: Used to download videos to the user's device after selecting "Order"
    CROW_ROUTE(app, "/download")
    .methods(crow::HTTPMethod::POST, crow::HTTPMethod::Options)
    ([](const crow::request& req)
    {
        if (req.method == crow::HTTPMethod::OPTIONS)
            {
                return make_response(204, "");
            }

        try 
        {
            //auto package = json::parse(req.body);
            std::string videoURL = req.body;
            std::string ticket_of_order = req.url_params.get("quality");

            //cout to the terminal to verify that the video URL and ticket are correct.
            std::cout << "|--- Progessing download video: " << videoURL << " with order : " << ticket_of_order << " ---|" << std::endl; 

            std::string command = "yt-dlp.exe --dump-json " + videoURL;

            FILE* pipe = popen(command.c_str(), "r");
            if (!pipe) {        
                return make_response(500, "We got some problem, please try again later");
            }

            std::string raw_json = ""; //A empty bag of json files
            char buffer[128];
            while(fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                raw_json += buffer; //put them in my 2nd bag dawg
            }
            pclose(pipe);

            json data = json::parse(raw_json); //parse json files in the bag

                //Get the Audio FormatID first
                std::string Audio_FormatID = "";
                for (auto& format : data["formats"]) {
                    if (format.contains("format_id") && format.contains("ext") && format.contains("vcodec")) {
                        if (format["ext"] == "m4a" && format["vcodec"] == "none" )  {
                            Audio_FormatID = format["format_id"];
                        }
                    }   
                }  
/*MP3 AUDIO*/   std::cout << "Audio       Format ID : " << Audio_FormatID << std::endl;

//"acodec": "none"
                //Next is 480p Video FormatID
                std::string Video480p_FormatID = "";
                for (auto& format : data["formats"]) {
                    if (format.contains("format_id") && format.contains("ext") && format.contains("format_note") && format.contains("acodec")) {
                        if (format["ext"] == "mp4" && format["format_note"] == "480p" && format["acodec"] == "none" && Video480p_FormatID.empty() )  {
                            Video480p_FormatID = format["format_id"];
                        }
                    }   
                }  
/*VIDEO 480p*/  std::cout << "Video  480p Format ID : " << Video480p_FormatID << std::endl;

                //  720p Video FormatID
                std::string Video720p_FormatID = "";
                for (auto& format : data["formats"]) {
                    if (format.contains("format_id") && format.contains("ext") && format.contains("height") && format.contains("acodec") && format.contains("format_note")) {
                        if (format["ext"] == "mp4" && format["acodec"] == "none" && (format["height"] == 720 || format["height"] == 1280) && (format["format_note"] == "720p" || format["format_note"] == "720p60" )
                        && Video720p_FormatID.empty()) {

                            Video720p_FormatID = format["format_id"];
                        }
                    }
                }
/*VIDEO 720p*/  std::cout << "Video  720p Format ID : " << Video720p_FormatID << std::endl;

                //  1080p Video FormatID
                std::string Video1080p_FormatID = "";
                for (auto& format : data["formats"]) {
                    if (format.contains("format_id") && format.contains("ext") && format.contains("height") && format.contains("acodec") && format.contains("format_note")) {
                        if (format["ext"] == "mp4" && format["acodec"] == "none" && (format["height"] == 1080 || format["height"] == 1920) && (format["format_note"] == "1080p" || format["format_note"] == "1080p60" )
                        && Video1080p_FormatID.empty()) {

                            Video1080p_FormatID = format["format_id"];
                        }
                    }
                }
/*VIDEO 1080p*/ std::cout << "Video 1080p Format ID : " << Video1080p_FormatID << std::endl;            

                //set up the download command based on the user's order.
                std::string video_id = data["id"];    
                std::string file_name = "";
                std::string download_command = "";

                        // -- Thumbnail --
                    if (ticket_of_order == "thumbnail") 
                    {
                        download_command = "yt-dlp.exe --skip-download --write-thumbnail --convert-thumbnails jpg -o \"Oven/%(id)s\" " + videoURL;
                        file_name = video_id + ".jpg";
                    }
                        // -- MP3 AUDIO --
                    else if (ticket_of_order == "mp3")
                    {
                        download_command = "yt-dlp.exe -x --audio-format mp3 -o \"Oven/%(id)s.mp3\" " + videoURL;
                        file_name = video_id + ".mp3";
                    }
                        // -- MP4 480p  --
                    else if (ticket_of_order == "480p")
                    {
                        download_command = "yt-dlp.exe -f \"" + Audio_FormatID + "+" + Video480p_FormatID + "\" --merge-output-format mp4 -o \"Oven/%(id)s_480p.mp4\" " + videoURL;
                        file_name = video_id + "_480p.mp4";
                    }
                        // -- MP4 720p --
                    else if (ticket_of_order == "720p")
                    {
                        download_command = "yt-dlp.exe -f \"" + Audio_FormatID + "+" + Video720p_FormatID + "\" --merge-output-format mp4 -o \"Oven/%(id)s_720p.mp4\" " + videoURL;
                        file_name = video_id + "_720p.mp4";
                    }                    
                        // -- MP4 1080p --
                    else if (ticket_of_order == "1080p")
                    {
                        download_command = "yt-dlp.exe -f \"" + Audio_FormatID + "+" + Video1080p_FormatID + "\" --merge-output-format mp4 -o \"Oven/%(id)s_1080p.mp4\" " + videoURL;
                        file_name = video_id + "_1080p.mp4";
                    }          
                        // -- uhh... like... something else... idk? --
                    else 
                    {
                        return make_response(400, "Unknown Option");
                    }
                
            // --- after got the ingredients, chef puts it in the Oven! ---
            system(download_command.c_str());       // cooking
            return make_response(200, file_name);   // food's ready !  take it out.
        }
        catch (const std::exception& e)        
        {
            std::cout << "Error in downloading process" << e.what() << std::endl;
            return make_response(500, "Internal Server Error");                          
        }        
    });            

/*------------------------------------------------------------------------------------------*/

    CROW_ROUTE(app, "/file/<string>")
    ([](const crow::request& req, std::string name)
    {
        // Security : Prevent users from exiting the Oven directory
        if (name.find("/") != std::string::npos || name.find("\\") != std::string::npos || name.find("..") != std::string::npos)
            {   
                return make_response(400, "Nice try.");
            }
            
        // Create a response object and call the crow function to point to a file in the Oven directory
        crow::response res;
        res.set_static_file_info("Oven/" + name);
        
        //add 1 header to tell the browser "This file that needs to be saved"
        res.set_header("Content-Disposition", "attachment; filename=\"" + name + "\"");
        
        //return res object containing the file for the browser to Download !
        return res; 
    });

    CROW_ROUTE(app, "/")([](const crow::request&) {
        crow::response res;
        res.set_static_file_info("LinkFlux.html");
        return res;
    });
    CROW_ROUTE(app, "/LinkFlux.css")([](const crow::request&) {
        crow::response res;
        res.set_static_file_info("LinkFlux.css");
        return res;
    });   
    CROW_ROUTE(app, "/LinkFlux.js")([](const crow::request&) {
        crow::response res;
        res.set_static_file_info("LinkFlux.js");
        return res;
    });

    CROW_ROUTE(app, "/assets/<path>")([](const crow::request&, std::string PathAssets){
        crow::response res;
        res.set_static_file_info("assets/" + PathAssets);
        return res;
    });
app.port(18080).run();
}