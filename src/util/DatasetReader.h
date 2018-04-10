//
// Created by maxkh on 07/03/2018.
//

#ifndef DSO_DATASETREADER_H
#define DSO_DATASETREADER_H


#include "util/Undistort.h"
#include "util/ImageAndExposure.h"
#include "IOWrapper/ImageRW.h"
#include "util/globalFuncs.h"
#include "util/globalCalib.h"

class DatasetReader {

    virtual dso::MinimalImageB* getImageRaw_internal(int id, int unused) = 0;


    virtual dso::ImageAndExposure* getImage_internal(int id, int unused) = 0;
protected:
    virtual  void loadTimestamps() = 0;
    std::vector<dso::ImageAndExposure*> preloadedImages;
    std::vector<double> timestamps;
    std::vector<float> exposures;
    std::string path;
    std::string calibfile;

public:
    virtual void getCalibMono(Eigen::Matrix3f &K, int &w, int &h) = 0;
    void setGlobalCalibration()
    {
        int w_out, h_out;
        Eigen::Matrix3f K;
        getCalibMono(K, w_out, h_out);
        dso::setGlobalCalib(w_out, h_out, K);

    }

    virtual int getNumImages() = 0;

    virtual double getTimestamp(int id) = 0;

    void prepImage(int id, bool as8U=false)
    {

    }

    virtual dso::MinimalImageB* getImageRaw(int id)
    {
            return getImageRaw_internal(id,0);
    }

    virtual dso::ImageAndExposure* getImage(int id, bool forceLoadDirectly=false)
    {
        return getImage_internal(id, 0);
    }

    virtual float* getPhotometricGamma() = 0;

    // undistorter. [0] always exists, [1-2] only when MT is enabled.
    dso::Undistort* undistort;
};


#endif //DSO_DATASETREADER_H
