#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <future>
#include <curl/curl.h>

#define SKIPLICENSE false

namespace imagepro
{
    class License
    {
    public:
        std::string getSiteEndpoint()
        {
            setupLicenseProperties();

            return property_site + "?id=" + getUniqueId();
        }

        std::string getSiteSupportEndpoint()
        {
			return property_site + "/support/support" + "?id=" + getUniqueId();;
        }

    private:
        License() = default;

        License(const License&) = delete;

        License& operator=(const License&) = delete;

        static size_t write(void* contents, size_t size, size_t nmemb, void* userp)
        {
            ((std::string*)userp)->append((char*)contents, size * nmemb);

            return size * nmemb;
        }

        bool readProperties(const std::string& filename, std::string& site, std::string& license)
        {
            std::ifstream file(filename);
            if (!file.is_open())
            {
                std::cerr << "Failed to open " << filename << std::endl;
                return false;
            }

            std::string line;
            while (std::getline(file, line))
            {
                size_t pos = line.find('=');
                if (pos == std::string::npos)
                {
                    continue;
                }

                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);

                if (key == "site")
                {
                    site = value;
                }
                else if (key == "license")
                {
                    license = value;
                }
            }

            file.close();
            return true;
        }

        bool check(const char* uniqueId, const char* endpoint, const char* payload, uintptr_t abxPtr)
        {
            thisabxPtr = abxPtr;
            CURL* curl;
            CURLcode res;
            std::string readBuffer;

            curl_global_init(CURL_GLOBAL_DEFAULT);
            curl = curl_easy_init();

            if (curl)
            {
                struct curl_slist* headers = NULL;
                headers = curl_slist_append(headers, "Content-Type: text/plain");
                headers = curl_slist_append(headers, "pri: consult");
                std::string entryHeader = std::string("IS_LICENSE_VALID: ") + uniqueId;
                headers = curl_slist_append(headers, entryHeader.c_str());

                curl_easy_setopt(curl, CURLOPT_URL, endpoint);
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

                res = curl_easy_perform(curl);

                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
            }

            curl_global_cleanup();

            if (!readBuffer.empty())
            {
                if (readBuffer == "YES")
                {
                    (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5))->set(1337);
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        int64_t CurrentTimeMillis()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }

        std::string getUniqueId()
        {
            DWORD volumeSerialNumber = 0;
            GetVolumeInformationA(
                "C:\\",
                nullptr,
                0,
                &volumeSerialNumber,
                nullptr,
                nullptr,
                nullptr,
                0
            );

            char buffer[32];
            sprintf_s(buffer, "%08X", volumeSerialNumber);
            return std::string(buffer);
        }

        void setupLicenseProperties()
        {
            if (readProperties("properties.txt", property_site, property_license))
            {
                //std::cout << "Site: " << property_site << std::endl;
                //std::cout << "License: " << property_license << std::endl;
            }
        }

        std::atomic<bool> licenseChecked = false;

        int64_t lastTimeChecked;

        std::string property_site;

        std::string property_license;

        uintptr_t thisabxPtr;

    public:
        static License& getInstance()
        {
            static License instance;
            return instance;
        }

        bool isLicenseChecked()
        {
            return licenseChecked;
        }

        void setLicenseChecked(bool licenseChecked)
        {
            this->licenseChecked = licenseChecked;
        }

        void checkLicense(uintptr_t abxPtr, bool force = true)
        {
            #ifdef BYPASSLICENSE
                (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5))->set(1337);
                return;
            #endif

            int64_t time = CurrentTimeMillis();

            if (force || time > lastTimeChecked + 1000 * 60)
            {
                setupLicenseProperties();

                if (SKIPLICENSE)
                {
                    (reinterpret_cast<Int*>(abxPtr ^ 0xA5A5A5A5))->set(1337);
                    licenseChecked = true;
                    return;
                }

                lastTimeChecked = time;

                std::string uniqueIdStr = getUniqueId();
                const char* uniqueId = uniqueIdStr.c_str();
                std::string endPointStr = property_license;
                const char* endpoint = endPointStr.c_str();
                const char* payload = "";

                std::thread([=]() {
                    std::string uniqueIdStr = getUniqueId();
                    std::string endpointStr = property_license;

                    check(
                        uniqueIdStr.c_str(),
                        endpointStr.c_str(),
                        "",
                        abxPtr
                    );

                    licenseChecked = true;
                    }).detach();
            }
        }
    };
}