// XmlTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "tinyxml2/tinyxml2.h"
#pragma comment(lib, "tinyxml2/tinyxml2.lib")

int main()
{
    // Create xml
    const char* declaration = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    tinyxml2::XMLDocument doc;
    doc.Parse(declaration);
    tinyxml2::XMLElement* root = doc.NewElement("UserInfo");
    doc.InsertEndChild(root);

    tinyxml2::XMLElement* xmlUser = doc.NewElement("User");
    xmlUser->SetAttribute("attribute", "user0");
    root->InsertEndChild(xmlUser);
    tinyxml2::XMLElement* xmlCode = doc.NewElement("Code");
    xmlCode->SetText("Code111");
    xmlUser->InsertEndChild(xmlCode);
    tinyxml2::XMLElement* xmlPwd = doc.NewElement("Password");
    xmlPwd->SetText("Pwd111");
    xmlUser->InsertEndChild(xmlPwd);

    xmlUser = doc.NewElement("User");
    xmlUser->SetAttribute("attribute", "user1");
    doc.RootElement()->InsertEndChild(xmlUser);
    xmlCode = doc.NewElement("Code");
    xmlCode->SetText("Code222");
    xmlUser->InsertEndChild(xmlCode);
    xmlPwd = doc.NewElement("Password");
    xmlPwd->SetText("Pwd222");
    xmlUser->InsertEndChild(xmlPwd);
    doc.SaveFile("test.xml");

    // Read xml
    if (0 == doc.LoadFile("test.xml")) {
        if (doc.RootElement()) {
            tinyxml2::XMLElement* userEle = doc.RootElement()->FirstChildElement();
            bool bFound = false;
            while (userEle) {
                tinyxml2::XMLElement* infoCode = userEle->FirstChildElement("Code");
                tinyxml2::XMLElement* infoPwd = userEle->FirstChildElement("Password");
                if(infoCode)
                    std::cout << infoCode->GetText() << std::endl;
                if(infoPwd)
                    std::cout << infoPwd->GetText() << std::endl;
                userEle = userEle->NextSiblingElement();
            }
        }
    }

    // Delete node
    if (0 == doc.LoadFile("test.xml")) {
        tinyxml2::XMLElement* root = doc.RootElement();
        tinyxml2::XMLElement* userEle = root->FirstChildElement("User");
        while (userEle){
            if (0 == strcmp(userEle->Attribute("attribute"), "user1")) {
                userEle->DeleteAttribute("attribute");
                userEle->DeleteChildren();
                break;
            }
            userEle = userEle->NextSiblingElement();
        }
    }
    doc.SaveFile("test.xml");

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
