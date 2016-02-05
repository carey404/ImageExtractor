/* Lightspeed OnSite Photo Extractor
 * Written by: Michael Carey
 * 
 * Requires libcurl, libxml
 *
 * Extracts Photos from CURRENT PRODUCTS ONLY
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

using namespace std;

string buffer;

// Function for saving XML response to file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// XML Parser - Get the product photo URI's and save the product code and image id to file
static void get_product_uri(xmlNode * a_node, FILE *uri_file, FILE *list_file)
{
    xmlNode *cur_node = NULL;
    xmlChar *uri = NULL, *code = NULL, *id = NULL;
    const char* png = ".png";
    xmlChar* temp = xmlCharStrndup(png, sizeof(png));
    
    for (cur_node = a_node; cur_node; cur_node = cur_node->next){
        
        if ((!xmlStrcmp(cur_node->name, (const xmlChar *)"product_photo"))) {
            uri = xmlGetProp(cur_node, (const xmlChar *)"uri");
            id = xmlGetProp(cur_node, (const xmlChar *)"id");
            code = xmlNodeGetContent(cur_node->parent->parent->children);
            
            //Add .png to end of filename
            xmlStrcat(id, temp);
            
            //Write Product Code and Image Name to TXT file
            fprintf(list_file, "%s,%s\n", code, id);
            
            //Save a list of Image URI's so we can get each image
            fprintf(uri_file, "%s\n", uri);

            xmlFree(uri);
            xmlFree(code);
            xmlFree(id);
        }
        
        get_product_uri(cur_node->children, uri_file, list_file);
    }
}

// Function to send request and get list of products
int curl_call(const char *url, const char *filename, const char *username, const char *password, const char *uagent, const char *xid)
{
    CURL* curl;
    CURLcode res;
    struct curl_slist *headerlist = NULL;
    curl = curl_easy_init();
    long http_code;
    
    const char* ua = "User-Agent:";
    const char* xp = "X-PAPPID:";
    
    char * useragent = (char *) malloc(1 + strlen(ua)+ strlen(uagent));
    char * xpappid = (char *) malloc(1 + strlen(xp)+ strlen(xid));
    
    strcpy(useragent, ua);
    strcat(useragent, uagent);
    
    strcpy(xpappid, xp);
    strcat(xpappid, xid);
    
    LIBXML_TEST_VERSION
    
    cout << "Selecting products (be patient)... " << flush;
    
    // Setup our custom headers
    headerlist = curl_slist_append(headerlist, "Accept:");
    headerlist = curl_slist_append(headerlist, useragent);
    headerlist = curl_slist_append(headerlist, xpappid);
    headerlist = curl_slist_append(headerlist, "Accept-Encoding: xml");
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "/Users/Shared/cookie.txt");
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/Users/Shared/cookie.txt");
    
    // Write curl response to file
    FILE* file = fopen(filename, "w");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    
    // Enable for testing purposes
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    res = curl_easy_perform(curl);
    
    if(res != CURLE_OK){
        fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        fclose(file);
        return 1;
    }
    
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (http_code == 401) {
        cout << "Could not connect to server. Invalid Username or Password.\n";
        curl_easy_cleanup(curl);
        fclose(file);
        return 1;
    }
    
    curl_easy_cleanup(curl);
    fclose(file);
    
    return 0;
}

// Function to request to get the photo data and save local copy
void curl_photos(const char *url, const char *filename, const char *username, const char *password, const char *uagent, const char *xid)
{
    CURL* curl;
    CURLcode res;
    struct curl_slist *headerlist = NULL;
    curl = curl_easy_init();
    
    const char* ua = "User-Agent:";
    const char* xp = "X-PAPPID:";
    
    char * useragent = (char *) malloc(1 + strlen(ua)+ strlen(uagent));
    char * xpappid = (char *) malloc(1 + strlen(xp)+ strlen(xid));
    
    strcpy(useragent, ua);
    strcat(useragent, uagent);
    
    strcpy(xpappid, xp);
    strcat(xpappid, xid);
    
    LIBXML_TEST_VERSION
    
    // Setup our custom headers
    headerlist = curl_slist_append(headerlist, "Accept:");
    headerlist = curl_slist_append(headerlist, useragent);
    headerlist = curl_slist_append(headerlist, xpappid);
    headerlist = curl_slist_append(headerlist, "Accept-Encoding: xml");
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "/Users/Shared/cookie.txt");
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/Users/Shared/cookie.txt");
    
    // Write curl response to file
    FILE* file = fopen(filename, "w");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    
    // Enable for testing purposes
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    res = curl_easy_perform(curl);
    
    if(res != CURLE_OK){
        fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
    }
    
    curl_easy_cleanup(curl);
    fclose(file);
}

int logout(const char *url, const char *username, const char *password, const char *uagent, const char *xid)
{
    CURL* curl;
    CURLcode res;
    struct curl_slist *headerlist = NULL;
    curl = curl_easy_init();
    
    const char* ua = "User-Agent:";
    const char* xp = "X-PAPPID:";
    
    char * useragent = (char *) malloc(1 + strlen(ua)+ strlen(uagent));
    char * xpappid = (char *) malloc(1 + strlen(xp)+ strlen(xid));
    
    strcpy(useragent, ua);
    strcat(useragent, uagent);
    
    strcpy(xpappid, xp);
    strcat(xpappid, xid);
    
    LIBXML_TEST_VERSION
    
    // Setup our custom headers
    headerlist = curl_slist_append(headerlist, useragent);
    headerlist = curl_slist_append(headerlist, xpappid);
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "/Users/Shared/cookie.txt");
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/Users/Shared/cookie.txt");
    
    // Enable for testing purposes
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    
    res = curl_easy_perform(curl);
    
    if(res != CURLE_OK){
        fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return 1;
    }
    
    curl_easy_cleanup(curl);
    return 0;
}

// Show usage
static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " [-h] --server IP_ADDRESS --port PORT_NUMBER --user LIGHTSPEED_USER --password LIGHTSPEED_PASSWORD\n"
    << "Optional Arguments:\n"
    << "\t-h, --help\t\tShow this help message\n"
    << "\t-s, --server\t\tIP_ADDRESS\tIP address of the Lightspeed Server. Defaults to 'localhost' if not specified\n"
    << "\t-n, --port\t\tPORT_NUMBER\tPort Number of the Lightspeed Server. Defaults to '9630' if not specified\n\n"
    << "Required Arguments:\n"
    << "\t-u, --user\t\tLightspeed Username\n"
    << "\t-p, --password\t\tLightspeed Password\n"
    << "\t-a, --useragent\t\tUser-Agent\n"
    << "\t-x, --xpappid\t\tX-PAPPID\n"

    << std::endl;
}

int main(int argc, char* argv[])
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
    string str;
    string image;
    string filename;
    string i;
    string delim = "/";
    
    vector <std::string> sources;
    string server = "localhost";
    string port = "9630";
    string username, password, useragent, xpappid;
    string url = "https://";
    string products, uris, updatedimages;
    
    const char *url_c;
    
    const char* homeDir = getenv ("HOME");
    char path [256];
    
    int result, session;
    
    if (argc < 6) {
        show_usage(argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        } else if ((arg == "-s") || (arg == "--server")) {
            if (i + 1 < argc) {
                server = argv[i+1];
            }
        } else if ((arg == "-n") || (arg == "--port")) {
            if (i + 1 < argc) {
                port = argv[i+1];
            }
        } else if ((arg == "-u") || (arg == "--user")) {
            if (i + 1 < argc) {
                username = argv[i+1];
            } else {
                std::cerr << "--username option requires one argument." << std::endl;
                return 1;
            }
        } else if ((arg == "-p") || (arg == "--password")) {
            if (i + 1 < argc) {
                password = argv[i+1];
            } else {
                std::cerr << "--password option requires one argument." << std::endl;
                return 1;
            }
        }else if ((arg == "-a") || (arg == "--useragent")) {
            if (i + 1 < argc) {
                useragent = argv[i+1];
            } else {
                std::cerr << "--useragent option requires one argument." << std::endl;
                return 1;
            }
        }else if ((arg == "-x") || (arg == "--xpappid")) {
            if (i + 1 < argc) {
                xpappid = argv[i+1];
            } else {
                std::cerr << "--xpappid option requires one argument." << std::endl;
                return 1;
            }
        } else {
            sources.push_back(argv[i]);
        }
    }
    
    // Create folder on Desktop to store images
    sprintf (path, "%s/Desktop/%s", homeDir, "ExtractedImages/");
    mkdir(path, 0775);

    // Get list of products, save response in "products.xml"
    url = "https://" + server + ":" + port + "/api/products/";
    url_c = url.c_str();
    uris = path;
    products = path;
    updatedimages = path;
    products.append("products.xml");
    uris.append("uri.xml");
    updatedimages.append("updatedimages.txt");
    
    result = curl_call(url_c, products.c_str(), username.c_str(), password.c_str(), useragent.c_str(), xpappid.c_str());
    // If call failed, quit application
    if (result) return 0;
    
    cout << "Done\n";
    
    // Parse response and save the URI's to "uri.xml"
    doc = xmlReadFile(products.c_str(), NULL, 0);
    root_element = xmlDocGetRootElement(doc);
    
    FILE* url_file = fopen(uris.c_str(), "w");
    FILE* image_file = fopen(updatedimages.c_str(), "w");
    fprintf(image_file, "%s,%s\n", "Code", "File");

    cout << "Saving images to disk\n";
    get_product_uri(root_element, url_file, image_file);
    fclose(url_file);
    fclose(image_file);

    xmlFreeDoc(doc);
    xmlCleanupParser();
    
    // Open the file and pass each URI to curl_call
    ifstream file2(uris.c_str());
    
    // Loop through the file line by line
    while (getline(file2, str))
    {
        image = str.c_str();
        
        // Parse the image string to get the photo ID
        auto start = 0U;
        auto end = str.find(delim);
        while (end != std::string::npos)
        {
            i = image.substr(start, end - start);
            start = end + delim.length();
            end = image.find(delim, start);
        }
        
        image = image.append("image/");
        
        filename = path;
        filename = filename.append(i.c_str());
        filename = filename.append(".png");
        cout << "\r" << filename << flush;
        curl_photos(image.c_str(), filename.c_str(), username.c_str(), password.c_str(), useragent.c_str(), xpappid.c_str());
        
    }
    
    url = "https://" + server + ":" + port + "/api/sessions/current/logout/";
    session = logout(url.c_str(), username.c_str(), password.c_str(), useragent.c_str(), xpappid.c_str());
    
    if (!session) {
        cout << "\n\nLogging out of Lightspeed... Done\n";
    }
    
    // Delete unused files
    remove(uris.c_str());
    remove(products.c_str());
    remove("/Users/Shared/cookie.txt");

    return 0;
}