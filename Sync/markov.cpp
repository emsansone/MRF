#include "mediasync.h"
#include <cmath>


/*************************************
    Finding the maximum in an array
	Returning the maximum value toge-
	ther with the index
**************************************/
MediaSync::BESTSTATE MediaSync::FindMax(float* array, int PossibleStates){

    BESTSTATE BestState;
    BestState.index = 0;
    BestState.value = -1*FLT_MAX;

	std::fstream outputFile;

    for(int i=0; i<PossibleStates; i++){

        if(array[i]>BestState.value){
            BestState.value = array[i];
            BestState.index = i;
        }
    }
    return BestState;
}



/*************************************
	RUNNING INFERENCE
**************************************/
void MediaSync::Test()
{


    std::string line;
    std::vector<std::string> params;
    float Performance = 0.0;
    float Confidence = 0.0;
    float Z = 0.0;
    int lines = 0;
    std::string IsGPS;
    int PrevComparison;
    int CurrComparison;
    float CurrGPSDistance;
    std::vector<float> GPSMAX;
    std::vector<float> TimeMAX;


    //Calculating parameters for potential functions
    std::fstream file;
	file.open(PARAMETERSFILE, std::fstream::in);
	if (!file.is_open()){
        std::cout << "Parameters.txt cannot be opened!\n";
        return;
    }

    while(std::getline(file,line)){
		std::getline(file,line);
		std::getline(file,line);
        IsGPS = line;
		std::getline(file,line);
		std::getline(file,line);
		std::getline(file,line);
		std::getline(file,line);
		std::getline(file,line);
		char * pch = std::strtok(strdup(line.c_str())," ");
		while (pch != NULL){
			params.push_back(pch);
			pch = strtok (NULL, " ");
		}
        PrevParameters.gamma = std::stof(params.at(0));
        PrevParameters.delta = std::stof(params.at(1));
        params.clear();
        if(IsGPS.compare("NO GPS") == 0){
            WITHOUTGPSPARAMS.push_back(PrevParameters);
        }
        else{
            WITHGPSPARAMS.push_back(PrevParameters);
        }
        params.clear();
    }
    file.close();



    PrevParameters.gamma = 0;
    PrevParameters.delta = 0;
    for(unsigned int i=0; i<WITHGPSPARAMS.size(); i++){
        PrevParameters.gamma += WITHGPSPARAMS.at(i).gamma;
        PrevParameters.delta += WITHGPSPARAMS.at(i).delta;
    }
    PrevParameters.gamma = PrevParameters.gamma/(float)WITHGPSPARAMS.size();

    CurrParameters.gamma = 0;
    CurrParameters.delta = 0;
    for(unsigned int i=0; i<WITHOUTGPSPARAMS.size(); i++){
        CurrParameters.delta += WITHOUTGPSPARAMS.at(i).delta;
    }

    CurrParameters.gamma = PrevParameters.gamma;
    CurrParameters.delta = (CurrParameters.delta + PrevParameters.delta)/((float)WITHOUTGPSPARAMS.size() + (float)WITHGPSPARAMS.size());

    std::cout << "Synchronization\n";
    std::cout << "[Parameters] [value]\n";
    std::cout << "gamma:        " << CurrParameters.gamma << "\n";
    std::cout << "delta:            " << CurrParameters.delta << "\n";


    for(unsigned int j=0; j<CompareGallery.size(); j++){
        GPSMAX.push_back(0.0);
        if(j != CompareGallery.size() - 1)
        TimeMAX.push_back(0.0);
    }

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     *                     MARKOV RANDOM FIELD: MAX-SUM PRODUCT ALGORITHM
     *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    //Dynamic allocation of arrays and matrices used for MAX-SUM PRODUCT algorithm
    int PossibleStates = (int) ComparePossibleStates.size();
    MarkovTool.PHIMESSAGE = new float[PossibleStates];
    MarkovTool.PSIMESSAGE = new float[PossibleStates];
    for(int i=0; i<PossibleStates; i++){
        MarkovTool.PHIMESSAGE[i] = 0;
        MarkovTool.PSIMESSAGE[i] = 0;
    }

    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     *              Computing MAX distance values
     *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    for(unsigned int i=0; i<CompareGallery.size(); i++){

        PrevComparison = -1;

        for(int j=0; j<PossibleStates; j++){
            CurrComparison = ComparePossibleStates.at(j).StatesCombination.at(i);

            if(i != CompareGallery.size() - 1)
            ComparePossibleStates.at(j).h.push_back(TimeDistance(&(CompareGallery.at(i)),
                                                                 &(CompareGallery.at(i + 1)),
                                                                 &(ReferenceGallery.at(ComparePossibleStates.at(j).StatesCombination.at(i) - 1)),
                                                                 &(ReferenceGallery.at(ComparePossibleStates.at(j).StatesCombination.at(i + 1) - 1)),
                                                                   ComparePossibleStates.at(j).offset));

            if(CurrComparison == PrevComparison){
                ComparePossibleStates.at(j).g.push_back(CurrGPSDistance);

            }
            else{
				CurrGPSDistance = GPSDistance(&(ReferenceGallery.at(ComparePossibleStates.at(j).StatesCombination.at(i) - 1)),
                                              &(CompareGallery.at(i)));
                ComparePossibleStates.at(j).g.push_back(CurrGPSDistance);
            }
            PrevComparison = CurrComparison;
        }
    }
    for(unsigned int j=0; j<CompareGallery.size(); j++){


        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){

            if(GPSMAX.at(j) <= ComparePossibleStates.at(i).g.at(j))
                GPSMAX.at(j) = ComparePossibleStates.at(i).g.at(j);

            if(j != CompareGallery.size() - 1){
                if(TimeMAX.at(j) <= ComparePossibleStates.at(i).h.at(j))
                    TimeMAX.at(j) = ComparePossibleStates.at(i).h.at(j);
            }
        }

    }

    for(unsigned int i=0; i<CompareGallery.size() - 1; i++){

        PrevComparison = -1;
        /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         *              Message from observed to hidden node
         *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
        for(int j=0; j<PossibleStates; j++){
            MarkovTool.PHIMESSAGE[j] = -1*CurrParameters.gamma*ComparePossibleStates.at(j).g.at(i)/GPSMAX.at(i);
        }

        /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         *              Message from hidden to hidden node
         *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
        //  1.From hidden node to factor node
        for(int j=0; j<PossibleStates; j++){
            MarkovTool.PHIMESSAGE[j] = MarkovTool.PHIMESSAGE[j] + MarkovTool.PSIMESSAGE[j];
        }

        //  2.From factor node to successive hidden node
        for(int j=0; j<PossibleStates; j++){
            MarkovTool.PSIMESSAGE[j] = -1*CurrParameters.delta*ComparePossibleStates.at(j).h.at(i)/TimeMAX.at(i);
        }
        for(int j=0; j<PossibleStates; j++){
                MarkovTool.PSIMESSAGE[j] = MarkovTool.PSIMESSAGE[j] + MarkovTool.PHIMESSAGE[j];
        }

    }

    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     *                     MARKOV RANDOM FIELD: BACK-TRACKING PHASE
     *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    //Computing optimal state for last photo
    int Solution;
    PrevComparison = -1;
    for(int j=0; j<PossibleStates; j++){
        MarkovTool.PHIMESSAGE[j] = -1*CurrParameters.gamma*ComparePossibleStates.at(j).g.at(CompareGallery.size() - 1)/GPSMAX.at(CompareGallery.size() - 1);
    }
    for(int j=0; j<PossibleStates; j++){
        MarkovTool.PHIMESSAGE[j] = MarkovTool.PHIMESSAGE[j] + MarkovTool.PSIMESSAGE[j];
        Z += MarkovTool.PHIMESSAGE[j];
    }
    EstINDEX = FindMax(MarkovTool.PHIMESSAGE, PossibleStates).index;
	Solution = ComparePossibleStates.at(EstINDEX).offset + CompareGallery.at(0).ReferencedDateTimeOriginal;
	Confidence = MarkovTool.PHIMESSAGE[EstINDEX]/Z;


    int RealOffset = CompareGallery.at(0).UTCDateTimeOriginal - ReferenceGallery.at(0).UTCDateTimeOriginal;

    std::cout << "MAX-SUM PRODUCT algorithm has finished!\n";
    std::cout << "Offset: " << RealOffset - Solution << "\n";

	std::fstream outputFile;
	outputFile.open (PERFORMANCEFILE, std::fstream::app);
    outputFile << ReferenceDir.c_str() << "\n";
    outputFile << CompareDir.c_str() << "\n";
    outputFile << CurrParameters.gamma << " " <<
            CurrParameters.delta << "\n";
    outputFile << "Offset: " << RealOffset - Solution << "\n";
    outputFile.close();

    GPSMAX.clear();
    TimeMAX.clear();
    WITHGPSPARAMS.clear();
    WITHOUTGPSPARAMS.clear();
    //Freeing memory from arrays and matrices
    delete [] MarkovTool.PHIMESSAGE;
    delete [] MarkovTool.PSIMESSAGE;
}



