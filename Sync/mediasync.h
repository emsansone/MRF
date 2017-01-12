#ifndef MEDIASYNC_H
#define MEDIASYNC_H

#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <Core>
#include <QR>
#include <string.h>

#define STATESFILE      "./Offsets.txt"
#define REFINESTATESFILE      "./RefineOffsets.txt"
#define PARAMETERSFILE  "./Parameters.txt"
#define PERFORMANCEFILE "./MediaSync_Results.txt"


#define TIMEREFERENCE "2001:01:01 01:00:00"

#define EARTHMEANRADIUS   6371
#define PI                3.14159265
#define GPSOVERVALUE      600

class MediaSync
{
public:
    std::string ReferenceDir;
    std::string CompareDir;
    std::string ReferenceMetadataFile;
    std::string CompareMetadataFile;
    struct Data{
        int index;
        std::string FileName;
        float GPSLatitude;
        float GPSLongitude;
        std::string DateTimeOriginal;
        int UTCDateTimeOriginal;
        int Offset;
        int ReferencedDateTimeOriginal;
        int State;
        int Class; //to discriminate photos belonging to ReferenceGallery and photos belonging to CompareGallery
    };
    std::vector<Data> ReferenceGallery;
    std::vector<Data> CompareGallery;
    std::vector<Data> SynchronizedGallery;
    struct Offset{
        //Steps starting from zero (measured in secs)
        int offset;
        //Associations with reference photos
        std::vector<int> StatesCombination;
        float gi;
        float hacca;

        //Temporarily functions
        std::vector<float> g;
        std::vector<float> h;
    };
    std::vector<Offset> ComparePossibleStates;
    time_t DateTimeToSec(std::string DateTime1, std::string DateTime2);
    struct Param{
        float gamma;
        float delta;
    };
    Param CurrParameters, PrevParameters;
    Param CoarseLearn; //used as initial value in the learning phase
    std::vector<Param> WITHGPSPARAMS, WITHOUTGPSPARAMS;
    struct Markov{
        float* PHIMESSAGE;
        float* PSIMESSAGE;
    };
    Markov MarkovTool;
    struct BESTSTATE{
        int index;
        float value;
    };
    int EstINDEX;
    int TrueOffset;
    int Iterations;
    float Threshold;
	float lambda;

	// Basic functions
	float GPSDistance(Data* RefPoint, Data* CompPoint);
    float TimeDistance(Data* CurrComp, Data* NextComp, Data* CurrRef, Data* NextRef, int offset);
    BESTSTATE FindMax(float* array, int PossibleStates);

	// Loading/Unloading galleries
    void BrowseReference(std::string Path);
    void BrowseReference(std::string Path, std::string TXTPath);
    void BrowseSecond(std::string Path);
    void BrowseSecond(std::string Path, std::string TXTPath);
    void ChangeReference();
    void ChangeSecond();

	// Inference
    void Test();

	// Training
    void Confirm();
    void Newton();

};

#endif // MEDIASYNC_H
