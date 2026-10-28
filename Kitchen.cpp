//g++ main.cpp -I"/đường/dẫn/tới/Crow-1.3.3/include" -lws2_32 -lmswsock -o server.exe
//g++ Kitchen.cpp -I"D:/CodeVault/Code Files/Crow-1.3.3/include" -lws2_32 -lmswsock -o server.exe
#include <iostream>
#include <cstdio>
#include <string>
#include "json.hpp"
#include "crow.h"

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

    CROW_ROUTE(app,"/download")
    .methods(crow::HTTPMethod::POST, crow::HTTPMethod::Options)
    ([](const crow::request& req)
    {
        if (req.method == crow::HTTPMethod::OPTIONS)
            {
                return make_response(204, "");
            }


        try 
        {
            auto package = json::parse(req.body);
            std::string videoURL = req.body;
            std::string ticket_of_order = req.url_params.get("quality");

            //cout to the terminal to verify that the video URL and ticket are correct.
            std::cout << "Progessing download video: " << videoURL << " with order : " << ticket_of_order << std::endl; 

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

            std::cout << "RAW: [" << raw_json << "]" << std::endl;
            json data = json::parse(raw_json); //parse json files in the bag
                //take the THUMBNAIL out of the bag (again...)
                std::string video_thumbnail = data["thumbnail"];



                //Get the Audio FormatID first
                std::string Audio_FormatID = "";
                for (auto& format : data["formats"]) {
                    if (format.contains("format_id") && format.contains("ext") && format.contains("vcodec")) {
                        if (format["ext"] == "m4a" && format["vcodec"] == "none" )  {
                            Audio_FormatID = format["format_id"];
                        }
                    }   
                }  
/*MP3 AUDIO*/   std::cout << "Audio Format ID : " << Audio_FormatID << std::endl;


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
/*VIDEO 480p*/  std::cout << "Video 480p Format ID : " << Video480p_FormatID << std::endl;

                //set up the download command based on the user's order.
                std::string download_command = "";
                        // -- Thumbnail --
                    if (ticket_of_order == "thumbnail") 
                    {
                        download_command = "yt-dlp.exe --skip-download --write-thumbnail -o \"Oven/%(id)s\" + videoURL";
                    }
                        // -- MP3 AUDIO --
                    else if (ticket_of_order == "mp3")
                    {
                        download_command = "yt-dlp.exe -x --audio-format mp3 -o \"Oven/%(id)s.mp3\" " + videoURL;
                    }
                        // -- MP4 480p  --
                    else if (ticket_of_order == "480p")
                    {
                        download_command = "yt-dlp.exe -f \"" + Audio_FormatID + "+" + Video480p_FormatID + "\" --merge-output-format mp4 -o \"Oven/%(id)s_480p.mp4\" " + videoURL;
                    }
                    else 
                    {
                        return make_response(400, "unknown option");
                    }
                    //tạm thời được
        //ex : yt-dlp -f "135+140" --merge-output-format mp4 -o "downloads/%(id)s_480p.mp4" https://www.youtube.com/watch?v=Oe8bDJAQx8Q
        //        std::string download_command = "yt-dlp.exe -f \"" + Audio_FormatID + "+" + Video480p_FormatID //  \" là dấu "
        //            + "\" --merge-output-format mp4 -o \"Oven/%(id)s_480p.mp4\" " + videoURL;
                //after got the ingredients, chef puts it inside the Oven!
            system(download_command.c_str());
            return make_response(200, "download successfully!");

        }
        catch (const std::exception& e)        
        {
            std::cout << "Loi xay ra o /download: " << e.what() << std::endl;
            return make_response(500, "Internal Server Error");                          
        }              
    });            

    app.port(18080).run();
}