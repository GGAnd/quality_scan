// CMakequality_scan.cpp: определяет точку входа для приложения.
//

#include "include\facerec\import.h"
#include "include\facerec\libfacerec.h"
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>


int main(int argc, char** argv)
{
    setlocale(LC_ALL, "Russian");
    try
    {

        pbio::FacerecService::Ptr service;
#ifdef _WIN32
        service = pbio::FacerecService::createService("../bin/facerec.dll", "../conf/facerec", "../license");
#else
        service = pbio::FacerecService::createService("../lib/libfacerec.so", "../conf/facerec/");
#endif

        auto configCtx = service->createContext();
        configCtx["unit_type"] = "QUALITY_ASSESSMENT_ESTIMATOR";
        configCtx["config_name"] = "quality_assessment.xml";

        auto detectorConfigCtx = service->createContext();
        detectorConfigCtx["unit_type"] = "FACE_DETECTOR";
        detectorConfigCtx["modification"] = "ssyv";

        auto fitterConfigCtx = service->createContext();
        fitterConfigCtx["unit_type"] = "FACE_FITTER";
        pbio::ProcessingBlock blockQuality = service->createProcessingBlock(configCtx);
        pbio::ProcessingBlock faceDetector = service->createProcessingBlock(detectorConfigCtx);
        pbio::ProcessingBlock faceFitter = service->createProcessingBlock(fitterConfigCtx);

        std::string  directory;
        int num_processed;
        std::cout << "Specify the path to the directory with pictures" << std::endl;
        std::cin >> directory;
        std::list<std::string> fails;
        std::filesystem::path dir_path(directory);
        for (const auto& entry : std::filesystem::directory_iterator(dir_path))
        {
            if (std::filesystem::is_directory(entry.path()))
                for (const auto& entry : std::filesystem::directory_iterator(entry))
                {
                    // Проверяем, что файл является изображением
                    if (entry.path().extension() == ".png" || entry.path().extension() == ".jpg" || entry.path().extension() == ".jpeg" || entry.path().extension() == ".bmp")
                        fails.push_back(entry.path().string());
                }
            if (entry.path().extension() == ".png" || entry.path().extension() == ".jpg" || entry.path().extension() == ".jpeg" || entry.path().extension() == ".bmp")
                fails.push_back(entry.path().string());
        }
        num_processed = fails.size();
        std::cout <<"Number of pictures in the directory: " << num_processed << std::endl;
        std::string answer;
        label:
        std::cout << "Set the number of images to process? (y/n): ";
        std::cin >> answer;
        if (answer == "y" || answer == "Y") {
            std::cout << "Number of pictures:" << std::endl;
            std::cin >> num_processed;
        }
        else if (answer == "n" || answer == "N") {
            num_processed = rand() % (num_processed - 10 + 1) + 10;
            std::cout << "Number of pictures" << num_processed << std::endl;
        }
        else {
            std::cout << "Invalid input" << std::endl;
            goto label;
        }
       
        std::ofstream outFile("result.csv", std::ios::out | std::ios::binary);
        // Проверяем, успешно ли открыт файл
        if (!outFile.is_open()) {
            std::cerr << "Failed to open file for writing!" << std::endl;
            return 1;
        }
        else
            outFile << "filename;confidence;total_score;is_sharp;sharpness_score;is_evenly_illuminated;illumination_score;";
        outFile << "no_flare;is_left_eye_opened;is_right_eye_opened;is_rotation_acceptable;not_masked;is_not_noisy;";
        outFile << "is_neutral_emotion;is_eyes_distance_acceptable;eyes_distance;has_watermark;dynamic_range_score;";
        outFile << "dynamic_range_score;is_dynamic_range_acceptable;" << std::endl;


        for (std::string fail : fails)
        {
           
            std::filesystem::path dir_path(fail);
            std::string inputImagePath = fail;
            std::ifstream imageFile(inputImagePath, std::ios::binary);                        
            std::istreambuf_iterator<char> start(imageFile);                      
            std::vector<char> imageData(start, std::istreambuf_iterator<char>());                       
            pbio::Context ioData = service->createContextFromEncodedImage(imageData);
                        
            outFile << dir_path.filename().string() << ";";
                       
            faceDetector(ioData);                       
            faceFitter(ioData);

            outFile << ioData["objects"][0]["confidence"].getDouble() << ";";
                        
            blockQuality(ioData);
                        
            outFile << ioData["objects"][0]["quality"]["total_score"].getDouble() << ";";                       
            outFile << ioData["objects"][0]["quality"]["is_sharp"].getBool() << ";";                        
            outFile << ioData["objects"][0]["quality"]["sharpness_score"].getDouble() << ";";                       
            outFile << ioData["objects"][0]["quality"]["is_evenly_illuminated"].getBool() << ";";                       
            outFile << ioData["objects"][0]["quality"]["illumination_score"].getDouble() << ";";                        
            outFile << ioData["objects"][0]["quality"]["no_flare"].getBool() << ";";                       
            outFile << ioData["objects"][0]["quality"]["is_left_eye_opened"].getBool() << ";";                      
            outFile << ioData["objects"][0]["quality"]["is_right_eye_opened"].getBool() << ";";                        
            outFile << ioData["objects"][0]["quality"]["is_rotation_acceptable"].getBool() << ";";                        
            outFile << ioData["objects"][0]["quality"]["not_masked"].getBool() << ";";                        
            outFile << ioData["objects"][0]["quality"]["is_neutral_emotion"].getBool() << ";";                       
            outFile << ioData["objects"][0]["quality"]["is_eyes_distance_acceptable"].getBool() << ";";                                 
            outFile << ioData["objects"][0]["quality"]["eyes_distance"].getLong() << ";";                        
            outFile << ioData["objects"][0]["quality"]["is_margins_acceptable"].getBool() << ";";                        
            outFile << ioData["objects"][0]["quality"]["is_not_noisy"].getBool() << ";";                       
            outFile << ioData["objects"][0]["quality"]["has_watermark"].getBool() << ";";                        
            outFile << ioData["objects"][0]["quality"]["dynamic_range_score"].getDouble() << ";";
            outFile << ioData["objects"][0]["quality"]["is_dynamic_range_acceptable"].getBool() << ";" << std::endl; 
            --num_processed;
            if (num_processed == 0)
                break;
            std::cout << "Files left : " << num_processed << std::endl;
        }

        outFile.close();
        std::cout << "Data recorded result.csv"  << std::endl;
    }

    catch (const pbio::Error& e)
    {
        std::cerr << "facerec exception catched: '" << e.what() << "' code: " << std::hex << e.code() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "exception catched: '" << e.what() << "'" << std::endl;
    }


}


