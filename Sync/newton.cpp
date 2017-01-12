#include "mediasync.h"
#include <cmath>

/*************************************
	CONFIGURATION OF TRAINING
**************************************/
void MediaSync::Confirm()
{
  std::cout << "Preparing the computation...\nThis operation can take some hours!\nWait until the operation has finished...\n";

    std::vector<int> prevComparison;
    std::vector<int> currComparison;
    std::vector<float> currGPSDistance;
    std::vector<float> GPSMAX;
    std::vector<float> TimeMAX;
    int trueoffset = CompareGallery.at(0).UTCDateTimeOriginal - ReferenceGallery.at(0).UTCDateTimeOriginal;
    int dist;


    for(unsigned int i=0; i<CompareGallery.size(); i++){
        int temp = -1;
        int tempf = 0.0;
        prevComparison.push_back(temp);
        currComparison.push_back(temp);
        currGPSDistance.push_back(tempf);
    }


    for(unsigned int i=0; i<ComparePossibleStates.size(); i++){


        //Updating time of the gallery to be compared for each offset
        for(unsigned int j=0; j<CompareGallery.size(); j++){
            CompareGallery.at(j).Offset = ComparePossibleStates.at(i).offset;
            GPSMAX.push_back(0.0);
            if(j != CompareGallery.size() - 1)
            TimeMAX.push_back(0.0);
        }
        ComparePossibleStates.at(i).gi = 0;
        ComparePossibleStates.at(i).hacca = 0;

        for(unsigned int j=0; j<CompareGallery.size(); j++){
            currComparison.at(j) = ComparePossibleStates.at(i).StatesCombination.at(j);

            if(currComparison.at(j) != prevComparison.at(j)){
                currGPSDistance.at(j) = GPSDistance(&(ReferenceGallery.at(ComparePossibleStates.at(i).StatesCombination.at(j) - 1)),
                                                    &(CompareGallery.at(j)));
            }
            ComparePossibleStates.at(i).g.push_back(currGPSDistance.at(j));
            prevComparison.at(j) = currComparison.at(j);

        }
        for(unsigned int j = 0; j < CompareGallery.size() - 1; j++){
            ComparePossibleStates.at(i).h.push_back(TimeDistance(&(CompareGallery.at(j)), &(CompareGallery.at(j + 1)),
                                                              &(ReferenceGallery.at(ComparePossibleStates.at(i).StatesCombination.at(j) - 1)),
                                                              &(ReferenceGallery.at(ComparePossibleStates.at(i).StatesCombination.at(j + 1) - 1)),
															  ComparePossibleStates.at(i).offset));
        }

    }


    prevComparison.clear();
    currComparison.clear();
    currGPSDistance.clear();

    //Computing real hacca,gi functions
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
    for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
        for(unsigned int j=0; j<CompareGallery.size(); j++){
            ComparePossibleStates.at(i).gi += (ComparePossibleStates.at(i).g.at(j)/GPSMAX.at(j));
            if(j != CompareGallery.size() - 1){
                ComparePossibleStates.at(i).hacca += (ComparePossibleStates.at(i).h.at(j)/TimeMAX.at(j));
            }
       }
    }

    for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
        if(i == 0){
            dist = abs(trueoffset - ComparePossibleStates.at(i).offset - CompareGallery.at(0).ReferencedDateTimeOriginal);
            TrueOffset = ComparePossibleStates.at(i).offset + CompareGallery.at(0).ReferencedDateTimeOriginal;
            EstINDEX = i;
        }
        if(dist > abs(trueoffset - ComparePossibleStates.at(i).offset - CompareGallery.at(0).ReferencedDateTimeOriginal)){
            dist = abs(trueoffset - ComparePossibleStates.at(i).offset - CompareGallery.at(0).ReferencedDateTimeOriginal);
            TrueOffset = ComparePossibleStates.at(i).offset + CompareGallery.at(0).ReferencedDateTimeOriginal;
            EstINDEX = i;
        }
    }

}


/*************************************
	TRAINING PROCEDURE
**************************************/
void MediaSync::Newton()
{
    std::string Temp;

    Iterations = 1000;
    Threshold = 0.0;
    CurrParameters.gamma = CoarseLearn.gamma;
    CurrParameters.delta = CoarseLearn.delta;
    float start3 = CurrParameters.gamma;
    float start4 = CurrParameters.delta;


    std::cout << "\n+++++++++++++++++++++++++++++++++++++++++++\n";
    std::cout << "        ESTIMATION OF PARAMETERS";
    std::cout << "+++++++++++++++++++++++++++++++++++++++++++\n";
    std::cout << "True offset (in seconds): " << TrueOffset << "\n";
    std::cout << "Max. number of iterations: " << Iterations << "\n";
    std::cout << "Stop threshold value: " << Threshold << "\n";
    std::cout << " Gamma: " << CurrParameters.gamma;
    std::cout << " Delta: " << CurrParameters.delta << "\n";

    float distance = sqrt(pow(1.0, 2) +
                          pow(1.0, 2));
    int index = 0;
    Eigen::Matrix4f A;
    Eigen::Vector4f b;
    float A33, A34, A43, A44;
    float b3, b4;

    //Newton's method
    while((index < Iterations) && (distance > Threshold)){
        PrevParameters.gamma = CurrParameters.gamma;
        PrevParameters.delta = CurrParameters.delta;

        A33 = 0;
        A34 = 0;
        A43 = 0;
        A44 = 0;
        b3 = 0;
        b4 = 0;

        int temp = 0;
        while((ComparePossibleStates.at(temp).offset + CompareGallery.at(0).ReferencedDateTimeOriginal) != TrueOffset){
            temp++;
        }
        //Calculation of elements of MATRIX A
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            A33 += pow(ComparePossibleStates.at(i).gi, 2)*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }
        A33 = -1*A33;
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            A33 += ComparePossibleStates.at(temp).gi*ComparePossibleStates.at(i).gi*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }


        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            A34 += ComparePossibleStates.at(i).gi*ComparePossibleStates.at(i).hacca*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }
        A34 = -1*A34;
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            A34 += ComparePossibleStates.at(temp).gi*ComparePossibleStates.at(i).hacca*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }


        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            A43 += ComparePossibleStates.at(i).hacca*ComparePossibleStates.at(i).gi*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }
        A43 = -1*A43;
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            A43 += ComparePossibleStates.at(temp).hacca*ComparePossibleStates.at(i).gi*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }


        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            A44 += pow(ComparePossibleStates.at(i).hacca, 2)*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }
        A44 = -1*A44;
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            A44 += ComparePossibleStates.at(temp).hacca*ComparePossibleStates.at(i).hacca*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }


        //Calculation of vector b
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            b3 += ComparePossibleStates.at(i).gi*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }
        b3 = -1*b3;
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            b3 += ComparePossibleStates.at(temp).gi*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }


        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            b4 += ComparePossibleStates.at(i).hacca*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }
        b4 = -1*b4;
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            b4 += ComparePossibleStates.at(temp).hacca*exp((-1)*
                   (PrevParameters.gamma*ComparePossibleStates.at(i).gi +
                    PrevParameters.delta*ComparePossibleStates.at(i).hacca));
        }


        A << A33,A34,A43,A44;
        b << b3, b4;
		Eigen::Vector4f x = A.fullPivHouseholderQr().solve(b);

        CurrParameters.gamma = PrevParameters.gamma + x(0);
        CurrParameters.delta = PrevParameters.delta + x(1);
        std::cout << "Iter: " << index << "\n";
        std::cout << " gamma: " << CurrParameters.gamma;
        std::cout << " delta: " << CurrParameters.delta << "\n";

        b3 = 0;
        b4 = 0;
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){

            b3 += ComparePossibleStates.at(temp).gi*exp((-1)*
                   (CurrParameters.gamma*ComparePossibleStates.at(i).gi +
                    CurrParameters.delta*ComparePossibleStates.at(i).hacca));

            b4 += ComparePossibleStates.at(temp).hacca*exp((-1)*
                   (CurrParameters.gamma*ComparePossibleStates.at(i).gi +
                    CurrParameters.delta*ComparePossibleStates.at(i).hacca));
        }
        b3 = -1*b3;
        b4 = -1*b4;
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
            b3 += ComparePossibleStates.at(i).gi*exp((-1)*
                   (CurrParameters.gamma*ComparePossibleStates.at(i).gi +
                    CurrParameters.delta*ComparePossibleStates.at(i).hacca));

            b4 += ComparePossibleStates.at(i).hacca*exp((-1)*
                   (CurrParameters.gamma*ComparePossibleStates.at(i).gi +
                    CurrParameters.delta*ComparePossibleStates.at(i).hacca));
        }


        distance = sqrt(pow(b3, 2) +
                        pow(b4, 2));

        index++;

    }
    std::cout << "Parameters have been estimated in " << index << " iterations and with an error of " << distance << "\n";
    std::cout << " gamma: " << CurrParameters.gamma;
    std::cout << " delta: " << CurrParameters.delta << "\n";

	std::fstream outputFile;
	outputFile.open (PARAMETERSFILE, std::fstream::app);
	outputFile << ReferenceDir << "\n";
	outputFile << CompareDir << "\n";
	if(CurrParameters.gamma == 0){
		outputFile << "NO GPS\n";
	}
	else{
		outputFile << "WITH GPS\n";
	}
	outputFile << "Max number of iter: " << std::to_string(Iterations) << "\n";
	outputFile << "Stop value: " << std::to_string(Threshold) << "\n";
	outputFile << "Iterations: " << index << "\n";
	outputFile << std::to_string(start3) << " " <<
				  std::to_string(start4) << "\n";
	outputFile << std::to_string(CurrParameters.gamma) << " " <<
				  std::to_string(CurrParameters.delta) << "\n";
	outputFile.close();
}

