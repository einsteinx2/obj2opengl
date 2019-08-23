//
//  main.cpp
//  blender2opengles
//
//  Created by RRC on 9/9/13.
//  Copyright (c) 2013 Ricardo Rendon Cepeda. All rights reserved.
//

// C++ Standard Library
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// Model Structure
typedef struct Model
{
    int vertices;
    int positions;
    int texels;
    int normals;
    int faces;
}
Model;

Model getOBJinfo(string fp)
{
    Model model = {0};
    
    // Open OBJ file
    ifstream inOBJ;
    inOBJ.open(fp);
    if(!inOBJ.good())
    {
        cout << "ERROR OPENING OBJ FILE" << endl;
        exit(1);
    }
    
    // Read OBJ file
    while(!inOBJ.eof())
    {
        string line;
        getline(inOBJ, line);
        string type = line.substr(0,2);
        
        if(type.compare("v ") == 0)
            model.positions++;
        else if(type.compare("vt") == 0)
            model.texels++;
        else if(type.compare("vn") == 0)
            model.normals++;
        else if(type.compare("f ") == 0)
            model.faces++;
    }
    
    model.vertices = model.faces*3;
    
    // Close OBJ file
    inOBJ.close();
    
    return model;
}

void extractOBJdata(string fp, float positions[][3], float texels[][2], float normals[][3], int faces[][9])
{
    // Counters
    int p = 0;
    int t = 0;
    int n = 0;
    int f = 0;
    
    // Open OBJ file
    ifstream inOBJ;
    inOBJ.open(fp);
    if(!inOBJ.good())
    {
        cout << "ERROR OPENING OBJ FILE" << endl;
        exit(1);
    }
    
    // Read OBJ file
    while(!inOBJ.eof())
    {
        string line;
        getline(inOBJ, line);
        string type = line.substr(0,2);
        
        // Positions
        if(type.compare("v ") == 0)
        {
            // Copy line for parsing
            char* l = new char[line.size()+1];
            memcpy(l, line.c_str(), line.size()+1);
            
            // Extract tokens
            strtok(l, " ");
            for(int i=0; i<3; i++)
                positions[p][i] = atof(strtok(NULL, " "));
            
            // Wrap up
            delete[] l;
            p++;
        }
        
        // Texels
        else if(type.compare("vt") == 0)
        {
            char* l = new char[line.size()+1];
            memcpy(l, line.c_str(), line.size()+1);
            
            strtok(l, " ");
            for(int i=0; i<2; i++)
                texels[t][i] = atof(strtok(NULL, " "));
            
            delete[] l;
            t++;
        }
        
        // Normals
        else if(type.compare("vn") == 0)
        {
            char* l = new char[line.size()+1];
            memcpy(l, line.c_str(), line.size()+1);
            
            strtok(l, " ");
            for(int i=0; i<3; i++)
                normals[n][i] = atof(strtok(NULL, " "));
            
            delete[] l;
            n++;
        }
        
        // Faces
        else if(type.compare("f ") == 0)
        {
            char* l = new char[line.size()+1];
            memcpy(l, line.c_str(), line.size()+1);
            
            strtok(l, " ");
            for(int i=0; i<9; i++)
                faces[f][i] = atof(strtok(NULL, " /"));
            
            delete[] l;
            f++;
        }
    }
    
    // Close OBJ file
    inOBJ.close();
}

string structType(string name) {
    string structType = name;
    structType[0] = toupper(structType[0]);
    structType = structType + "Mesh";
    return structType;
}

string structName(string name) {
    string structName = structType(name);
    structName[0] = tolower(structName[0]);
    return structName;
}

void writeH(string fp, string name, Model model)
{
    // Create H file
    ofstream outH;
    outH.open(fp);
    if(!outH.good())
    {
        cout << "ERROR CREATING H FILE" << endl;
        exit(1);
    }
    
    // Write to H file
    outH << "// This is a .h file for the model: " << name << ".obj" << endl;
    outH << endl;
    
    // Write statistics
    outH << "// Positions: " << model.positions << endl;
    outH << "// Texels: " << model.texels << endl;
    outH << "// Normals: " << model.normals << endl;
    outH << "// Faces: " << model.faces << endl;
    outH << "// Vertices: " << model.vertices << endl;
    outH << endl;
    outH << "#pragma once" << endl;
    outH << endl;
    
    // Write struct declaration
    outH << "struct " << structType(name) << " {" << endl;
    outH << "    const int numVertices;" << endl;
    outH << "    const float positions[" << model.vertices << "][3];" << endl;
    outH << "    const float texels[" << model.vertices << "][2];" << endl;
    outH << "    const float normals[" << model.vertices << "][3];" << endl;
    outH << "};" << endl;
    outH << endl;
    outH << "extern struct " << structType(name) << " " << structName(name) << ";" << endl;
    
    // Close H file
    outH.close();
}

void writeCvertices(string fp, string name, Model model)
{
    // Create C file
    ofstream outC;
    outC.open(fp);
    if(!outC.good())
    {
        cout << "ERROR CREATING C FILE" << endl;
        exit(1);
    }
    
    // Write to C file
    outC << "// This is a .c file for the model: " << name << endl;
    outC << endl;
    
    // Header
    outC << "#include " << "\"" << name << "Mesh.h" << "\"" << endl;
    outC << endl;
    
    // Vertices
    outC << "struct " << structType(name) << " " << structName(name) << " = {" << endl;
    outC << "    .numVertices = " << model.vertices << "," << endl;
    
    // Close C file
    outC.close();
}

void writeCpositions(string fp, string name, Model model, int faces[][9], float positions[][3])
{
    // Append C file
    ofstream outC;
    outC.open(fp, ios::app);
    
    // Positions
    outC << "    .positions = {" << endl;
    for(int i = 0; i < model.faces; i++)
    {
        int vA = faces[i][0] - 1;
        int vB = faces[i][3] - 1;
        int vC = faces[i][6] - 1;
        
        outC << "        {" << positions[vA][0] << ", " << positions[vA][1] << ", " << positions[vA][2] << "}, " << endl;
        outC << "        {" << positions[vB][0] << ", " << positions[vB][1] << ", " << positions[vB][2] << "}, " << endl;
        outC << "        {" << positions[vC][0] << ", " << positions[vC][1] << ", " << positions[vC][2] << "}, " << endl;
    }
    outC << "    }," << endl;
    
    // Close C file
    outC.close();
}

void writeCtexels(string fp, string name, Model model, int faces[][9], float texels[][2])
{
    // Append C file
    ofstream outC;
    outC.open(fp, ios::app);
    
    // Texels
    outC << "    .texels = {" << endl;
    for(int i = 0; i < model.faces; i++)
    {
        int vtA = faces[i][1] - 1;
        int vtB = faces[i][4] - 1;
        int vtC = faces[i][7] - 1;
        
        outC << "        {" << texels[vtA][0] << ", " << texels[vtA][1] << "}, " << endl;
        outC << "        {" << texels[vtB][0] << ", " << texels[vtB][1] << "}, " << endl;
        outC << "        {" << texels[vtC][0] << ", " << texels[vtC][1] << "}, " << endl;
    }
    outC << "    }," << endl;
    
    // Close C file
    outC.close();
}

void writeCnormals(string fp, string name, Model model, int faces[][9], float normals[][3])
{
    // Append C file
    ofstream outC;
    outC.open(fp, ios::app);
    
    // Normals
    outC << "    .normals = {" << endl;
    for(int i = 0; i < model.faces; i++)
    {
        int vnA = faces[i][2] - 1;
        int vnB = faces[i][5] - 1;
        int vnC = faces[i][8] - 1;
        
        outC << "        {" << normals[vnA][0] << ", " << normals[vnA][1] << ", " << normals[vnA][2] << "}, " << endl;
        outC << "        {" << normals[vnB][0] << ", " << normals[vnB][1] << ", " << normals[vnB][2] << "}, " << endl;
        outC << "        {" << normals[vnC][0] << ", " << normals[vnC][1] << ", " << normals[vnC][2] << "}, " << endl;
    }
    outC << "    }" << endl;
    outC << "};" << endl;
    
    // Close C file
    outC.close();
}

void printUsage(const char* argv[]) {
    printf("Usage: %s model_name.obj\n", argv[0]);
}

int main(int argc, const char* argv[])
{
    // Check arguments
    if (argc != 2) {
        printUsage(argv);
        return -1;
    }

    string nameObj = argv[1];
    size_t nameLength = nameObj.length();
    if (nameLength < 5) {
        printUsage(argv);
        return -1;
    }

    string name = nameObj.substr(0, nameLength - 4);
    string nameExt = nameObj.substr(nameLength - 4, nameLength);
    if (nameExt != ".obj" && nameExt != ".OBJ") {
        printUsage(argv);
        return -1;
    }
    
    // Output Files
    string filepathH = name + "Mesh.h";
    string filepathC = name + "Mesh.c";
    
    // Model Info
    Model model = getOBJinfo(nameObj);
    
    // Model Data
    float positions[model.positions][3];    // XYZ
    float texels[model.texels][2];          // UV
    float normals[model.normals][3];        // XYZ
    int faces[model.faces][9];              // PTN PTN PTN
    
    extractOBJdata(nameObj, positions, texels, normals, faces);
    
    // Write H file
    writeH(filepathH, name, model);
    
    // Write C file
    writeCvertices(filepathC, name, model);
    writeCpositions(filepathC, name, model, faces, positions);
    writeCtexels(filepathC, name, model, faces, texels);
    writeCnormals(filepathC, name, model, faces, normals);
    
    return 0;
}

