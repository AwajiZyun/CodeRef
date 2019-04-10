// JsonTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// http://open-source-parsers.github.io/jsoncpp-docs/doxygen/annotated.html
// json_reader.cpp
// json_value.cpp
// json_writer.cpp
// jsonTest.cpp

#include "pch.h"
#include <iostream>
#include "lib_json/include/json.h"
#include <string>
using std::cout;
using std::endl;

int main()
{
    const char *jsonStr = "{" \
        "\"success\" : true," \
        "\"cnts\" : 10," \
        "\"msgs\" : \"OK\"," \
        "\"arrary\" : [" \
            "{\"code\" : \"12345\", \"name\" : \"Tomas\"}," \
            "{\"code\" : \"23456\", \"name\" : \"Jerry\"}" \
        "]" \
        "}";

    // Parse
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    Json::Value root;
    JSONCPP_STRING errStr;
    if (reader->parse(jsonStr, jsonStr + strlen(jsonStr), &root, &errStr)) {
        // access boolean value
        if (!root["success"].isNull()) {
            bool bSuccess = root["success"].asBool();
            cout << "success:" << bSuccess << endl;
        }
        // access integer value
        if (!root["cnts"].isNull()) {
            int cnts = root["cnts"].asInt();
            cout << "cnts:" << cnts << endl;
        }
        // access string
        if (!root["msgs"].isNull()) {
            JSONCPP_STRING msgs = root["msgs"].asString();
            cout << "msgs:" << msgs << endl;
        }
        // access arrary
        if (!root["arrary"].isNull()) {
            for (unsigned int idx = 0; idx < root["arrary"].size(); ++idx) {
                // access object
                Json::Value data = root["arrary"][idx];
                if (!data["code"].isNull()) {
                    JSONCPP_STRING code = data["code"].asString();
                    cout << "code:" << code << endl;
                }
                if (!data["name"].isNull()) {
                    JSONCPP_STRING name = data["name"].asString();
                    cout << "name:" << name << endl;
                }
            }
        }
    }
    delete reader;
    cout << "********************" << endl;

    // Build
    Json::Value array;
    {
        Json::Value data;
        data["code"] = "1234";
        data["name"] = "Tomas";
        array.append(data); // object
        data["code"] = "2345";
        data["name"] = "Jerry";
        array.append(data);
    }

    Json::Value root2;
    root2["success"] = false;   // bool
    root2["cnts"] = 0;          // integer
    root2["msgs"] = "NG";       // string
    root2["array"] = array;     // arrary
    cout << root2.toStyledString() << endl;

    system("pause");

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
