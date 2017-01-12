#include "mediasync.h"
#include <cmath>


/*************************************
    This function takes as input
    two strings that represent
    two dates and times and returns
    the difference in seconds
**************************************/
time_t MediaSync::DateTimeToSec(std::string DateTime1, std::string DateTime2){

    struct tm tm1, tm2;
    time_t val1,val2,sec;

    char dateInput1[19];
    char dateInput2[19];
    for (int a=0;a<19;a++)
    {
        dateInput1[a] = DateTime1[a];
        dateInput2[a] = DateTime2[a];
    }


    memset(&tm1, 0, sizeof(struct tm));
    memset(&tm2, 0, sizeof(struct tm));

    sscanf_s(dateInput1, "%4d:%2d:%2d %2d:%2d:%2d",&(tm1.tm_year),&(tm1.tm_mon),&(tm1.tm_mday),&(tm1.tm_hour),&(tm1.tm_min),&(tm1.tm_sec));
    sscanf_s(dateInput2, "%4d:%2d:%2d %2d:%2d:%2d",&(tm2.tm_year),&(tm2.tm_mon),&(tm2.tm_mday),&(tm2.tm_hour),&(tm2.tm_min),&(tm2.tm_sec));

    tm1.tm_year -= 1900;
    tm2.tm_year -= 1900;

    val1 = mktime(&tm1);
    val2 = mktime(&tm2);

    if(DateTime1 < DateTime2){
        sec = difftime(val2,val1);
    }
    else{
        sec = difftime(val1,val2);
    }

    return sec;
}


/*************************************
	LOADING THE REFERENCE GALLERY
**************************************/
void MediaSync::BrowseReference(std::string Path)
{

    std::string Command;
    Data PhotoInfos;
    PhotoInfos.index = 0;
    PhotoInfos.GPSLatitude = GPSOVERVALUE;
    PhotoInfos.GPSLongitude = GPSOVERVALUE;
    ReferenceDir = Path;

    //Loading reference gallery
    std::cout << "Loading reference gallery...\n";

    //Executing Exiftool and printing a text file with all the meta-information of the photos
    ReferenceMetadataFile = "ReferenceMetadata.txt";
    Command = "exiftool.exe -S -fileOrder DateTimeOriginal -FileName -EXIF:DateTimeOriginal -GPS:GPSLatitudeRef -c \"%.6f\" -GPS:GPSLatitude -GPS:GPSLongitudeRef -c \"%.6f\" -GPS:GPSLongitude \"";
	Command.append(ReferenceDir);
	Command.append("/\" > ./");
	Command.append(ReferenceMetadataFile);
    system(Command.c_str());

    //Reading text file and populating the reference gallery vector
    Command = "./";
	Command.append(ReferenceMetadataFile);
	std::fstream file;
	file.open (Command, std::fstream::in);

    std::string stringline;
	std::getline(file, stringline);
    while (stringline.find("========") != std::string::npos) {

         //Reading FileName
	     std::getline(file, stringline);
         stringline = stringline.substr(stringline.find(": ") + 2);
         //Processing FileName
         PhotoInfos.index++;
         PhotoInfos.FileName = stringline;
         PhotoInfos.Offset = 0;
         PhotoInfos.State = PhotoInfos.index;

         //Reading DateTimeOriginal
	     std::getline(file, stringline);
         if(stringline.find("CreateDate") == std::string::npos){
             std::cout << "ERROR:The photo gallery doesn't have DateTimeDigitized field!\n";
             return;
         }
         stringline = stringline.substr(stringline.find(": ") + 2);

		 //Processing DateTimeOriginal
         PhotoInfos.DateTimeOriginal = stringline;
         PhotoInfos.UTCDateTimeOriginal = DateTimeToSec(TIMEREFERENCE, stringline);
         ReferenceGallery.push_back(PhotoInfos);
         ReferenceGallery.at(PhotoInfos.index - 1).ReferencedDateTimeOriginal = ReferenceGallery.at(PhotoInfos.index - 1).UTCDateTimeOriginal - ReferenceGallery.at(0).UTCDateTimeOriginal + PhotoInfos.Offset;

         //Reading GPSLatitudeRef
	     std::getline(file, stringline);
         if(stringline.find("GPSLatitudeRef") != std::string::npos){
             stringline = stringline.substr(stringline.find(": ") + 2);
             std::string prefix = "";
             if(stringline.find("North") == std::string::npos)
                 prefix = "-";

             //Reading GPSLatitude
		     std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             stringline = prefix.append(stringline);
			 ReferenceGallery.at(PhotoInfos.index - 1).GPSLatitude = std::stof(stringline);

             //Reading GPSLongitudeRef
		     std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             prefix = "";
             if(stringline.find("East") == std::string::npos)
                 prefix = "-";

             //Reading GPSLongitude
		     std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             stringline = prefix.append(stringline);
             ReferenceGallery.at(PhotoInfos.index - 1).GPSLongitude = std::stof(stringline);

		     std::getline(file, stringline);
         }
    }
    file.close();

    for(unsigned int i=0; i<ReferenceGallery.size(); i++){
        ReferenceGallery.at(i).Class = 1;
    }
    std::cout << "Reference gallery has been loaded!\n";

    if(ReferenceGallery.size() == 0){
        std::cout << "This is not a valid entry!\n";
    }
}


/*************************************
	LOADING THE REFERENCE GALLERY
	WITH LESS IMAGES
**************************************/
void MediaSync::BrowseReference(std::string Path, std::string TXTPath)
{

    std::string Command;
	std::vector<std::string> Images;
    Data PhotoInfos;
    PhotoInfos.index = 0;
    PhotoInfos.GPSLatitude = GPSOVERVALUE;
    PhotoInfos.GPSLongitude = GPSOVERVALUE;
    ReferenceDir = Path;

    //Loading reference gallery
    std::cout << "Loading reference gallery...\n";

    //Executing Exiftool and printing a text file with all the meta-information of the photos
    ReferenceMetadataFile = "ReferenceMetadata.txt";
    Command = "exiftool.exe -S -fileOrder DateTimeOriginal -FileName -EXIF:DateTimeOriginal -GPS:GPSLatitudeRef -c \"%.6f\" -GPS:GPSLatitude -GPS:GPSLongitudeRef -c \"%.6f\" -GPS:GPSLongitude \"";
	Command.append(ReferenceDir);
	Command.append("/\" > ./");
	Command.append(ReferenceMetadataFile);
    system(Command.c_str());

    //Reading text file and populating the reference gallery vector
    Command = "./";
	Command.append(ReferenceMetadataFile);

	std::fstream file;
    std::string stringline;
	file.open(TXTPath, std::fstream::in);
	while(std::getline(file, stringline)){
		Images.push_back(stringline);
	}
	file.close();

	file.open (Command, std::fstream::in);
	std::getline(file, stringline);
    while (stringline.find("========") != std::string::npos) {

         //Reading FileName
	     std::getline(file, stringline);
         stringline = stringline.substr(stringline.find(": ") + 2);

		 int flag = 0;
		 for(unsigned int i=0; i<Images.size(); i++){
			 if(stringline.compare(Images.at(i)) == 0){
				 flag = 1;
			 }
		 }

		 //Processing FileName
		 if(flag == 1){
			 PhotoInfos.index++;
			 PhotoInfos.FileName = stringline;
			 PhotoInfos.Offset = 0;
			 PhotoInfos.State = PhotoInfos.index;
		 }

         //Reading DateTimeOriginal
	     std::getline(file, stringline);
		 if(stringline.find("CreateDate") == std::string::npos){
		 	std::cout << "ERROR:The photo gallery doesn't have DateTimeDigitized field!\n";
		 	return;
		 }
		 stringline = stringline.substr(stringline.find(": ") + 2);

	     //Processing DateTimeOriginal
		 if(flag == 1){
			 PhotoInfos.DateTimeOriginal = stringline;
			 PhotoInfos.UTCDateTimeOriginal = DateTimeToSec(TIMEREFERENCE, stringline);
			 ReferenceGallery.push_back(PhotoInfos);
			 ReferenceGallery.at(PhotoInfos.index - 1).ReferencedDateTimeOriginal = ReferenceGallery.at(PhotoInfos.index - 1).UTCDateTimeOriginal - ReferenceGallery.at(0).UTCDateTimeOriginal + PhotoInfos.Offset;
		 }

         //Reading GPSLatitudeRef
	     std::getline(file, stringline);
         if(stringline.find("GPSLatitudeRef") != std::string::npos){
             stringline = stringline.substr(stringline.find(": ") + 2);
             std::string prefix = "";
             if(stringline.find("North") == std::string::npos)
                 prefix = "-";

             //Reading GPSLatitude
		     std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             stringline = prefix.append(stringline);
			 if(flag == 1)
				ReferenceGallery.at(PhotoInfos.index - 1).GPSLatitude = std::stof(stringline);

             //Reading GPSLongitudeRef
		     std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             prefix = "";
             if(stringline.find("East") == std::string::npos)
                 prefix = "-";

             //Reading GPSLongitude
		     std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             stringline = prefix.append(stringline);
			 if(flag == 1)
				ReferenceGallery.at(PhotoInfos.index - 1).GPSLongitude = std::stof(stringline);

		     std::getline(file, stringline);
         }
    }
    file.close();

    for(unsigned int i=0; i<ReferenceGallery.size(); i++){
        ReferenceGallery.at(i).Class = 1;
    }
    std::cout << "Reference gallery has been loaded!\n";

    if(ReferenceGallery.size() == 0){
        std::cout << "This is not a valid entry!\n";
    }
}


/*************************************
	LOADING THE SECOND GALLERY
**************************************/
void MediaSync::BrowseSecond(std::string Path)
{
    std::string Command;
    Data PhotoInfos;
    PhotoInfos.index = 0;
    PhotoInfos.GPSLatitude = GPSOVERVALUE;
    PhotoInfos.GPSLongitude = GPSOVERVALUE;
    CompareGallery.clear();
    CompareDir = Path;

    //Loading the gallery to be compared
    std::cout << "Loading gallery to be compared...\n";
    unsigned t0=clock(),t1;

    CompareMetadataFile = "CompareMetadata.txt";
    Command = "exiftool.exe -S -fileOrder DateTimeOriginal -FileName -EXIF:DateTimeOriginal -GPS:GPSLatitudeRef -c \"%.6f\" -GPS:GPSLatitude -GPS:GPSLongitudeRef -c \"%.6f\" -GPS:GPSLongitude \"";
	Command.append(CompareDir);
	Command.append("/\" > ./");
	Command.append(CompareMetadataFile);
    system(Command.c_str());

    //Reading text file and populating the reference gallery vector
    Command = "./";
	Command.append(CompareMetadataFile);
	std::fstream file;
	file.open (Command, std::fstream::in);

    std::string stringline;
	std::getline(file, stringline);
    while (stringline.find("========") != std::string::npos) {

         //Reading FileName
		 std::getline(file, stringline);
         stringline = stringline.substr(stringline.find(": ") + 2);

		 //Processing FileName
         PhotoInfos.index++;
         PhotoInfos.FileName = stringline;
         PhotoInfos.Offset = 0;
         PhotoInfos.State = 0;

         //Reading DateTimeOriginal
		 std::getline(file, stringline);
         if(stringline.find("CreateDate") == std::string::npos){
             std::cout << "ERROR:The photo gallery doesn't have DateTimeDigitized field!\n";
            return;
         }
         stringline = stringline.substr(stringline.find(": ") + 2);

		 //Processing DateTimeOriginal
         PhotoInfos.DateTimeOriginal = stringline;
         PhotoInfos.UTCDateTimeOriginal = DateTimeToSec(TIMEREFERENCE, stringline);
         CompareGallery.push_back(PhotoInfos);
         CompareGallery.at(PhotoInfos.index - 1).ReferencedDateTimeOriginal = CompareGallery.at(PhotoInfos.index - 1).UTCDateTimeOriginal - CompareGallery.at(0).UTCDateTimeOriginal + PhotoInfos.Offset;

         //Reading GPSLatitudeRef
		 std::getline(file, stringline);
         if(stringline.find("GPSLatitudeRef") != std::string::npos){
             stringline = stringline.substr(stringline.find(": ") + 2);
             std::string prefix = "";
             if(stringline.find("North") == std::string::npos)
                 prefix = "-";

             //Reading GPSLatitude
			 std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             stringline = prefix.append(stringline);
             CompareGallery.at(PhotoInfos.index - 1).GPSLatitude = std::stof(stringline);

             //Reading GPSLongitudeRef
			 std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             prefix = "";
             if(stringline.find("East") == std::string::npos)
                 prefix = "-";

             //Reading GPSLongitude
			 std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             stringline = prefix.append(stringline);
             CompareGallery.at(PhotoInfos.index - 1).GPSLongitude = std::stof(stringline);

			 std::getline(file, stringline);

         }
    }
    file.close();

    if(CompareGallery.size() == 0){
        std::cout << "THIS IS NOT A VALID ENTRY!\n";
    }
    else{

        std::cout << "Gallery to be compared has been loaded!\n\nCalculating the states for the Markov Random Field network...\n";

		for(unsigned int i=0; i<CompareGallery.size(); i++){
			CompareGallery.at(i).Class = 0;
			CompareGallery.at(i).ReferencedDateTimeOriginal = CompareGallery.at(i).UTCDateTimeOriginal - CompareGallery.at(0).UTCDateTimeOriginal;
		}

		int Steps;
		int Max_Steps = ReferenceGallery.at(ReferenceGallery.size()-1).ReferencedDateTimeOriginal-CompareGallery.at(CompareGallery.size()-1).ReferencedDateTimeOriginal;
		std::vector<int> States;
		for(unsigned int i=0; i<ReferenceGallery.size(); i++){
			for(unsigned int j=0; j<CompareGallery.size(); j++){
				int temp = ReferenceGallery.at(i).ReferencedDateTimeOriginal-CompareGallery.at(j).ReferencedDateTimeOriginal;
				if(abs(temp) < Max_Steps){
					States.push_back(temp);
				}
			}		
		}
		Steps = States.size();
		std::sort(States.begin(),States.end());

		std::vector<int> CurrentCombination;
		CurrentCombination.clear();
		//Initializing possible combinations
		for(unsigned int i=0; i<CompareGallery.size(); i++){
			CurrentCombination.push_back(0);
		}

		//Searching among all possible offsets the potential ones
		for(unsigned int i=0; i<Steps; i++){
			//Calculation of the combination of states, given an offset
			for(unsigned int j=0; j<CompareGallery.size(); j++){
				int index = 1;
				int CurrentSum, NextSum;
				if(Steps > 1){
					// Distance from the first reference photo
					CurrentSum = abs(CompareGallery.at(j).ReferencedDateTimeOriginal + States.at(i));

					// Distance from the second reference photo
					NextSum = abs(ReferenceGallery.at(index).ReferencedDateTimeOriginal - CompareGallery.at(j).ReferencedDateTimeOriginal - States.at(i));

					while((CurrentSum >= NextSum) && (((unsigned int) index) < ReferenceGallery.size())){
						index++;
						CurrentSum = NextSum;
						if((unsigned int) index != ReferenceGallery.size()){
								NextSum = abs(ReferenceGallery.at(index).ReferencedDateTimeOriginal - CompareGallery.at(j).ReferencedDateTimeOriginal - States.at(i));
						}
					}
				}
				CurrentCombination.at(j) = index;
			}
			Offset temp;
			temp.offset = States.at(i);
			for(unsigned int z=0; z<CompareGallery.size(); z++){
				temp.StatesCombination.push_back(CurrentCombination.at(z));
			}
			ComparePossibleStates.push_back(temp);
		}

		std::fstream outputFile;
		outputFile.open (STATESFILE, std::fstream::out);
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
			outputFile << "Offset " << std::to_string(i + 1) << ": " <<
                    std::to_string(ComparePossibleStates.at(i).offset + CompareGallery.at(0).ReferencedDateTimeOriginal) << "\n";
            std::string temp;
            for(unsigned int z=0; z<CompareGallery.size(); z++){
                if((unsigned int) z == CompareGallery.size() - 1){
					temp.append(std::to_string(ComparePossibleStates.at(i).StatesCombination.at(z)));
                }
                else{
					temp.append(std::to_string(ComparePossibleStates.at(i).StatesCombination.at(z)));
					temp.append(",");
                }
            }
            outputFile << temp << "\n\n";
        }
        outputFile.close();

        t1=clock()-t0;
        int RunTime = t1/CLOCKS_PER_SEC;
        int Hours = RunTime/3600;
        int Minutes = (RunTime%3600)/60;
        int Seconds = ((RunTime%3600)%60);
        std::cout << "DONE! Total elapsed time: " << Hours << "h " <<
                                                    Minutes << "m " <<
                                                    Seconds << "s\n";


    }
}


/*************************************
	LOADING THE SECOND GALLERY
	WITH LESS IMAGES
**************************************/
void MediaSync::BrowseSecond(std::string Path, std::string TXTPath)
{
    std::string Command;
	std::vector<std::string> Images;
    Data PhotoInfos;
    PhotoInfos.index = 0;
    PhotoInfos.GPSLatitude = GPSOVERVALUE;
    PhotoInfos.GPSLongitude = GPSOVERVALUE;
    CompareGallery.clear();
    CompareDir = Path;

    //Loading the gallery to be compared
    std::cout << "Loading gallery to be compared...\n";
    unsigned t0=clock(),t1;

    CompareMetadataFile = "CompareMetadata.txt";
    Command = "exiftool.exe -S -fileOrder DateTimeOriginal -FileName -EXIF:DateTimeOriginal -GPS:GPSLatitudeRef -c \"%.6f\" -GPS:GPSLatitude -GPS:GPSLongitudeRef -c \"%.6f\" -GPS:GPSLongitude \"";
	Command.append(CompareDir);
	Command.append("/\" > ./");
	Command.append(CompareMetadataFile);
    system(Command.c_str());

    //Reading text file and populating the reference gallery vector
    Command = "./";
	Command.append(CompareMetadataFile);

	std::fstream file;
    std::string stringline;
	file.open(TXTPath, std::fstream::in);
	while(std::getline(file, stringline)){
		Images.push_back(stringline);
	}
	file.close();

	file.open (Command, std::fstream::in);
	std::getline(file, stringline);
    while (stringline.find("========") != std::string::npos) {

         //Reading FileName
		 std::getline(file, stringline);
         stringline = stringline.substr(stringline.find(": ") + 2);

		 int flag = 0;
		 for(unsigned int i=0; i<Images.size(); i++){
			 if(stringline.compare(Images.at(i)) == 0){
				 flag = 1;
			 }
		 }

		 //Processing FileName
		 if(flag == 1){
			 PhotoInfos.index++;
			 PhotoInfos.FileName = stringline;
			 PhotoInfos.Offset = 0;
			 PhotoInfos.State = 0;
		 }

         //Reading DateTimeOriginal
		 std::getline(file, stringline);
		 if(stringline.find("CreateDate") == std::string::npos){
		 	std::cout << "ERROR:The photo gallery doesn't have DateTimeDigitized field!\n";
			return;
		 }
		 stringline = stringline.substr(stringline.find(": ") + 2);

	     //Processing DateTimeOriginal
		 if(flag == 1){
			 PhotoInfos.DateTimeOriginal = stringline;
			 PhotoInfos.UTCDateTimeOriginal = DateTimeToSec(TIMEREFERENCE, stringline);
			 CompareGallery.push_back(PhotoInfos);
			 CompareGallery.at(PhotoInfos.index - 1).ReferencedDateTimeOriginal = CompareGallery.at(PhotoInfos.index - 1).UTCDateTimeOriginal - CompareGallery.at(0).UTCDateTimeOriginal + PhotoInfos.Offset;
		 }
         //Reading GPSLatitudeRef
		 std::getline(file, stringline);
         if(stringline.find("GPSLatitudeRef") != std::string::npos){
             stringline = stringline.substr(stringline.find(": ") + 2);
             std::string prefix = "";
             if(stringline.find("North") == std::string::npos)
                 prefix = "-";

             //Reading GPSLatitude
			 std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             stringline = prefix.append(stringline);
			 if(flag == 1)
	             CompareGallery.at(PhotoInfos.index - 1).GPSLatitude = std::stof(stringline);

             //Reading GPSLongitudeRef
			 std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             prefix = "";
             if(stringline.find("East") == std::string::npos)
                 prefix = "-";

             //Reading GPSLongitude
			 std::getline(file, stringline);
             stringline = stringline.substr(stringline.find(": ") + 2);
             stringline = prefix.append(stringline);
			 if(flag == 1)
				CompareGallery.at(PhotoInfos.index - 1).GPSLongitude = std::stof(stringline);

			 std::getline(file, stringline);

         }
    }
    file.close();

    if(CompareGallery.size() == 0){
        std::cout << "THIS IS NOT A VALID ENTRY!\n";
    }
    else{
        std::cout << "Gallery to be compared has been loaded!\n\nCalculating the states for the Markov Random Field network...\n";

		for(unsigned int i=0; i<CompareGallery.size(); i++){
			CompareGallery.at(i).Class = 0;
			CompareGallery.at(i).ReferencedDateTimeOriginal = CompareGallery.at(i).UTCDateTimeOriginal - CompareGallery.at(0).UTCDateTimeOriginal;
		}

		int Steps;
		int Max_Steps = ReferenceGallery.at(ReferenceGallery.size()-1).ReferencedDateTimeOriginal-CompareGallery.at(CompareGallery.size()-1).ReferencedDateTimeOriginal;
		std::vector<int> States;
		for(unsigned int i=0; i<ReferenceGallery.size(); i++){
			for(unsigned int j=0; j<CompareGallery.size(); j++){
				int temp = ReferenceGallery.at(i).ReferencedDateTimeOriginal-CompareGallery.at(j).ReferencedDateTimeOriginal;
				if(abs(temp) < Max_Steps){
					States.push_back(temp);
				}
			}		
		}
		Steps = States.size();
		std::sort(States.begin(),States.end());

		std::vector<int> CurrentCombination;
		CurrentCombination.clear();
		//Initializing possible combinations
		for(unsigned int i=0; i<CompareGallery.size(); i++){
			CurrentCombination.push_back(0);
		}

		//Searching among all possible offsets the potential ones
		for(unsigned int i=0; i<Steps; i++){
			//Calculation of the combination of states, given an offset
			for(unsigned int j=0; j<CompareGallery.size(); j++){
				int index = 1;
				int CurrentSum, NextSum;
				if(Steps > 1){
					// Distance from the first reference photo
					CurrentSum = abs(CompareGallery.at(j).ReferencedDateTimeOriginal + States.at(i));

					// Distance from the second reference photo
					NextSum = abs(ReferenceGallery.at(index).ReferencedDateTimeOriginal - CompareGallery.at(j).ReferencedDateTimeOriginal - States.at(i));

					while((CurrentSum >= NextSum) && (((unsigned int) index) < ReferenceGallery.size())){
						index++;
						CurrentSum = NextSum;
						if((unsigned int) index != ReferenceGallery.size()){
								NextSum = abs(ReferenceGallery.at(index).ReferencedDateTimeOriginal - CompareGallery.at(j).ReferencedDateTimeOriginal - States.at(i));
						}
					}
				}
				CurrentCombination.at(j) = index;
			}
			Offset temp;
			temp.offset = States.at(i);
			for(unsigned int z=0; z<CompareGallery.size(); z++){
				temp.StatesCombination.push_back(CurrentCombination.at(z));
			}
			ComparePossibleStates.push_back(temp);
		}

		std::fstream outputFile;
		outputFile.open (STATESFILE, std::fstream::out);
        for(unsigned int i=0; i<ComparePossibleStates.size(); i++){
			outputFile << "Offset " << std::to_string(i + 1) << ": " <<
                    std::to_string(ComparePossibleStates.at(i).offset + CompareGallery.at(0).ReferencedDateTimeOriginal) << "\n";
            std::string temp;
            for(unsigned int z=0; z<CompareGallery.size(); z++){
                if((unsigned int) z == CompareGallery.size() - 1){
					temp.append(std::to_string(ComparePossibleStates.at(i).StatesCombination.at(z)));
                }
                else{
					temp.append(std::to_string(ComparePossibleStates.at(i).StatesCombination.at(z)));
					temp.append(",");
                }
            }
            outputFile << temp << "\n\n";
        }
        outputFile.close();

        t1=clock()-t0;
        int RunTime = t1/CLOCKS_PER_SEC;
        int Hours = RunTime/3600;
        int Minutes = (RunTime%3600)/60;
        int Seconds = ((RunTime%3600)%60);
        std::cout << "DONE! Total elapsed time: " << Hours << "h " <<
                                                    Minutes << "m " <<
                                                    Seconds << "s\n";


    }
}


/*************************************
	CLEAR LOADED REFERENCE GALLERY
**************************************/
void MediaSync::ChangeReference()
{
    std::cout << "Resetting all galleries!\n";

    //Code for resetting all galleries
    std::string Command("./");
	Command.append(ReferenceMetadataFile);
    remove(Command.c_str());
    Command = "./";
	Command.append(CompareMetadataFile);
    remove(Command.c_str());
    ReferenceGallery.clear();
    CompareGallery.clear();

}

/*************************************
	CLEAR LOADED GALLERY
**************************************/
void MediaSync::ChangeSecond()
{
    //Code for resetting the gallery to be compared
    std::string Command(STATESFILE);
    remove(Command.c_str());
    CompareGallery.clear();
    ComparePossibleStates.clear();
    WITHGPSPARAMS.clear();
    WITHOUTGPSPARAMS.clear();
}
