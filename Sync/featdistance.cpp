#include "mediasync.h"
#include <cmath>


/*************************************
    This function calculates the
    distance between two images
    using GPS coordinates
**************************************/
float MediaSync::GPSDistance(MediaSync::Data* RefPoint, MediaSync::Data* CompPoint){
    float RefX, RefY, RefZ, CompX, CompY, CompZ, distance, result;

    if((RefPoint->GPSLatitude == GPSOVERVALUE) || (CompPoint->GPSLatitude == GPSOVERVALUE)){
        result = GPSOVERVALUE;
    }
    else{
        //Transformation from spherical to cartesian coordinates
        RefX = EARTHMEANRADIUS * cos(2*PI*(RefPoint->GPSLongitude)/360) * cos(2*PI*(RefPoint->GPSLatitude)/360);
        CompX = EARTHMEANRADIUS * cos(2*PI*(CompPoint->GPSLongitude)/360) * cos(2*PI*(CompPoint->GPSLatitude)/360);
        RefY = EARTHMEANRADIUS * sin(2*PI*(RefPoint->GPSLongitude)/360) * cos(2*PI*(RefPoint->GPSLatitude)/360);
        CompY = EARTHMEANRADIUS * sin(2*PI*(CompPoint->GPSLongitude)/360) * cos(2*PI*(CompPoint->GPSLatitude)/360);
        RefZ = EARTHMEANRADIUS * sin(2*PI*(RefPoint->GPSLatitude)/360);
        CompZ = EARTHMEANRADIUS * sin(2*PI*(CompPoint->GPSLatitude)/360);

        //Calculating the distance between two points
        distance = sqrt(pow((RefX - CompX), 2) + pow((RefY - CompY), 2) + pow((RefZ - CompZ), 2));

        result = 2*asin(distance/(2*EARTHMEANRADIUS));
    }
    return fabs(result);
}

/*************************************
    This function calculates the
    distance between two images
    using timestamps
**************************************/
float MediaSync::TimeDistance(MediaSync::Data* CurrComp, MediaSync::Data* NextComp, MediaSync::Data* CurrRef, MediaSync::Data* NextRef, int offset){
    CurrComp->ReferencedDateTimeOriginal += offset;
    NextComp->ReferencedDateTimeOriginal += offset;

    float result = abs(abs(NextComp->ReferencedDateTimeOriginal - NextRef->ReferencedDateTimeOriginal) +
                        abs(CurrComp->ReferencedDateTimeOriginal - CurrRef->ReferencedDateTimeOriginal));

    CurrComp->ReferencedDateTimeOriginal -= offset;
    NextComp->ReferencedDateTimeOriginal -= offset;

    return result;
}


