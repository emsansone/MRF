// ConsoleApplication1.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include "mediasync.h"


int main(int argc, char* argv[])
{

   	MediaSync app;

    if(argv[1] == NULL){
        std::cout << "LEARNING:\n\n\tSync.exe -l \"DIR_PATH_TO_REFERENCE\" \"DIR_PATH_TO_GALLERY\" gamma delta\n\n";
        std::cout << "TESTING:\n\n\tSync.exe -t \"DIR_PATH_TO_REFERENCE\" \"DIR_PATH_TO_GALLERY\"\n\n";
        std::cout << "TESTING with LESS IMAGES:\n\n\tSync.exe -f \"TXT_FILE_REFERENCE\" \"TXT_FILE_GALLERY\" \"DIR_PATH_TO_REFERENCE\" \"DIR_PATH_TO_GALLERY\"\n\n";
        return -1;
    }

	std::string temp(argv[1]);
	if(temp.compare("-f") == 0){
		app.BrowseReference(argv[4],argv[2]);
		app.BrowseSecond(argv[5],argv[3]);
		if(app.ComparePossibleStates.size() < 2){
			std::fstream outputFile;
			outputFile.open (PERFORMANCEFILE, std::fstream::app);
  			outputFile << app.ReferenceDir.c_str() << "\n";
			outputFile << app.CompareDir.c_str() << "\n";
			outputFile << "idem idem\n";
			outputFile << "Offset: " << app.CompareGallery.at(0).UTCDateTimeOriginal - app.ReferenceGallery.at(0).UTCDateTimeOriginal << "\n";
			outputFile.close();
		}
		else{
			app.Test();		
		}
		app.ChangeSecond();
		app.ChangeReference();
	    return 0;
	}

    app.BrowseReference(argv[2]);
    app.BrowseSecond(argv[3]);
	if(temp.compare("-t") == 0){
		if(app.ComparePossibleStates.size() < 2){
			std::fstream outputFile;
			outputFile.open (PERFORMANCEFILE, std::fstream::app);
  			outputFile << app.ReferenceDir.c_str() << "\n";
			outputFile << app.CompareDir.c_str() << "\n";
			outputFile << "idem idem\n";
			outputFile << "Offset: " << app.CompareGallery.at(0).UTCDateTimeOriginal - app.ReferenceGallery.at(0).UTCDateTimeOriginal << "\n";
			outputFile.close();
		}
		else{
			app.Test();		
		}
    }

	if(temp.compare("-l") == 0){
        app.CoarseLearn.gamma = atof(argv[4]);
        app.CoarseLearn.delta = atof(argv[5]);
        app.Confirm();
        app.Newton();    }

    app.ChangeSecond();
    app.ChangeReference();
    
    return 0;
}

